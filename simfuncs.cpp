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

static double COMPENSATION;

static double OC_PROBABILITY;

using namespace std::chrono;

static std::default_random_engine engine(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
static std::uniform_int_distribution<int> distribution(MIN_DELIVERIES, MAX_DELIVERIES);
static std::uniform_real_distribution<double> randomDist(0.0, 1.0);

class DayInfo {

private:
    std::tuple<int, int> packagesGenerated;

    std::tuple<int, int, int> deliveriesMade;

    std::tuple<double, double> deliveryCosts;

    std::tuple<int, int> lockerStatusEndOfDay;

public:
    DayInfo(std::tuple<int, int> packages, std::tuple<int, int, int> deliveries, std::tuple<double, double> costs,
            std::tuple<int, int> lockerStatus)
            : packagesGenerated(std::move(packages)), deliveriesMade(std::move(deliveries)),
              deliveryCosts(std::move(costs)), lockerStatusEndOfDay(std::move(lockerStatus)) {}

    const std::tuple<int, int> &getPackagesGenerated() const {
        return packagesGenerated;
    }

    const std::tuple<int, int, int> &getDeliveriesMade() const {
        return deliveriesMade;
    }

    const std::tuple<double, double> &getDeliveryCosts() const {
        return deliveryCosts;
    }

    const std::tuple<int, int> &getLockerStatusEndOfDay() const {
        return lockerStatusEndOfDay;
    }

};

std::tuple<int, int> getDeliveriesForDay() {

    int newPackages = distribution(engine);

    int newPackagesHome = 0;
    int newPackagesLocker = 0;

    for (int i = 0; i < newPackages; i++) {
        if (randomDist(engine) <= HOME_PROBABILITY) {
            newPackagesHome++;
        } else {
            newPackagesLocker++;
        }

    }

    return std::make_tuple(newPackagesHome, newPackagesLocker);
}

int calculatePossibleOCs(int lockerPackages) {

    int possibleOCs = 0;

    for (int i = 0; i < lockerPackages; i++) {
        if (randomDist(engine) <= PICK_UP_PROBABILITY) {
            possibleOCs++;
        }
    }

    return possibleOCs;
}

int calculatePackagesTakenByOC(int possibleOCs) {

    int packagesTaken = 0;

    for (int i = 0; i < possibleOCs; i++) {
        if (randomDist(engine) <= OC_PROBABILITY) {
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
DayInfo simulateDay(int packagesLeftOverLocker, int packagesLeftOverHome) {

    int newPackagesLocker, newPackagesHome;

    std::tie(newPackagesHome, newPackagesLocker) = getDeliveriesForDay();

    int lockerPackages = packagesLeftOverLocker + newPackagesLocker;

    int possibleOCs = calculatePossibleOCs(lockerPackages);

    //All possible OCs are people that pick up locker packages
    int notDeliveredLocker = lockerPackages - possibleOCs;

    int packagesTakenByOCs = calculatePackagesTakenByOC(possibleOCs);

    double costCompensation = packagesTakenByOCs * COMPENSATION;

    int packagesToDeliverPFNextDay = newPackagesHome - packagesTakenByOCs;

    double costPF;

    if (packagesLeftOverHome <= PF_PRICE_CHANGE) {
        costPF = packagesLeftOverHome * PRICE_PF_UNDER_PC;
    } else {
        costPF = PF_PRICE_CHANGE * PRICE_PF_UNDER_PC + (packagesLeftOverHome - PF_PRICE_CHANGE) * PRICE_PF_OVER_PC;
    }

    return DayInfo{std::make_tuple(newPackagesHome, newPackagesLocker),
                   std::make_tuple(packagesLeftOverLocker, packagesTakenByOCs, possibleOCs),
                   std::make_tuple(costPF, costCompensation),
                   std::make_tuple(packagesToDeliverPFNextDay, notDeliveredLocker)};
}

/**
 * Runs an observation, returns the cost of the compensations and the cost of the professional deliveries and the max amount of packages in the locker rooms at the same time
 * @param dayCount
 * @return
 */
std::tuple<double, double, int> runObservation(int dayCount) {

    double totalCostPF = 0, totalCostCompensation = 0;

    int packagesLeftOver = 0, packagesLeftOverHome = 0;

    int maxPackagesInLocker = 0;

    for (int day = 0; day < dayCount; day++) {

        auto returnValues = simulateDay(packagesLeftOver, packagesLeftOverHome);

        int newPackagesHome, newPackagesLocker;

        std::tie(newPackagesHome, newPackagesLocker) = returnValues.getPackagesGenerated();

        int deliveredByPF, deliveriesByOC, pickedUp;

        std::tie(deliveredByPF, deliveriesByOC, pickedUp) = returnValues.getDeliveriesMade();

        double costByPF, costByCompensation;

        std::tie(costByPF, costByCompensation) = returnValues.getDeliveryCosts();

        int packagesLeftOverLocker, packagesLeftOverForHome;

        std::tie(packagesLeftOverForHome, packagesLeftOverLocker) = returnValues.getLockerStatusEndOfDay();

        totalCostPF += costByPF;

        totalCostCompensation += costByCompensation;

        maxPackagesInLocker = std::max(maxPackagesInLocker, (newPackagesHome + newPackagesLocker + deliveredByPF));

        packagesLeftOver = packagesLeftOverLocker;
        packagesLeftOverHome = packagesLeftOverForHome;
    }

    return std::make_tuple(totalCostCompensation, totalCostPF, maxPackagesInLocker);
}

/**
 * Runs the simulation with the given parameters
 *
 * @param observations
 * @param dayCount
 * @param confidence
 * @return The min value and the max value for the compensation cost and profession delivery cost and the max amount of items in the lockers, with the confidence specified
 */
std::tuple<std::tuple<double, double>, std::tuple<double, double>, std::tuple<double, double>>
runSimulation(int observations, int dayCount, double confidence) {
    std::vector<double> costCompensationData(observations), costPFData(observations);

    std::vector<int> packagesInLockers(observations);

    double averageCostPF = 0,
            averageCostCompensation = 0,
            averageMaxPackages = 0;

    double varianceCostCompensation = 0,
            varianceCostPF = 0,
            varianceMaxPackages = 0;

    boost::math::students_t_distribution<double> dist(observations);

    double invAlpha = (1 - confidence) / 2;

    double totalCostComp = 0, totalCostPF = 0;

    long totalMaxPackagesInLockers = 0;

    for (int i = 0; i < observations; i++) {

        double observationCostCompensation, observationCostPF;

        int maxPackagesInLockers;

        std::tie(observationCostCompensation, observationCostPF, maxPackagesInLockers) = runObservation(dayCount);

        costCompensationData.push_back(observationCostCompensation);

        costPFData.push_back(observationCostPF);

        packagesInLockers.push_back(maxPackagesInLockers);

        totalMaxPackagesInLockers += maxPackagesInLockers;

        totalCostComp += observationCostCompensation;
        totalCostPF += observationCostPF;

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

    averageCostPF = totalCostPF / observations;
    averageCostCompensation = totalCostComp / observations;

    averageMaxPackages = ((double) totalMaxPackagesInLockers) / observations;

    for (double &it : costCompensationData) {
        varianceCostCompensation += std::pow(it - averageCostCompensation, 2);
    }

    varianceCostCompensation /= (observations - 1);

    for (double &it : costPFData) {
        varianceCostPF += std::pow(it - averageCostPF, 2);
    }

    varianceCostPF /= (observations - 1);

    for (int &packages : packagesInLockers) {
        varianceMaxPackages += std::pow((packages - averageMaxPackages), 2);
    }

    varianceMaxPackages /= (observations - 1);

    std::cout << "Variance cost compensation " << varianceCostCompensation << " | Variance professional "
              << varianceCostPF << " | Variance max packages " << varianceMaxPackages << std::endl;

    std::cout << "Sum cost compensation: " << totalCostComp << " | Sum cost PF: " << totalCostPF << " | Sum packages: "
              << totalMaxPackagesInLockers << " | Average CC: " << averageCostCompensation << " | Average PF: "
              << averageCostPF << " | Average Max packages: " << averageMaxPackages << std::endl;

    double T = boost::math::quantile(boost::math::complement(dist, invAlpha));

    double h = T * sqrt(varianceCostCompensation / observations);

    double h2 = T * sqrt(varianceCostPF / observations);

    double h3 = T * sqrt(varianceMaxPackages / observations);

    double max = averageCostCompensation + h, min = averageCostCompensation - h;

    double maxPF = averageCostPF + h2, minPF = averageCostPF - h2;

    double maxPackages = averageMaxPackages + h3, minPackages = averageMaxPackages - h3;

    return std::make_tuple(std::make_tuple(min, max), std::make_tuple(minPF, maxPF),
                           std::make_tuple(minPackages, maxPackages));
}

void setCompensation(double compensation, double oc_probability) {
    COMPENSATION = compensation;

    OC_PROBABILITY = oc_probability;
}