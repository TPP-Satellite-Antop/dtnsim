#include "ContactPlanParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

ParsedContactPlan ContactPlanParser::parse(const std::string& fileName,
                         int nodesNumber,
                         double failureProb)
{
    auto parsed = ParsedContactPlan();
    parsed.nodesNumber = nodesNumber;

    std::ifstream file(fileName);
    if (!file.is_open())
        throw std::runtime_error("Error: wrong path to contacts file " + fileName);

    std::string line;
    while (std::getline(file, line)) {
        processLine(line, parsed, failureProb);
    }

    return parsed;
}

void ContactPlanParser::processLine(const std::string& fileLine,
                                    ParsedContactPlan& output,
                                    double failureProb)
{
    if (fileLine.empty() || fileLine.at(0) == '#')
        return;

    std::string a, command;
    double start = 0.0, end = 0.0, dataRateOrRange = 0.0, failureProbability = 0.0;
    int sourceEid = 0, destinationEid = 0;

    std::stringstream ss(fileLine);
    ss >> a >> command >> start >> end >> sourceEid;

    if (a != "a")
        return;

    // contact / range lines
    ss >> destinationEid >> dataRateOrRange >> failureProbability;

    if (failureProb >= 0) {
        failureProbability = failureProb;
    }

    if (command == "contact" || command == "ocontact") {
        ParsedContact c;
        c.start = start;
        c.end   = end;
        c.src   = sourceEid;
        c.dst   = destinationEid;
        c.dataRate = dataRateOrRange;
        c.failureProbability = failureProbability / 100.0;
        c.opportunistic = command == "ocontact";

        output.contacts.push_back(c);
    }
    else if (command == "range" || command == "orange") {
        ParsedRange r;
        r.start = start;
        r.end   = end;
        r.src   = sourceEid;
        r.dst   = destinationEid;
        r.owlt  = dataRateOrRange;
        r.opportunistic = command == "orange";

        output.ranges.push_back(r);
    }
    else {
        std::cout << "dtnsim warning: unknown contact plan command: "
                  << fileLine << std::endl;
    }
}

