import logging
import os
from abc import ABC
from threading import Thread

import torch_neuronx
from hf_batch_streamer import TextIteratorStreamerBatch
from transformers import AutoConfig, LlamaTokenizer
from transformers_neuronx.generation_utils import HuggingFaceGenerationModelAdapter
from transformers_neuronx.llama.model import LlamaForSampling

from ts.handler_utils.micro_batching import MicroBatching
from ts.protocol.otf_message_handler import send_intermediate_predict_response
from ts.torch_handler.base_handler import BaseHandler

logger = logging.getLogger(__name__)


class LLMHandler(BaseHandler, ABC):
    """
    Transformers handler class for text completion streaming on Inferentia2
    """

    def __init__(self):
        super(LLMHandler, self).__init__()
        self.initialized = False
        self.max_length = None
        self.tokenizer = None
        self.output_streamer = None
        # enable micro batching
        self.mb_handle = MicroBatching(self)
        self.handle = self.mb_handle

    def initialize(self, ctx):
        self.manifest = ctx.manifest
        properties = ctx.system_properties
        model_dir = properties.get("model_dir")

        # micro batching initialization
        parallelism = ctx.model_yaml_config.get("micro_batching", {}).get(
            "parallelism", None
        )
        if parallelism:
            logger.info(
                f"Setting micro batching parallelism  from model_config_yaml: {parallelism}"
            )
            self.mb_handle.parallelism = parallelism

        micro_batch_size = ctx.model_yaml_config.get("micro_batching", {}).get(
            "micro_batch_size", 1
        )
        logger.info(f"Setting micro batching size: {micro_batch_size}")
        self.mb_handle.micro_batch_size = micro_batch_size

        # settings for model compiliation and loading
        model_name = ctx.model_yaml_config["handler"]["model_name"]
        tp_degree = ctx.model_yaml_config["handler"]["tp_degree"]
        self.max_length = ctx.model_yaml_config["handler"]["max_length"]

        # allocate "tp_degree" number of neuron cores to the worker process
        os.environ["NEURON_RT_NUM_CORES"] = str(tp_degree)
        try:
            num_neuron_cores_available = (
                torch_neuronx.xla_impl.data_parallel.device_count()
            )
            assert num_neuron_cores_available >= int(tp_degree)
        except (RuntimeError, AssertionError) as error:
            raise RuntimeError(
                "Required number of neuron cores for tp_degree "
                + str(tp_degree)
                + " are not available: "
                + str(error)
            )

        os.environ["NEURON_CC_FLAGS"] = "--model-type=transformer-inference"

        self.tokenizer = LlamaTokenizer.from_pretrained(model_name)
        self.tokenizer.pad_token = self.tokenizer.eos_token
        self.model = LlamaForSampling.from_pretrained(
            model_dir, batch_size=self.mb_handle.micro_batch_size, tp_degree=tp_degree
        )
        logger.info("Starting to compile the model")
        self.model.to_neuron()
        logger.info("Model has been successfully compiled")
        model_config = AutoConfig.from_pretrained(model_dir)
        self.model = HuggingFaceGenerationModelAdapter(model_config, self.model)
        self.output_streamer = TextIteratorStreamerBatch(
            self.tokenizer,
            batch_size=self.mb_handle.micro_batch_size,
            skip_special_tokens=True,
        )

        self.initialized = True

    def preprocess(self, requests):
        input_text = []
        for req in requests:
            data = req.get("data") or req.get("body")
            if isinstance(data, (bytes, bytearray)):
                data = data.decode("utf-8")
            input_text.append(data.strip())

        # Ensure the compiled model can handle the input received
        if len(input_text) > self.mb_handle.micro_batch_size:
            raise ValueError(
                f"Model is compiled for batch size {self.mb_handle.micro_batch_size} but received input of size {len(input_text)}"
            )

        # Pad input to match compiled model batch size
        input_text.extend([""] * (self.mb_handle.micro_batch_size - len(input_text)))

        return self.tokenizer(input_text, return_tensors="pt", padding=True)

    def inference(self, tokenized_input):
        generation_kwargs = dict(
            tokenized_input,
            streamer=self.output_streamer,
            max_new_tokens=self.max_length,
        )
        self.model.reset_generation()
        thread = Thread(target=self.model.generate, kwargs=generation_kwargs)
        thread.start()

        micro_batch_req_id_map = self.mb_handle.get_micro_batch_req_id_map(
            self.context.request_ids
        )
        for new_text in self.output_streamer:
            send_intermediate_predict_response(
                new_text[: len(micro_batch_req_id_map)],
                micro_batch_req_id_map,
                "Intermediate Prediction success",
                200,
                self.context,
            )

        thread.join()

        return [""] * len(micro_batch_req_id_map)

    def postprocess(self, inference_output):
        return inference_output
