#ifndef MADSIM_SIMFUNCS_H
#define MADSIM_SIMFUNCS_H

#include <tuple>

void setCompensation(double compensation, double oc_probability);

std::tuple<double, double, int> runObservation(int dayCount);

std::tuple<std::tuple<double, double>, std::tuple<double, double>,
        std::tuple<double, double>> runSimulation(int observations, int dayCount, double confidence);

#endif //MADSIM_SIMFUNCS_H
