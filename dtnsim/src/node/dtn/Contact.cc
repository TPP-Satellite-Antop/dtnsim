#include <src/node/dtn/Contact.h>

Contact::Contact(int id, double start, double end, int sourceEid, int destinationEid,
                 double dataRate, double confidence, double range) {
    this->id_ = id;
    this->start_ = start;
    this->end_ = end;
    this->sourceEid_ = sourceEid;
    this->destinationEid_ = destinationEid;
    this->dataRate_ = dataRate;
    this->confidence_ = confidence;
    this->residualVolume_ = (end - start) * dataRate;
    this->range_ = range;
    this->discovered_ = false;
    this->predicted_ = false;
    this->failureProbability_ = 0.0;
}

Contact::Contact(int id, double start, double end, double failureProbability, int sourceEid,
                 int destinationEid, double dataRate, double confidence, double range) {
    this->id_ = id;
    this->start_ = start;
    this->end_ = end;
    this->sourceEid_ = sourceEid;
    this->destinationEid_ = destinationEid;
    this->dataRate_ = dataRate;
    this->confidence_ = confidence;
    this->residualVolume_ = (end - start) * dataRate;
    this->range_ = range;
    this->discovered_ = false;
    this->predicted_ = false;
    this->failureProbability_ = failureProbability;
}

Contact::Contact(int id, double start, double end, int sourceEid, int destinationEid,
                 double dataRate, double confidence, double range, bool discovered,
                 bool predicted) {
    this->id_ = id;
    this->start_ = start;
    this->end_ = end;
    this->sourceEid_ = sourceEid;
    this->destinationEid_ = destinationEid;
    this->dataRate_ = dataRate;
    this->confidence_ = confidence;
    this->residualVolume_ = (end - start) * dataRate;
    this->range_ = range;
    this->discovered_ = discovered;
    this->predicted_ = predicted;
    this->failureProbability_ = 0.0;
}

Contact::Contact(int id, double start, double end, double failureProbability, int sourceEid,
                 int destinationEid, double dataRate, double confidence, double range,
                 bool discovered, bool predicted) {
    this->id_ = id;
    this->start_ = start;
    this->end_ = end;
    this->sourceEid_ = sourceEid;
    this->destinationEid_ = destinationEid;
    this->dataRate_ = dataRate;
    this->confidence_ = confidence;
    this->residualVolume_ = (end - start) * dataRate;
    this->range_ = range;
    this->discovered_ = discovered;
    this->predicted_ = predicted;
    this->failureProbability_ = failureProbability;
}

Contact::~Contact() {}

double Contact::getDataRate() const {
    return dataRate_;
}

int Contact::getDestinationEid() const {
    return destinationEid_;
}

int Contact::getId() const {
    return id_;
}

double Contact::getResidualVolume() const {
    return residualVolume_;
}

void Contact::setResidualVolume(double residualVolume) {
    this->residualVolume_ = residualVolume;
}

int Contact::getSourceEid() const {
    return sourceEid_;
}

double Contact::getStart() const {
    return start_;
}

double Contact::getEnd() const {
    return end_;
}

double Contact::getDuration() const {
    return (end_ - start_);
}

double Contact::getFailureProbability() const {
    return this->failureProbability_;
}

void Contact::setFailureProbability(double failureProbability) {
    this->failureProbability_ = failureProbability;
}

double Contact::getVolume() const {
    return (end_ - start_) * dataRate_;
}

double Contact::getConfidence() const {
    return confidence_;
}

void Contact::setRange(double range) {
    this->range_ = range;
}

bool Contact::isDiscovered() const {
    return this->discovered_;
}

bool Contact::isPredicted() const {
    return this->predicted_;
}

double Contact::getRange() const {
    if (range_ < 0) {
        cout << "Contact.cc: warning, range not available for nodes " << sourceEid_ << "-"
             << destinationEid_ << ", assuming range=0" << endl;
        return 0.0;
    }
    return range_;
}
