#include "simfuncs.h"
#include <random>
#include <utility>
#include <iostream>
#include <chrono>
#include <boost/math/distributions/students_t.hpp>

#define MIN_DELIVERIES 10
#define MAX_DELIVERIES 50

#define PF_PRICE_CHANGE 10

#define PRICE_PF_UNDER_PC 1
#define PRICE_PF_OVER_PC 2

#define LOCKER_PROBABILITY .5
#define HOME_PROBABILITY (1 - LOCKER_PROBABILITY)

#define PICK_UP_PROBABILITY .75

using namespace std::chrono;

long simulationTimeTaken = 0;

ObservationHolder::ObservationHolder(double compensation, double oc_probability)
        : engine(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()),
          distribution(MIN_DELIVERIES, MAX_DELIVERIES),
          randomDist(0.0, 1.0),
          COMPENSATION(compensation),
          OC_PROBABILITY(oc_probability) {
    srand(time(nullptr));
}

class DayInfo {

private:

    int packagesGeneratedHome, packagesGeneratedLocker;

    int deliveriesMadePF, deliveriesByOC, pickedUp;

    double costByPF, costByComp;

    int packagesLeftOverLocker, packagesLeftOverHome;

public:
    int getPackagesGeneratedHome() const {
        return packagesGeneratedHome;
    }

    int getPackagesGeneratedLocker() const {
        return packagesGeneratedLocker;
    }

    int getDeliveriesMadePf() const {
        return deliveriesMadePF;
    }

    int getDeliveriesByOc() const {
        return deliveriesByOC;
    }

    int getPickedUp() const {
        return pickedUp;
    }

    double getCostByPf() const {
        return costByPF;
    }

    double getCostByComp() const {
        return costByComp;
    }

    int getPackagesLeftOverLocker() const {
        return packagesLeftOverLocker;
    }

    int getPackagesLeftOverHome() const {
        return packagesLeftOverHome;
    }

    void setPackagesGeneratedHome(int packagesGeneratedMin) {
        DayInfo::packagesGeneratedHome = packagesGeneratedMin;
    }

    void setPackagesGeneratedLocker(int packagesGeneratedMax) {
        DayInfo::packagesGeneratedLocker = packagesGeneratedMax;
    }

    void setDeliveriesMadePf(int deliveriesMadePf) {
        deliveriesMadePF = deliveriesMadePf;
    }

    void setDeliveriesByOc(int deliveriesByOc) {
        deliveriesByOC = deliveriesByOc;
    }

    void setPickedUp(int pickedUp) {
        DayInfo::pickedUp = pickedUp;
    }

    void setCostByPf(double costByPf) {
        costByPF = costByPf;
    }

    void setCostByComp(double costByComp) {
        DayInfo::costByComp = costByComp;
    }

    void setPackagesLeftOverLocker(int packagesLeftOverLocker) {
        DayInfo::packagesLeftOverLocker = packagesLeftOverLocker;
    }

    void setPackagesLeftOverHome(int packagesLeftOverHome) {
        DayInfo::packagesLeftOverHome = packagesLeftOverHome;
    }
};

Results::Results(std::tuple<double, double> totalCost, std::tuple<double, double> totalCostPF,
                 std::tuple<double, double> totalCostCompensation, std::tuple<double, double> totalPackages) :
        totalCost(std::move(totalCost)),
        totalCostPF(std::move(totalCostPF)),
        totalCostCompensation(std::move(totalCostCompensation)),
        totalPackages(std::move(totalPackages)) {
}

void ObservationHolder::getDeliveriesForDay(int &home, int &locker) {

    int newPackages = (rand() % (MAX_DELIVERIES - MIN_DELIVERIES)) + MIN_DELIVERIES;

    int newPackagesHome = 0;
    int newPackagesLocker = 0;

    double prob = HOME_PROBABILITY * 100;

    for (int i = 0; i < newPackages; i++) {
        if ((rand() % 100) <= prob) {
            newPackagesHome++;
        } else {
            newPackagesLocker++;
        }

    }

    home = newPackagesHome;
    locker = newPackagesLocker;
}

int ObservationHolder::calculatePossibleOCs(int lockerPackages) {

    int possibleOCs = 0;

    double prob = PICK_UP_PROBABILITY * 100;

    for (int i = 0; i < lockerPackages; i++) {
        if ((rand() % 100) <= prob) {
            possibleOCs++;
        }
    }

    return possibleOCs;
}

