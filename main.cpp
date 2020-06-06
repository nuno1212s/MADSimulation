#include <iostream>
#include <random>
#include <tuple>
#include <iomanip>
#include <chrono>
#include "simfuncsasync.h"

static std::vector<std::tuple<double, double>> defaultCompensations = {{0,   0.01},
                                                                       {.5,  .25},
                                                                       {1,   .5},
                                                                       {1.5, .6},
                                                                       {1.8, .75}};

void runWithConfidence(int observations, int dayCount, double confidence, double compensation, double oc_probability) {

    AsyncObservation observation(compensation, oc_probability);

    auto timeStart = std::chrono::system_clock::now().time_since_epoch();

    auto result = observation.runSimulationAsync(observations, dayCount, confidence);

    auto timeEnd = std::chrono::system_clock::now().time_since_epoch() - timeStart;

    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd).count() << " ms"
              << std::endl;

    std::cout << "RESULTS FOR " << compensation << "€ with probability " << oc_probability << std::endl;

    std::cout << std::setprecision(7) << "Compensation: " << "Min: " << result.getMinComp() << " | Max: "
              << result.getMaxComp()
              << std::endl;

    std::cout << "Profession delivery cost: " << "Min: " << result.getMinPf() << " | Max: " << result.getMaxPf()
              << std::endl;

    std::cout << "Packages in lockers: " << "Min: " << result.getMinPackages() << " | Max: "
              << result.getMaxPackages()
              << std::endl;

    std::cout << "Max packages: " << result.getMaxPackageTotal() << std::endl;

    double totalCostMin = result.getMinTotal(),
            totalCostMax = result.getMaxTotal();

    std::cout << "Total cost: " << std::endl << "Min: " << totalCostMin << " | Max: " << totalCostMax << std::endl;
}

void checkSimType(int observations, int dayCount, double confidence) {

    std::cout << "1) Use default compensation levels." << std::endl
              << "2) Use custom compensation levels." << std::endl;

    int choice;

    std::cin >> choice;

    switch (choice) {
        case 1: {

            for (auto &it : defaultCompensations) {
                runWithConfidence(observations, dayCount, confidence, std::get<0>(it), std::get<1>(it));
            }

            break;
        }
        case 2: {

            double compensation, compensationProbability;

            std::cout << "Insert the compensation: " << std::endl;

            std::cin >> compensation;

            std::cout << "Insert the compensation probability: " << std::endl;

            std::cin >> compensationProbability;

            runWithConfidence(observations, dayCount, confidence, compensation, compensationProbability);

            break;
        }
        default:
            checkSimType(observations, dayCount, confidence);
            break;
    }

}

int main() {

    int observations, dayCount;

    double confidence;

    std::cout << "Insert the observation count:" << std::endl;

    std::cin >> observations;

    std::cout << "Insert the day count:" << std::endl;

    std::cin >> dayCount;

    std::cout << "Insert the confidence level:" << std::endl;

    std::cin >> confidence;

    checkSimType(observations, dayCount, confidence);
}