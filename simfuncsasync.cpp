#include "simfuncsasync.h"
#include "simfuncs.h"
#include <boost/math/distributions/students_t.hpp>

#include "ctpl.h"

AsyncObservation::AsyncObservation(double compensation, double oc_prob) :
        COMPENSATION(compensation),
        OC_PROBABILITY(oc_prob) {

}

static unsigned int threadCount = std::thread::hardware_concurrency();

std::unique_ptr<std::vector<std::tuple<double, double, int>>>
AsyncObservation::runObservationAsync(int id, int observationCounts, int dayCount) {

//    std::cout << "Scheduled " << observationCounts << " on thread " << id << std::endl;

    ObservationHolder holder(COMPENSATION, OC_PROBABILITY);

    auto vector = std::make_unique<std::vector<std::tuple<double, double, int>>>(observationCounts);

    for (int i = 0; i < observationCounts; i++) {

        vector->push_back(holder.runObservation(dayCount));

    }

    std::cout << "Thread " << id << " has finished. " << std::endl;

    return vector;
}

std::tuple<std::tuple<double, double>, std::tuple<double, double>, std::tuple<double, double>>
AsyncObservation::runSimulationAsync(int observations, int dayCount, double confidence) {

    std::cout << "Running " << observations << " observations on " << threadCount << " threads" << std::endl;

    ctpl::thread_pool threadPool((int) threadCount);

    std::vector<
            std::future<
                    std::unique_ptr<
                            std::vector<
                                    std::tuple<double, double, int>>>>> results((int) threadCount);

    int observationsPerThread = observations / (int) threadCount;

    for (int i = 0; i < threadCount; i++) {
        results[i] = threadPool.push([this, observationsPerThread, dayCount](int id){
            return this->runObservationAsync(id, observationsPerThread, dayCount);
        });
    }

    double averageCostPF = 0,
            averageCostCompensation = 0,
            averageMaxPackages = 0;

    double varianceCostCompensation = 0,
            varianceCostPF = 0,
            varianceMaxPackages = 0;

    boost::math::students_t_distribution<double> dist(observations - 1);

    double invAlpha = (1 - confidence) / 2;

    double totalCostComp = 0, totalCostPF = 0;

    long totalMaxPackagesInLockers = 0;

    std::vector<double> costCompensationData(observations), costPFData(observations);

    std::vector<int> packagesInLockers(observations);

    for (int i = 0; i < threadCount; i++) {
        std::unique_ptr<std::vector<std::tuple<double, double, int>>> result = results[i].get();

        for (auto &it : *result) {

            double observationCostCompensation, observationCostPF;

            int maxPackagesInLockers;

            std::tie(observationCostCompensation, observationCostPF, maxPackagesInLockers) = it;

            costCompensationData.push_back(observationCostCompensation);

            costPFData.push_back(observationCostPF);

            packagesInLockers.push_back(maxPackagesInLockers);

            totalMaxPackagesInLockers += maxPackagesInLockers;

            totalCostComp += observationCostCompensation;
            totalCostPF += observationCostPF;
        }
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