int ObservationHolder::calculatePackagesTakenByOC(int possibleOCs, int maxPackages) {

    int packagesTaken = 0;

    double prob = OC_PROBABILITY * 100;

    for (int i = 0; i < possibleOCs && packagesTaken <= maxPackages; i++) {
        if ((rand() % 100) <= prob) {
            packagesTaken++;
        }
    }

    return packagesTaken;
}

/**
 *
 * @param packagesLeftOverLocker
 * @return Returns a tuple containing the tuples for the following information:
 *      new packages generated for home and locker
 *      deliveries made by PF, OC and locker
 *      Costs of delivery by PF and OC
 *      Locker status at the end of the day (Home and Locker)
 */
void ObservationHolder::simulateDay(int packagesLeftOverLocker, int packagesLeftOverHome, DayInfo &info) {

    int newPackagesLocker, newPackagesHome;

    getDeliveriesForDay(newPackagesHome, newPackagesLocker);

    int lockerPackages = packagesLeftOverLocker + newPackagesLocker;

    int possibleOCs = calculatePossibleOCs(lockerPackages);

    //All possible OCs are people that pick up locker packages
    int notDeliveredLocker = lockerPackages - possibleOCs;

    int packagesTakenByOCs = calculatePackagesTakenByOC(possibleOCs, newPackagesHome);

    double costCompensation = packagesTakenByOCs * COMPENSATION;

    int packagesToDeliverPFNextDay = newPackagesHome - packagesTakenByOCs;

    double costPF;

    //The packages left from the day before will be delivered on the following day
    if (packagesLeftOverHome <= PF_PRICE_CHANGE) {
        costPF = packagesLeftOverHome * PRICE_PF_UNDER_PC;
    } else {
        costPF = PF_PRICE_CHANGE * PRICE_PF_UNDER_PC + (packagesLeftOverHome - PF_PRICE_CHANGE) * PRICE_PF_OVER_PC;
    }

    info.setPackagesGeneratedHome(newPackagesHome);
    info.setPackagesGeneratedLocker(newPackagesLocker);
    info.setPackagesLeftOverLocker(packagesLeftOverLocker);
    info.setDeliveriesByOc(packagesTakenByOCs);
    info.setPickedUp(possibleOCs);
    info.setCostByPf(costPF);
    info.setCostByComp(costCompensation);
    info.setPackagesLeftOverLocker(packagesToDeliverPFNextDay);
    info.setPackagesLeftOverHome(notDeliveredLocker);
}

/**
 * Runs an observation, returns the cost of the compensations and the cost of the professional deliveries and the max amount of packages in the locker rooms at the same time
 * @param dayCount
 * @return
 */
std::tuple<double, double, int> ObservationHolder::runObservation(int dayCount) {

    double totalCostPF = 0, totalCostCompensation = 0;

    int packagesLeftOver = 0, packagesLeftOverHome = 0;

    int maxPackagesInLocker = 0;

    DayInfo info{};

    for (int day = 0; day < dayCount; day++) {
        auto t1 = std::chrono::high_resolution_clock::now();

        simulateDay(packagesLeftOver, packagesLeftOverHome, info);

        auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        simulationTimeTaken += duration;

        int newPackagesHome = info.getPackagesGeneratedHome(), newPackagesLocker = info.getPackagesGeneratedLocker();

        int deliveredByPF = info.getDeliveriesMadePf(),
                deliveriesByOC = info.getDeliveriesByOc(), pickedUp = info.getPickedUp();

        double costByPF = info.getCostByPf(), costByCompensation = info.getCostByComp();

        int packagesLeftOverLocker = info.getPackagesLeftOverLocker(),
                packagesLeftOverForHome = info.getPackagesLeftOverHome();

        totalCostPF += costByPF;

        totalCostCompensation += costByCompensation;

        maxPackagesInLocker = std::max(maxPackagesInLocker, (newPackagesHome + newPackagesLocker + deliveredByPF));

        packagesLeftOver = packagesLeftOverLocker;
        packagesLeftOverHome = packagesLeftOverForHome;

    }

    return std::make_tuple(totalCostCompensation, totalCostPF, maxPackagesInLocker);
}

