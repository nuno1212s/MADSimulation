#ifndef MADSIM_SIMFUNCS_H
#define MADSIM_SIMFUNCS_H

#include <tuple>
#include <random>
#include <utility>
#include <memory>

class DayInfo;

class Results {

public:
    const std::tuple<double, double> &getTotalCost() const {
        return totalCost;
    }

    const std::tuple<double, double> &getTotalCostPF() const {
        return totalCostPF;
    }

    const std::tuple<double, double> &getTotalCostCompensation() const {
        return totalCostCompensation;
    }

    const std::tuple<double, double> &getTotalPackages() const {
        return totalPackages;
    }

    Results(std::tuple<double, double> totalCost, std::tuple<double, double> totalCostPF,
            std::tuple<double, double> totalCostComp,
            std::tuple<double, double> totalPackages);

private:
    std::tuple<double, double> totalCost, totalCostPF, totalCostCompensation,
            totalPackages;

};

Results doResults(std::vector<double> &costPF, std::vector<double> &costCompensation, std::vector<int> &totalPackages,
                  int observations, double confidence);

class ObservationHolder {

public:
    ObservationHolder(double compensation, double oc_probability);

    std::tuple<double, double, int> runObservation(int dayCount);


    Results runSimulation(int observations, int dayCount, double confidence);

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

    void getDeliveriesForDay(int &, int &);

    int calculatePossibleOCs(int lockerPackages);

    int calculatePackagesTakenByOC(int possibleOCs, int maxPackagesToTake);

    void simulateDay(int packagesLeftOverLocker, int packagesLeftOverHome, DayInfo &);
};


#endif //MADSIM_SIMFUNCS_H
