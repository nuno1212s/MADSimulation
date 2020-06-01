//
// Created by nuno on 01/06/20.
//

#ifndef MADSIM_SIMFUNCSASYNC_H
#define MADSIM_SIMFUNCSASYNC_H

#include <tuple>

std::tuple<std::tuple<double, double>, std::tuple<double, double>,
        std::tuple<double, double>> runSimulationAsync(int observations, int dayCount, double confidence);

#endif //MADSIM_SIMFUNCSASYNC_H
