#ifndef TS_CPP_BACKENDS_TORCH_SCRIPTED_HANDLER_BASE_HANDLER_HH_
#define TS_CPP_BACKENDS_TORCH_SCRIPTED_HANDLER_BASE_HANDLER_HH_

#include <torch/script.h>
#include <torch/torch.h>

#include <chrono>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <ratio>
#include <utility>

#include "src/utils/logging.hh"
#include "src/utils/message.hh"
#include "src/utils/metrics/cache.hh"
#include "src/utils/model_archive.hh"

namespace torchserve {
namespace torchscripted {
/**
 * @brief
 * TorchBaseHandler <=> BaseHandler:
 * https://github.com/pytorch/serve/blob/master/ts/torch_handler/base_handler.py#L37
 *
 * TorchBaseHandler is not responsible for loading model since it is derived
 * from TorchScritpedModelInstance.
 */
class BaseHandler {
 public:
  // NOLINTBEGIN(bugprone-exception-escape)
  BaseHandler() = default;
  // NOLINTEND(bugprone-exception-escape)
  virtual ~BaseHandler() = default;

  virtual void Initialize(
      const std::string& model_dir,
      std::shared_ptr<torchserve::Manifest>& manifest,
      std::shared_ptr<torchserve::MetricsCache>& metrics_cache) {
    model_dir_ = model_dir;
    manifest_ = manifest;
    metrics_cache_ = metrics_cache;
  };

  virtual std::pair<std::shared_ptr<torch::jit::script::Module>,
                    std::shared_ptr<torch::Device>>
  LoadModel(std::shared_ptr<torchserve::LoadModelRequest>& load_model_request);

  virtual std::vector<torch::jit::IValue> Preprocess(
      std::shared_ptr<torch::Device>& device,
      std::map<uint8_t, std::string>& idx_to_req_id,
      std::shared_ptr<torchserve::InferenceRequestBatch>& request_batch,
      std::shared_ptr<torchserve::InferenceResponseBatch>& response_batch);

  virtual torch::Tensor Inference(
      std::shared_ptr<torch::jit::script::Module> model,
      std::vector<torch::jit::IValue>& inputs,
      std::shared_ptr<torch::Device>& device,
      std::map<uint8_t, std::string>& idx_to_req_id,
      std::shared_ptr<torchserve::InferenceResponseBatch>& response_batch);

  virtual void Postprocess(
      const torch::Tensor& data, std::map<uint8_t, std::string>& idx_to_req_id,
      std::shared_ptr<torchserve::InferenceResponseBatch>& response_batch);

  /**
   * @brief
   * function Predict <=> entry point function handle
   * https://github.com/pytorch/serve/blob/master/ts/torch_handler/base_handler.py#L205
   * @param inference_request
   * @return std::shared_ptr<torchserve::InferenceResponse>
   */
  void Handle(
      std::shared_ptr<torch::jit::script::Module>& model,
      std::shared_ptr<torch::Device>& device,
      std::shared_ptr<torchserve::InferenceRequestBatch>& request_batch,
      std::shared_ptr<torchserve::InferenceResponseBatch>& response_batch) {
    try {
      auto start_time = std::chrono::high_resolution_clock::now();

      std::map<uint8_t, std::string> idx_to_req_id;
      auto inputs =
          Preprocess(device, idx_to_req_id, request_batch, response_batch);
      auto outputs =
          Inference(model, inputs, device, idx_to_req_id, response_batch);
      Postprocess(outputs, idx_to_req_id, response_batch);

      auto stop_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> duration =
          stop_time - start_time;
      try {
        auto& handler_time_metric = metrics_cache_->GetMetric(
            torchserve::MetricType::GAUGE, "HandlerTime");
        handler_time_metric.AddOrUpdate(
            std::vector<std::string>{manifest_->GetModel().model_name, "Model"},
            duration.count());
      } catch (std::invalid_argument& e) {
        TS_LOGF(ERROR, "Failed to record metric. {}", e.what());
      }
    } catch (...) {
      TS_LOG(ERROR, "Failed to handle this batch");
    }
  };

 protected:
  std::shared_ptr<torch::Device> GetTorchDevice(
      std::shared_ptr<torchserve::LoadModelRequest>& load_model_request);

  std::shared_ptr<torchserve::Manifest> manifest_;
  std::string model_dir_;
  std::shared_ptr<torchserve::MetricsCache> metrics_cache_;
};
}  // namespace torchscripted
}  // namespace torchserve
#endif  // TS_CPP_BACKENDS_TORCH_HANDLER_BASE_HANDLER_HH_
