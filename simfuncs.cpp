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

ObservationHolder::ObservationHolder(double compensation, double oc_probability)
        : COMPENSATION(compensation),
          OC_PROBABILITY(oc_probability),
          randBuffer() {

    srand48_r(time(NULL), &randBuffer);

}

class DayInfo {

private:
    int packagesGeneratedHome, packagesGeneratedLocker;

    int deliveredByPF, deliveredByOC, pickedUP;

    double costPF, costCompensation;

    int packagesToDeliverPF, packagesToDeliverLocker;
public:
    DayInfo()
            : packagesGeneratedHome(0), packagesGeneratedLocker(0),
              deliveredByPF(0), deliveredByOC(0), pickedUP(0),
              costPF(0), costCompensation(0), packagesToDeliverPF(0),
              packagesToDeliverLocker(0) {}

    void setPackagesHome(int packagesGeneratedHome, int packagesGeneratedLocker) {
        DayInfo::packagesGeneratedHome = packagesGeneratedHome;
        DayInfo::packagesGeneratedLocker = packagesGeneratedLocker;
    }

    void setDeliveresMade(int deliveredByPF, int deliveredByOC, int pickedUp) {
        DayInfo::deliveredByPF = deliveredByPF;
        DayInfo::deliveredByOC = deliveredByOC;
        DayInfo::pickedUP = pickedUp;
    }

    void setCosts(double costsPF, double costsComp) {
        DayInfo::costPF = costsPF;
        DayInfo::costCompensation = costsComp;
    }

    void setEndOfDayStatus(int packagesToDeliverPF, int packagesToDeliverLocker) {
        DayInfo::packagesToDeliverPF = packagesToDeliverPF;
        DayInfo::packagesToDeliverLocker = packagesToDeliverLocker;
    }

    int getPackagesGeneratedHome() const {
        return packagesGeneratedHome;
    }

    int getPackagesGeneratedLocker() const {
        return packagesGeneratedLocker;
    }

    int getDeliveredByPf() const {
        return deliveredByPF;
    }

    int getDeliveredByOc() const {
        return deliveredByOC;
    }

    int getPickedUp() const {
        return pickedUP;
    }

    double getCostPf() const {
        return costPF;
    }

    double getCostCompensation() const {
        return costCompensation;
    }

    int getPackagesToDeliverPf() const {
        return packagesToDeliverPF;
    }

    int getPackagesToDeliverLocker() const {
        return packagesToDeliverLocker;
    }

};

int ObservationHolder::getRandomDeliveries() {

    double result;

    drand48_r(&randBuffer, &result);

    result *= (MAX_DELIVERIES - MIN_DELIVERIES);

    result += MIN_DELIVERIES;

    return (int) std::round(result);
}

double ObservationHolder::getRandomProb() {
    double result;

    drand48_r(&randBuffer, &result);

    return result;
}

std::tuple<int, int> ObservationHolder::getDeliveriesForDay() {
    int newPackages = getRandomDeliveries();

    int newPackagesHome = 0;
    int newPackagesLocker = 0;

    for (int i = 0; i < newPackages; i++) {
        if (getRandomProb() <= HOME_PROBABILITY) {
            newPackagesHome++;
        } else {
            newPackagesLocker++;
        }

    }

    return std::make_tuple(newPackagesHome, newPackagesLocker);
}

int ObservationHolder::calculatePossibleOCs(int lockerPackages) {

    int possibleOCs = 0;

    for (int i = 0; i < lockerPackages; i++) {
        if (getRandomProb() <= PICK_UP_PROBABILITY) {
            possibleOCs++;
        }
    }

    return possibleOCs;
}

