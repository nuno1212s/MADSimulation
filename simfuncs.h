#ifndef MADSIM_SIMFUNCS_H
#define MADSIM_SIMFUNCS_H

#include <tuple>
#include <random>
#include <utility>

class DayInfo;

class Results {

private:
    double minTotal, maxTotal,
            minComp, maxComp,
            minPF, maxPF,
            minPackages, maxPackages;

    int maxPackageTotal;
public:
    Results(double minTotal, double maxTotal, double minComp, double maxComp,
            double minPF, double maxPF, double minPackages, double maxPackages, int maxPackageTotal) :
            minTotal(minTotal), maxTotal(maxTotal), minComp(minComp), maxComp(maxComp),
            minPF(minPF), maxPF(maxPF), minPackages(minPackages), maxPackages(maxPackages),
            maxPackageTotal(maxPackageTotal) {}

    double getMinTotal() const {
        return minTotal;
    }

    double getMaxTotal() const {
        return maxTotal;
    }

    double getMinComp() const {
        return minComp;
    }

    double getMaxComp() const {
        return maxComp;
    }

    double getMinPf() const {
        return minPF;
    }

    double getMaxPf() const {
        return maxPF;
    }

    double getMinPackages() const {
        return minPackages;
    }

    double getMaxPackages() const {
        return maxPackages;
    }

    int getMaxPackageTotal() const {
        return maxPackageTotal;
    }

};

Results doResults(const std::vector<double> &costsPF, const std::vector<double> &costsComp,
                  const std::vector<int> &maxPackages, int observations, double confidence);

class ObservationHolder {

public:
    ObservationHolder(double compensation, double oc_probability);

    std::tuple<double, double, int> runObservation(int dayCount);

    virtual Results runSimulation(int observations, int dayCount, double confidence);

protected:
    double COMPENSATION;
    double OC_PROBABILITY;
private:
    /*
     * We encapsulate the rand buffer data into an observation holder.
     * We can create many of these holders and allow them to each have their own
     * Random number generator, which allows them to be parallelized extremely
     * Easily with almost linear performance benefits.
     */
    struct drand48_data randBuffer;

    std::tuple<int, int> getDeliveriesForDay();

    int calculatePossibleOCs(int lockerPackages);

    int calculatePackagesTakenByOC(int possibleOCs, int maxPackagesToTake);

    void simulateDay(int packagesLeftOverLocker, int packagesLeftOverHome, DayInfo &info);

    int getRandomDeliveries();

    double getRandomProb();
};


#endif //MADSIM_SIMFUNCS_H
