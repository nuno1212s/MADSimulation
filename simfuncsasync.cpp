#include "simfuncsasync.h"
#include "simfuncs.h"
#include <boost/math/distributions/students_t.hpp>

#include "ctpl.h"

AsyncObservation::AsyncObservation(double compensation, double oc_prob) :
        COMPENSATION(compensation),
        OC_PROBABILITY(oc_prob),
        threadsToUse(std::thread::hardware_concurrency()) {}

AsyncObservation::AsyncObservation(double compensation, double oc_prob, unsigned int threads) :
        COMPENSATION(compensation), OC_PROBABILITY(oc_prob),
        threadsToUse(threads) {}

std::unique_ptr<std::vector<std::tuple<double, double, int>>>
AsyncObservation::runObservationAsync(int id, int observationCounts, int dayCount) {

//    std::cout << "Scheduled " << observationCounts << " on thread " << id << std::endl;

    ObservationHolder holder(COMPENSATION, OC_PROBABILITY);

    auto vector = std::make_unique<std::vector<std::tuple<double, double, int>>>(observationCounts);

    for (int i = 0; i < observationCounts; i++) {

        (*vector)[i] = (holder.runObservation(dayCount));

    }

    return vector;
}

Results
AsyncObservation::runSimulationAsync(int observations, int dayCount, double confidence) {

    std::cout << "Running " << observations << " observations on " << threadsToUse << " threads" << std::endl;

    ctpl::thread_pool threadPool((int) threadsToUse);

    std::vector<
            std::future<
                    std::unique_ptr<
                            std::vector<
                                    std::tuple<double, double, int>>>>> results((int) threadsToUse);

    int observationsPerThread = observations / (int) threadsToUse;

    for (int i = 0; i < threadsToUse; i++) {
        if (i == threadsToUse - 1) {
            //On the last created thread, assign any left over observations to the last thread.
            observationsPerThread += (observations % (int) threadsToUse);
        }

        results[i] = threadPool.push([this, observationsPerThread, dayCount](int id) {
            return this->runObservationAsync(id, observationsPerThread, dayCount);
        });
    }

    std::vector<double> costCompensationData(observations), costPFData(observations);

    std::vector<int> packagesInLockers(observations);

    int current = 0;

    for (int i = 0; i < threadsToUse; i++) {
        std::unique_ptr<std::vector<std::tuple<double, double, int>>> result = results[i].get();

        for (auto &it : *result) {

            double observationCostCompensation, observationCostPF;

            int maxPackagesInLockers;

            std::tie(observationCostCompensation, observationCostPF, maxPackagesInLockers) = it;

            costCompensationData[current] = (observationCostCompensation);

            costPFData[current] = observationCostPF;

            packagesInLockers[current] = maxPackagesInLockers;

            current++;
        }
    }

    return doResults(costPFData, costCompensationData, packagesInLockers, observations, confidence);
}