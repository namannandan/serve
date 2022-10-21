#include "src/utils/metrics/yaml_config.hh"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <set>
#include <string>

namespace torchserve {
TEST(YAMLConfigTest, TestLoadValidConfig) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());

  const std::string config_file_path =
      "test/resources/metrics_config/valid_config.yaml";
  config_handler.LoadConfiguration(config_file_path);

  std::set<std::string> expected_dimension_names = {"model_name", "host_name",
                                                    "level"};
  ASSERT_EQ(config_handler.GetDimensionNames(), expected_dimension_names);

  std::vector<MetricConfiguration> expected_system_metrics = {
      MetricConfiguration{MetricType::COUNTER,
                          "CounterSystemMetricExample",
                          "count",
                          {"model_name", "host_name"}},
      MetricConfiguration{MetricType::GAUGE,
                          "GaugeSystemMetricExample",
                          "count",
                          {"model_name", "host_name"}},
      MetricConfiguration{MetricType::HISTOGRAM,
                          "HistogramSystemMetricExample",
                          "ms",
                          {"model_name", "host_name"}}};
  ASSERT_EQ(config_handler.GetSystemMetrics(), expected_system_metrics);

  std::vector<MetricConfiguration> expected_model_metrics = {
      MetricConfiguration{MetricType::COUNTER,
                          "CounterModelMetricExample",
                          "count",
                          {"model_name", "host_name"}},
      MetricConfiguration{MetricType::COUNTER,
                          "AnotherCounterModelMetricExample",
                          "count",
                          {"model_name", "level"}},
      MetricConfiguration{MetricType::GAUGE,
                          "GaugeModelMetricExample",
                          "count",
                          {"model_name", "level"}},
      MetricConfiguration{MetricType::HISTOGRAM,
                          "HistogramModelMetricExample",
                          "ms",
                          {"model_name", "level"}}};
  ASSERT_EQ(config_handler.GetModelMetrics(), expected_model_metrics);
}

TEST(YAMLConfigTest, TestLoadMinimalValidConfig) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/minimal_valid_config.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());

  std::vector<MetricConfiguration> expected_model_metrics = {
      MetricConfiguration{
          MetricType::HISTOGRAM, "HistogramModelMetricExample", "ms", {}}};
  ASSERT_EQ(config_handler.GetModelMetrics(), expected_model_metrics);
}

TEST(YAMLConfigTest, TestLoadInvalidConfigWithDuplicateDimension) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/invalid_config_duplicate_dimension.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());
}

TEST(YAMLConfigTest, TestLoadInvalidConfigWithEmptyDimension) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/invalid_config_empty_dimension.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());
}

TEST(YAMLConfigTest, TestLoadInvalidConfigWithUndefinedDimension) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/invalid_config_undefined_dimension.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());
}

TEST(YAMLConfigTest, TestLoadInvalidConfigWithDuplicateMetricDimension) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/"
      "invalid_config_duplicate_metric_dimension.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());
}

TEST(YAMLConfigTest, TestLoadInvalidConfigWithMissingMetricName) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/"
      "invalid_config_missing_metric_name.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());
}

TEST(YAMLConfigTest, TestLoadInvalidConfigWithEmptyMetricName) {
  YAMLMetricsConfigurationHandler config_handler =
      YAMLMetricsConfigurationHandler();
  const std::string config_file_path =
      "test/resources/metrics_config/"
      "invalid_config_empty_metric_name.yaml";
  config_handler.LoadConfiguration(config_file_path);

  ASSERT_TRUE(config_handler.GetDimensionNames().empty());
  ASSERT_TRUE(config_handler.GetSystemMetrics().empty());
  ASSERT_TRUE(config_handler.GetModelMetrics().empty());
}
}  // namespace torchserve
