#ifndef MADSIM_SIMFUNCSASYNC_H
#define MADSIM_SIMFUNCSASYNC_H

#include <tuple>
#include <memory>
#include <vector>

class AsyncObservation {

public:
    AsyncObservation(double, double);

    std::tuple<std::tuple<double, double>, std::tuple<double, double>,
            std::tuple<double, double>> runSimulationAsync(int observations, int dayCount, double confidence);

private:
    double COMPENSATION;
    double OC_PROBABILITY;

    std::unique_ptr<std::vector<std::tuple<double, double, int>>>
        runObservationAsync(int id, int observationCounts, int dayCount);
};


#endif //MADSIM_SIMFUNCSASYNC_H
