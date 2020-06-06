#ifndef MADSIM_SIMFUNCSASYNC_H
#define MADSIM_SIMFUNCSASYNC_H

#include <tuple>
#include <memory>
#include <vector>
#include "simfuncs.h"

class AsyncObservation {

public:
    AsyncObservation(double, double);

    AsyncObservation(double, double, unsigned int);

    Results runSimulationAsync(int observations, int dayCount, double confidence);

private:
    double COMPENSATION;
    double OC_PROBABILITY;
    unsigned int threadsToUse;

    std::unique_ptr<std::vector<std::tuple<double, double, int>>>
        runObservationAsync(int id, int observationCounts, int dayCount);
};


#endif //MADSIM_SIMFUNCSASYNC_H
