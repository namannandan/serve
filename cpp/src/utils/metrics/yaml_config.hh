#ifndef TS_CPP_UTILS_METRICS_YAML_CONFIG_HH_
#define TS_CPP_UTILS_METRICS_YAML_CONFIG_HH_

#include <yaml-cpp/yaml.h>

#include <set>
#include <string>
#include <vector>

#include "src/utils/metrics/config.hh"
#include "src/utils/metrics/metric.hh"

namespace torchserve {
class YAMLMetricsConfigurationHandler : public MetricsConfigurationHandler {
 public:
  YAMLMetricsConfigurationHandler()
      : dimension_names{}, system_metrics{}, model_metrics{} {}
  ~YAMLMetricsConfigurationHandler() override {}
  void LoadConfiguration(const std::string metrics_config_file_path) override;
  std::set<std::string> GetDimensionNames() override;
  std::vector<MetricConfiguration> GetSystemMetrics() override;
  std::vector<MetricConfiguration> GetModelMetrics() override;

 private:
  std::set<std::string> dimension_names;
  std::vector<MetricConfiguration> system_metrics;
  std::vector<MetricConfiguration> model_metrics;

  void ClearConfiguration();
  void ParseDimensionNames(const YAML::Node& document_node);
  void ParseSystemMetrics(const YAML::Node& document_node);
  void ParseModelMetrics(const YAML::Node& document_node);
  std::vector<MetricConfiguration> ParseMetrics(
      const YAML::Node& metrics_list_node, const MetricType& metric_type);
};
}  // namespace torchserve

#endif  // TS_CPP_UTILS_METRICS_YAML_CONFIG_HH_
