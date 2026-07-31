#include "MMS.h"
#include "InputValidator.h"
#include <cmath>


// Constructor
MMS::MMS(double arrivalRate,
         double serviceRate,
         int numberOfServers)
{
    this->arrivalRate = arrivalRate;
    this->serviceRate = serviceRate;
    this->numberOfServers = numberOfServers;
}


// Calculates factorial
long long MMS::factorial(int number)
{
    long long result = 1;

    for(int i = 1; i <= number; i++)
    {
        result *= i;
    }

    return result;
}


// Calculates queue performance measures  (utilizatin = rho)
QueueResult MMS::calculate()
{
    QueueResult result;

    // Calculate utilization (rho)
    if(arrivalRate <= 0 ||
        serviceRate <= 0 ||
        numberOfServers <= 0)
    {
        return result;
    }


    // NOTE: utilization (rho) for a multi-server M/M/s system is the
    // *per-server* utilization lambda / (s * mu), NOT lambda / mu.
    // lambda / mu on its own is the "offered load" (a), which only
    // equals utilization when there is a single server (s = 1).
    double lambdaByMu = arrivalRate / serviceRate;

    double utilization =
        lambdaByMu / numberOfServers;


    result.setUtilization(utilization);


    if(!InputValidator::isMultiServerStable(arrivalRate, serviceRate, numberOfServers))
    {
        return result;
    }

    // Calculate P0

    double sum = 0;

    for(int i = 0; i < numberOfServers; i++)
    {
        sum += pow(lambdaByMu, i) / factorial(i);
    }


    sum += pow(lambdaByMu, numberOfServers) /
           (factorial(numberOfServers) *
            (1 - utilization));


    double P0 = 1 / sum;


    // Calculate average customers in queue
    double averageCustomersInQueue =
        (P0 * pow(lambdaByMu, numberOfServers) * utilization) /
        (factorial(numberOfServers) *
         pow(1 - utilization, 2));


    result.setAverageCustomersInQueue(averageCustomersInQueue);


    // Calculate average customers in system
    double averageCustomersInSystem =
        averageCustomersInQueue + lambdaByMu;


    result.setAverageCustomersInSystem(averageCustomersInSystem);


    // Calculate average waiting time
    double averageWaitingTime =
        averageCustomersInQueue / arrivalRate;


    result.setAverageWaitingTime(averageWaitingTime);


    // Calculate average time in system
    double averageTimeInSystem =
        averageWaitingTime + (1 / serviceRate);


    result.setAverageTimeInSystem(averageTimeInSystem);


    return result;
}