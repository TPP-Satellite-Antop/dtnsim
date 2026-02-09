#ifndef CONTACTPLANPARSER_H_
#define CONTACTPLANPARSER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

struct ParsedContact {
    double start;
    double end;
    int src;
    int dst;
    double dataRate;
    double failureProbability;
};

struct ParsedRange {
    double start;
    double end;
    int src;
    int dst;
    double owlt;
};

struct ParsedContactPlan {
    int nodesNumber;
    std::vector<ParsedContact> contacts;
    std::vector<ParsedRange> ranges;
};

class ContactPlanParser {
public:
    ParsedContactPlan parse(const std::string& fileName,
          int nodesNumber,
          double failureProb);

private:
    static void processLine(const std::string& line,
                            ParsedContactPlan& output,
                            double failureProb);
};

#endif /* CONTACTPLANPARSER_H_ */