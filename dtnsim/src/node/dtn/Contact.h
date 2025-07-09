#ifndef CONTACT_H_
#define CONTACT_H_

#include <iostream>
#include <vector>

using namespace std;

class Contact {
  public:
    Contact(int id, double start, double end, int sourceEid, int destinationEid, double dataRate,
            double confidence, double range);
    Contact(int id, double start, double end, double failureProbability, int sourceEid,
            int destinationEid, double dataRate, double confidence, double range);
    Contact(int id, double start, double end, int sourceEid, int destinationEid, double dataRate,
            double confidence, double range, bool discovered, bool predicted);
    Contact(int id, double start, double end, double failureProbability, int sourceEid,
            int destinationEid, double dataRate, double confidence, double range, bool discovered,
            bool predicted);
    virtual ~Contact();

    // A contact Id (unique)
    int getId() const;

    // Get basic parameters
    double getStart() const;
    double getEnd() const;
    int getSourceEid() const;
    int getDestinationEid() const;
    double getDataRate() const;
    double getVolume() const;
    double getDuration() const;
    double getConfidence() const;
    double getRange() const;
    double getFailureProbability() const;
    void setFailureProbability(double failureProbability);
    void setRange(double range);
    bool isDiscovered() const;
    bool isPredicted() const;

    // Get and Set residual capacity (Bytes)
    double getResidualVolume() const;
    void setResidualVolume(double residualVolume);

    // A pointer to external structures
    // (used by routing algorithms)
    void *work;

  private:
    int id_;
    double start_;
    double end_;
    int sourceEid_;
    int destinationEid_;
    double dataRate_;       // In Bytes per seconds
    double residualVolume_; // In Bytes
    double confidence_;
    double range_;
    double failureProbability_;
    bool discovered_;
    bool predicted_;
};

#endif /* CONTACT_H_ */
