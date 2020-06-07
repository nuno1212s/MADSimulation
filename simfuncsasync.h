#ifndef MADSIM_SIMFUNCSASYNC_H
#define MADSIM_SIMFUNCSASYNC_H

#include <tuple>
#include <memory>
#include <vector>
#include "simfuncs.h"

class AsyncObservation : public ObservationHolder {

public:
    AsyncObservation(double, double);

    AsyncObservation(double, double, unsigned int);

    Results runSimulation(int observations, int dayCount, double confidence) override;

private:
    unsigned int threadsToUse;

    std::unique_ptr<std::vector<std::tuple<double, double, int>>>
        runObservationAsync(int id, int observationCounts, int dayCount);
};


#endif //MADSIM_SIMFUNCSASYNC_H