int ObservationHolder::calculatePackagesTakenByOC(int possibleOCs, int maxPackages) {

    int packagesTaken = 0;

    for (int i = 0; i < possibleOCs && packagesTaken <= maxPackages; i++) {
        if (getRandomProb() <= OC_PROBABILITY) {
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

    std::tie(newPackagesHome, newPackagesLocker) = getDeliveriesForDay();

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

    info.setPackagesHome(newPackagesHome, newPackagesLocker);
    info.setDeliveresMade(packagesLeftOverHome, packagesTakenByOCs, possibleOCs);
    info.setCosts(costPF, costCompensation);
    info.setEndOfDayStatus(packagesToDeliverPFNextDay, notDeliveredLocker);
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

    DayInfo info;

    for (int day = 0; day < dayCount; day++) {

        simulateDay(packagesLeftOver, packagesLeftOverHome, info);

        int newPackagesHome = info.getPackagesGeneratedHome(),
                newPackagesLocker = info.getPackagesGeneratedLocker();

        int deliveredByPF = info.getDeliveredByPf(),
                deliveriesByOC = info.getDeliveredByOc(), pickedUp = info.getPickedUp();

        double costByPF = info.getCostPf(),
                costByCompensation = info.getCostCompensation();

        int packagesLeftOverLocker = info.getPackagesToDeliverLocker(),
                packagesLeftOverForHome = info.getPackagesToDeliverPf();

        totalCostPF += costByPF;

        totalCostCompensation += costByCompensation;

        maxPackagesInLocker = std::max(maxPackagesInLocker, (newPackagesHome + newPackagesLocker +
                                                             packagesLeftOver + deliveredByPF));

        packagesLeftOver = packagesLeftOverLocker;
        packagesLeftOverHome = packagesLeftOverForHome;
    }

    return std::make_tuple(totalCostCompensation, totalCostPF, maxPackagesInLocker);
}

Results doResults(const std::vector<double> &costsPF, const std::vector<double> &costsComp,
                  const std::vector<int> &maxPackages, int observations, double confidence) {

    double averageCostTotal,
            averageCostPF,
            averageCostComp,
            averageMaxPackages;

    double varianceCostTotal = 0,
            varianceCostComp = 0,
            varianceCostPF = 0,
            varianceMaxPackages = 0;

    boost::math::students_t_distribution<double> dist(observations - 1);

    double invAlpha = (1 - confidence) / 2;

    double totalCost = 0, totalCostComp = 0, totalCostPF = 0;

    int totalMaxPackages = 0;

    for (int i = 0; i < observations; i++) {

        totalCost += (costsPF[i] + costsComp[i]);

        totalCostPF += (costsPF[i]);

        totalCostComp += (costsComp[i]);

        totalMaxPackages += maxPackages[i];

    }

    averageCostTotal = totalCost / observations;
    averageCostPF = totalCostPF / observations;
    averageCostComp = totalCostComp / observations;
    averageMaxPackages = ((double) totalMaxPackages) / observations;

    for (int i = 0; i < observations; i++) {
        varianceCostTotal += std::pow((costsPF[i] + costsComp[i]) - averageCostTotal, 2);

        varianceCostPF += std::pow(costsPF[i] - averageCostPF, 2);

        varianceCostComp += std::pow(costsComp[i] - averageCostComp, 2);

        varianceMaxPackages += std::pow(maxPackages[i] - averageMaxPackages, 2);
    }

    varianceCostTotal /= (observations - 1);

    varianceCostComp /= (observations - 1);

    varianceCostPF /= (observations - 1);

    varianceMaxPackages /= (observations - 1);


    std::cout << "Variance cost compensation " << varianceCostComp << " | Variance professional "
              << varianceCostPF << " | Variance max packages " << varianceMaxPackages << std::endl;

    std::cout << "Sum cost compensation: " << totalCostComp << " | Sum cost PF: " << totalCostPF << " | Sum packages: "
              << totalMaxPackages << " | Average CC: " << averageCostComp << " | Average PF: "
              << averageCostPF << " | Average Max packages: " << averageMaxPackages << std::endl;

    double T = boost::math::quantile(boost::math::complement(dist, invAlpha));

    double hTotal = T * sqrt(varianceCostTotal / observations);

    double hComp = T * sqrt(varianceCostComp / observations);

    double hPF = T * sqrt(varianceCostPF / observations);

    double hPackages = T * sqrt(varianceMaxPackages / observations);

    double minTotal = averageCostTotal - hTotal, maxTotal = averageCostTotal + hTotal;

    double minPF = averageCostPF - hPF, maxPF = averageCostPF + hPF;

    double minComp = averageCostComp - hComp, maxCOmp = averageCostComp + hComp;

    double minPackages = averageMaxPackages - hPackages, maxPackage = averageMaxPackages + hPackages;

    return Results{minTotal, maxTotal, minComp, maxCOmp,
                   minPF, maxPF, minPackages, maxPackage};
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

        costCompensationData[i] = observationCostCompensation;

        costPFData[i] = observationCostPF;

        packagesInLockers[i] = maxPackagesInLockers;

    }

    return doResults(costPFData, costCompensationData, packagesInLockers, observations, confidence);
}