Results doResults(std::vector<double> &costPF, std::vector<double> &costCompensation,
                  std::vector<int> &packages, int observations, double confidence) {

    double averageCostTotal = 0,
            averageCostPF = 0,
            averageCostCompensation = 0,
            averageMaxPackages = 0;

    double varianceCostTotal = 0,
            varianceCostPF = 0,
            varianceCostCompensation = 0,
            varianceMaxPackages = 0;

    boost::math::students_t_distribution<double> dist(observations - 1);

    double invAlpha = (1 - confidence) / 2;

    double totalCost = 0, totalCostPF = 0,
            totalCostCompensation = 0, totalPackages = 0;

    for (int i = 0; i < observations; i++) {
        totalCost += (costPF[i] + costCompensation[i]);

        totalCostPF += costPF[i];
        totalCostCompensation += costCompensation[i];

        totalPackages += packages[i];
    }

    averageCostTotal = totalCost / observations;
    averageCostCompensation = totalCostCompensation / observations;
    averageCostPF = totalCostPF / observations;
    averageMaxPackages = totalPackages / observations;

    for (int i = 0; i < observations; i++) {

        varianceCostTotal += std::pow((costPF[i] + costCompensation[i]) - averageCostTotal, 2);

        varianceCostPF += std::pow(costPF[i] - averageCostPF, 2);

        varianceCostCompensation += std::pow(costCompensation[i] - averageCostCompensation, 2);

        varianceMaxPackages += std::pow(packages[i] - averageMaxPackages, 2);
    }

    varianceCostTotal /= (double) observations - 1;

    varianceCostPF /= (double) observations - 1;

    varianceCostCompensation /= (double) observations - 1;

    varianceMaxPackages /= (double) observations - 1;

    double T = boost::math::quantile(boost::math::complement(dist, invAlpha));

    double hTotal = T * std::sqrt(varianceCostTotal / observations);

    double hPF = T * std::sqrt(varianceCostPF / observations);
    double hComp = T * std::sqrt(varianceCostCompensation / observations);

    double hPackage = T * std::sqrt(varianceMaxPackages / observations);

    std::cout << "TIME TAKEN: " << simulationTimeTaken << std::endl;

    std::cout << "Variance cost compensation " << varianceCostCompensation << " | Variance professional "
              << varianceCostPF << " | Variance max packages " << varianceMaxPackages << std::endl;

    std::cout << "Sum cost compensation: " << totalCostCompensation << " | Sum cost PF: " << totalCostPF
              << " | Sum packages: "
              << totalPackages << " | Average CC: " << averageCostCompensation << " | Average PF: "
              << averageCostPF << " | Average Max packages: " << averageMaxPackages << std::endl;

    return Results{
            std::make_pair(averageCostTotal - hTotal, averageCostTotal + hTotal),
            std::make_pair(averageCostPF - hPF, averageCostPF + hPF),
            std::make_pair(averageCostCompensation - hComp, averageCostCompensation + hComp),
            std::make_pair(averageMaxPackages - hPackage, averageMaxPackages + hPackage)
    };
}

/**
 * Runs the simulation with the given parameters
 *
 * @param observations
 * @param dayCount
 * @param confidence
 * @return The min value and the max value for the compensation cost and profession delivery cost and the max amount of items in the lockers, with the confidence specified
 */
Results
ObservationHolder::runSimulation(int observations, int dayCount, double confidence) {
    std::vector<double> costCompensationData(observations), costPFData(observations);

    std::vector<int> packagesInLockers(observations);

    for (int i = 0; i < observations; i++) {

        double observationCostCompensation, observationCostPF;

        int maxPackagesInLockers;

        std::tie(observationCostCompensation, observationCostPF, maxPackagesInLockers) = runObservation(dayCount);

        costCompensationData[i] = (observationCostCompensation);

        costPFData[i] = (observationCostPF);

        packagesInLockers[i] = (maxPackagesInLockers);

//        if (i % 100 == 0) {
//            std::cout << "Starting observation: " << i << std::endl;
//
//            std::cout << "Current cost: Comp: " << observationCostCompensation << " - PF: " << observationCostPF
//                      << std::endl;
//
//            std::cout << "Max packages: " << maxPackagesInLockers << std::endl;
//
//        }

    }

    return doResults(costPFData, costCompensationData, packagesInLockers, observations, confidence);
}
