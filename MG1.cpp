#include "MG1.h"
#include "InputValidator.h"
#include <cmath>


// Constructor
MG1::MG1(double arrivalRate,
         double serviceRate,
         double serviceTimeVariance)
{
    this->arrivalRate = arrivalRate;
    this->serviceRate = serviceRate;
    this->serviceTimeVariance = serviceTimeVariance;
}


// Calculates queue performance measures
QueueResult MG1::calculate()
{
    QueueResult result;


    // Input validation (previously missing: this let arrivalRate/serviceRate
    // through as <= 0, which divides by zero or produces negative results).
    if (!InputValidator::isPositive(arrivalRate) ||
        !InputValidator::isPositive(serviceRate) ||
        !InputValidator::isValidVariance(serviceTimeVariance))
    {
        return result;
    }

    double utilization = arrivalRate / serviceRate;

    result.setUtilization(utilization);

    if(utilization >= 1)
    {
        return result;
    }



    // Calculate average customers in queue
    double averageCustomersInQueue =
        (pow(arrivalRate, 2) * serviceTimeVariance
         + pow(utilization, 2))
        /
        (2 * (1 - utilization));


    result.setAverageCustomersInQueue(averageCustomersInQueue);



    // Calculate average customers in system
    double averageCustomersInSystem =
        averageCustomersInQueue + utilization;


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