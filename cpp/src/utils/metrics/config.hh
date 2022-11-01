#ifndef TS_CPP_UTILS_METRICS_CONFIG_HH_
#define TS_CPP_UTILS_METRICS_CONFIG_HH_

#include <set>
#include <string>
#include <vector>

#include "src/utils/metrics/metric.hh"

namespace torchserve {
enum MetricsMode { LOG, PROMETHEUS };

enum MetricsContext { BACKEND, FRONTEND };

struct MetricConfiguration {
  MetricType type;
  std::string name;
  std::string unit;
  std::vector<std::string> dimension_names;

  bool operator==(const MetricConfiguration& config) const {
    return (config.type == type) && (config.name == name) &&
           (config.unit == unit) && (config.dimension_names == dimension_names);
  }
};

class MetricsConfigurationHandler {
 public:
  virtual ~MetricsConfigurationHandler() {}
  virtual void LoadConfiguration(const std::string& metrics_config_file_path,
                                 const MetricsContext& metrics_context) = 0;
  virtual MetricsMode GetMode() = 0;
  virtual std::set<std::string> GetDimensionNames() = 0;
  virtual std::vector<MetricConfiguration> GetModelMetrics() = 0;
  virtual std::vector<MetricConfiguration> GetTsMetrics() = 0;
};
}  // namespace torchserve

#endif  // TS_CPP_UTILS_METRICS_CONFIG_HH_
