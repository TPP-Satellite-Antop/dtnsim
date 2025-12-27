#include "Dtn.h"

Dtn::Dtn() {}

Dtn::~Dtn() = default;

void Dtn::setMetricCollector(MetricCollector *metricCollector) {
    this->metricCollector_ = metricCollector;
}

int Dtn::numInitStages() const {
    return 2;
}

/**
 * Implementation of method inherited from observer to update gui according to the number of
 * bundles stored in sdr.
 */
void Dtn::update() {
    // Update srd size text
    graphicsModule->setBundlesInSdr(sdr_->getBundlesCountInSdr());
}

Routing* Dtn::getRouting() {
    return this->routing;
}