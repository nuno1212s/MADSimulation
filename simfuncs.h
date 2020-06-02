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
    /*
     * We encapsulate these into their own observation holder so that
     * We can create many of these holders and allow them to each have their own
     * Random number generator, which allows them to be parallelized extremely
     * Easily with almost linear performance benefits.
     *
     * Using these engines as global variables removes most of the performance benefits.
     */
    std::default_random_engine engine;
    std::uniform_int_distribution<int> distribution;
    std::uniform_real_distribution<double> randomDist;
    double COMPENSATION;
    double OC_PROBABILITY;

    std::tuple<int, int> getDeliveriesForDay();

    int calculatePossibleOCs(int lockerPackages);

    int calculatePackagesTakenByOC(int possibleOCs, int maxPackagesToTake);

    DayInfo simulateDay(int packagesLeftOverLocker, int packagesLeftOverHome);
};


#endif //MADSIM_SIMFUNCS_H
