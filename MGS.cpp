#include "MGS.h"
#include "InputValidator.h"
#include <cmath>


// Constructor
MGS::MGS(double arrivalRate,
         double serviceRate,
         int numberOfServers,
         double serviceTimeVariance)
{
    this->arrivalRate = arrivalRate;
    this->serviceRate = serviceRate;
    this->numberOfServers = numberOfServers;
    this->serviceTimeVariance = serviceTimeVariance;
}


// Calculates queue performance measures
QueueResult MGS::calculate()
{
    QueueResult result;


    // Input checks (arrivalRate must be strictly positive: arrivalRate == 0
    // used to slip through the old "< 0" check and later caused a 0/0
    // division when computing the waiting time).
    if(!InputValidator::isValidVariance(serviceTimeVariance) ||
        arrivalRate <= 0 ||
        serviceRate <= 0 ||
        numberOfServers <= 0)
    {
        return result;
    }


    // Calculate utilization
    double utilization =
        arrivalRate /
        (numberOfServers * serviceRate);


    result.setUtilization(utilization);


    // Check system stability
    if(utilization >= 1)
    {
        return result;
    }


    // Calculate coefficient of variation
    double serviceTimeMean = 1 / serviceRate;

    double CsSquared =
        serviceTimeVariance /
        pow(serviceTimeMean, 2);



    // Sakasegawa approximation of the M/G/s waiting time (Wq), the
    // standard formula for this model:
    //   Wq = ((Cs^2 + 1) / 2) * [ rho^(sqrt(2(s+1)) - 1) / (s * (1 - rho)) ] * (1 / mu)
    //
    // NOTE: this formula gives Wq (a TIME), not Lq (a COUNT) directly.
    // The previous version skipped straight to "Lq = factor * rho" without
    // going through Wq/Little's Law first, which silently drops a factor
    // of "numberOfServers" for every s > 1 and only happened to look right
    // when s == 1. Computing Wq first and then applying Little's Law
    // (Lq = lambda * Wq) keeps this consistent with GG1/GGS.
    double rhoPower =
        pow(utilization, sqrt(2 * (numberOfServers + 1)) - 1);


    double mmsQueueFactor =
        rhoPower /
        (numberOfServers * (1 - utilization));


    // Calculate average waiting time (Wq)
    double averageWaitingTime =
        ((CsSquared + 1) / 2)
        *
        mmsQueueFactor
        *
        serviceTimeMean;


    result.setAverageWaitingTime(
        averageWaitingTime
        );



    // Calculate average customers in queue (Little's Law: Lq = lambda * Wq)
    double averageCustomersInQueue =
        arrivalRate *
        averageWaitingTime;


    result.setAverageCustomersInQueue(
        averageCustomersInQueue
        );



    // Calculate average customers in system
    double averageCustomersInSystem =
        averageCustomersInQueue +
        (arrivalRate / serviceRate);


    result.setAverageCustomersInSystem(
        averageCustomersInSystem
        );



    // Calculate average time in system
    double averageTimeInSystem =
        averageWaitingTime +
        (1 / serviceRate);


    result.setAverageTimeInSystem(
        averageTimeInSystem
        );


    return result;
}