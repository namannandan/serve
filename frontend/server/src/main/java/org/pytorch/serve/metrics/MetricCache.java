package org.pytorch.serve.metrics;

import org.pytorch.serve.util.ConfigManager;
import org.pytorch.serve.wlm.WorkerLifeCycle;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;

public class MetricCache {
    private static final Logger logger = LoggerFactory.getLogger(MetricCache.class);

    private static final MetricCache METRIC_CACHE = new MetricCache();

    private MetricCache() {
        metricsFrontend = new HashMap<String, IMetric>();
        metricsBackend = new ConcurrentMap<String, IMetric>();
    }

    public static MetricCache getInstance() {
        return METRIC_CACHE;
    }

    private static MetricCache instance;
    private Map<String, IMetric> metricsFrontend;
    private ConcurrentMap<String, IMetric> metricsBackend;

    public void addMetricFrontend(String metricName, IMetric metric);

    public IMetric getMetricFrontend(String metricName);

    public void addMetricBackend(String metricName, IMetric metric);

    public IMetric getMetricBackend(String metricName);
}
