#ifndef MADSIM_SIMFUNCS_H
#define MADSIM_SIMFUNCS_H

#include <tuple>
#include <random>
#include <utility>

class DayInfo;

class ObservationHolder {

public:
    ObservationHolder(double compensation, double oc_probability);

    std::tuple<double, double, int> runObservation(int dayCount);

    std::tuple<std::tuple<double, double>, std::tuple<double, double>,
            std::tuple<double, double>> runSimulation(int observations, int dayCount, double confidence);

private:
    std::default_random_engine engine;
    std::uniform_int_distribution<int> distribution;
    std::uniform_real_distribution<double> randomDist;
    double COMPENSATION;
    double OC_PROBABILITY;

    std::tuple<int, int> getDeliveriesForDay();

    int calculatePossibleOCs(int lockerPackages);

    int calculatePackagesTakenByOC(int possibleOCs);

    DayInfo simulateDay(int packagesLeftOverLocker, int packagesLeftOverHome);
};


#endif //MADSIM_SIMFUNCS_H
