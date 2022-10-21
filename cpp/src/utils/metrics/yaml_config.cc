#include "src/utils/metrics/yaml_config.hh"

#include <set>

#include "src/utils/logging.hh"

namespace torchserve {
void YAMLMetricsConfigurationHandler::LoadConfiguration(
    const std::string metrics_config_file_path) {
  ClearConfiguration();

  try {
    YAML::Node config_node = YAML::LoadFile(metrics_config_file_path);
    ParseDimensionNames(config_node);
    ParseSystemMetrics(config_node);
    ParseModelMetrics(config_node);
  } catch (YAML::ParserException& e) {
    TS_LOGF(ERROR, "[METRICS]Failed to parse YAML configuration file: {}. {}",
            metrics_config_file_path, e.what());
    ClearConfiguration();
  } catch (YAML::RepresentationException& e) {
    TS_LOGF(ERROR,
            "[METRICS]Failed to interpret YAML configuration file: {}. {}",
            metrics_config_file_path, e.what());
    ClearConfiguration();
  } catch (YAML::Exception& e) {
    TS_LOGF(ERROR, "[METRICS]Failed to load YAML configuration file: {}. {}",
            metrics_config_file_path, e.what());
    ClearConfiguration();
  }
}

std::set<std::string> YAMLMetricsConfigurationHandler::GetDimensionNames() {
  return dimension_names;
}

std::vector<MetricConfiguration>
YAMLMetricsConfigurationHandler::GetSystemMetrics() {
  return system_metrics;
}

std::vector<MetricConfiguration>
YAMLMetricsConfigurationHandler::GetModelMetrics() {
  return model_metrics;
}

void YAMLMetricsConfigurationHandler::ClearConfiguration() {
  dimension_names.clear();
  system_metrics.clear();
  model_metrics.clear();
}

void YAMLMetricsConfigurationHandler::ParseDimensionNames(
    const YAML::Node& document_node) {
  if (!document_node["dimensions"]) {
    return;
  }

  const std::vector<std::string> dimension_names_list =
      document_node["dimensions"].as<std::vector<std::string>>();

  for (const auto& name : dimension_names_list) {
    if (dimension_names.insert(name).second == false) {
      throw YAML::Exception(YAML::Mark::null_mark(),
                            "Dimension names defined under central "
                            "\"dimensions\" key must be unique");
    }
    if (name.empty()) {
      throw YAML::Exception(
          YAML::Mark::null_mark(),
          "Dimension names defined under \"dimensions\" key cannot be emtpy");
    }
  }
}

void YAMLMetricsConfigurationHandler::ParseSystemMetrics(
    const YAML::Node& document_node) {
  if (!document_node["ts_metrics"]) {
    return;
  }

  const YAML::Node system_metrics_node = document_node["ts_metrics"];

  if (system_metrics_node["counter"]) {
    const std::vector<MetricConfiguration> counter_metrics =
        ParseMetrics(system_metrics_node["counter"], MetricType::COUNTER);
    system_metrics.insert(system_metrics.end(), counter_metrics.begin(),
                          counter_metrics.end());
  }

  if (system_metrics_node["gauge"]) {
    const std::vector<MetricConfiguration> gauge_metrics =
        ParseMetrics(system_metrics_node["gauge"], MetricType::GAUGE);
    system_metrics.insert(system_metrics.end(), gauge_metrics.begin(),
                          gauge_metrics.end());
  }

  if (system_metrics_node["histogram"]) {
    const std::vector<MetricConfiguration> histogram_metrics =
        ParseMetrics(system_metrics_node["histogram"], MetricType::HISTOGRAM);
    system_metrics.insert(system_metrics.end(), histogram_metrics.begin(),
                          histogram_metrics.end());
  }
}

void YAMLMetricsConfigurationHandler::ParseModelMetrics(
    const YAML::Node& document_node) {
  if (!document_node["model_metrics"]) {
    return;
  }

  const YAML::Node model_metrics_node = document_node["model_metrics"];

  if (model_metrics_node["counter"]) {
    const std::vector<MetricConfiguration> counter_metrics =
        ParseMetrics(model_metrics_node["counter"], MetricType::COUNTER);
    model_metrics.insert(model_metrics.end(), counter_metrics.begin(),
                         counter_metrics.end());
  }

  if (model_metrics_node["gauge"]) {
    const std::vector<MetricConfiguration> gauge_metrics =
        ParseMetrics(model_metrics_node["gauge"], MetricType::GAUGE);
    model_metrics.insert(model_metrics.end(), gauge_metrics.begin(),
                         gauge_metrics.end());
  }

  if (model_metrics_node["histogram"]) {
    const std::vector<MetricConfiguration> histogram_metrics =
        ParseMetrics(model_metrics_node["histogram"], MetricType::HISTOGRAM);
    model_metrics.insert(model_metrics.end(), histogram_metrics.begin(),
                         histogram_metrics.end());
  }
}

std::vector<MetricConfiguration> YAMLMetricsConfigurationHandler::ParseMetrics(
    const YAML::Node& metrics_list_node, const MetricType& metric_type) {
  std::vector<MetricConfiguration> metrics_config =
      metrics_list_node.as<std::vector<MetricConfiguration>>();
  for (auto& metric : metrics_config) {
    std::set<std::string> metric_dimensions_set{};
    for (const auto& name : metric.dimension_names) {
      if (dimension_names.find(name) == dimension_names.end()) {
        std::string error_message =
            "Dimension \"" + name +
            "\" associated with metric: " + metric.name +
            " not defined under central \"dimensions\" key in configuration";
        throw YAML::Exception(YAML::Mark::null_mark(), error_message);
      }
      if (metric_dimensions_set.insert(name).second == false) {
        std::string error_message =
            "Dimensions for metric: " + metric.name + " must be unique";
        throw YAML::Exception(YAML::Mark::null_mark(), error_message);
      }
    }

    metric.type = metric_type;
  }

  return metrics_config;
}
}  // namespace torchserve

namespace YAML {
template <>
struct convert<torchserve::MetricConfiguration> {
  static bool decode(const Node& metric_config_node,
                     torchserve::MetricConfiguration& metric_config) {
    if (!metric_config_node["name"] || !metric_config_node["unit"] ||
        !metric_config_node["dimensions"]) {
      TS_LOG(ERROR,
             "[METRICS]Configuration for a metric must consist of \"name\", "
             "\"unit\" and \"dimensions\"");
      return false;
    }

    metric_config.name = metric_config_node["name"].as<std::string>();
    metric_config.unit = metric_config_node["unit"].as<std::string>();
    metric_config.dimension_names =
        metric_config_node["dimensions"].as<std::vector<std::string>>();

    if (metric_config.name.empty()) {
      TS_LOG(ERROR,
             "[METRICS]Configuration for a metric must consist of a non-empty "
             "\"name\"");
      return false;
    }

    return true;
  }
};
}  // namespace YAML
