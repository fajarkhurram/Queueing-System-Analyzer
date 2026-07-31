#include "GGS.h"
#include "InputValidator.h"
#include <cmath>


// Constructor
GGS::GGS(double arrivalRate,
         double serviceRate,
         int numberOfServers,
         double arrivalTimeVariance,
         double serviceTimeVariance)
{
    this->arrivalRate = arrivalRate;
    this->serviceRate = serviceRate;
    this->numberOfServers = numberOfServers;
    this->arrivalTimeVariance = arrivalTimeVariance;
    this->serviceTimeVariance = serviceTimeVariance;
}


// Calculates queue performance measures
QueueResult GGS::calculate()
{
    QueueResult result;


    // Input checks
    if(arrivalRate <= 0 ||
        serviceRate <= 0 ||
        numberOfServers <= 0 ||
        arrivalTimeVariance < 0 ||
        serviceTimeVariance < 0)
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


    // Calculate mean arrival time
    double arrivalTimeMean =
        1 / arrivalRate;


    // Calculate mean service time
    double serviceTimeMean =
        1 / serviceRate;


    // Calculate coefficient of variation
    double CaSquared =
        arrivalTimeVariance /
        pow(arrivalTimeMean, 2);


    double CsSquared =
        serviceTimeVariance /
        pow(serviceTimeMean, 2);



    // Sakasegawa / Allen-Cunneen approximation of the G/G/s waiting
    // time (Wq):
    //   Wq = ((Ca^2 + Cs^2) / 2) * [ rho^(sqrt(2(s+1)) - 1) / (s * (1 - rho)) ] * (1 / mu)
    //
    // NOTE: "waitingFactor" below is dimensionless; it must be scaled by
    // the mean service time to become a waiting TIME. The previous version
    // used it directly as Lq (a count), which is dimensionally wrong and
    // silently dropped the "numberOfServers" factor that Little's Law
    // reintroduces. Computing Wq first (as GG1 already does) and then
    // Lq = lambda * Wq keeps every model consistent.
    double waitingFactor =
        pow(utilization,
            sqrt(2 * (numberOfServers + 1)) - 1)
        /
        (numberOfServers * (1 - utilization));


    double averageWaitingTime =
        ((CaSquared + CsSquared) / 2)
        *
        waitingFactor
        *
        serviceTimeMean;


    result.setAverageWaitingTime(
        averageWaitingTime
        );



    // Calculate Lq using Little's Law (Lq = lambda * Wq)
    double averageCustomersInQueue =
        arrivalRate *
        averageWaitingTime;


    result.setAverageCustomersInQueue(
        averageCustomersInQueue
        );



    // Calculate Ls
    double averageCustomersInSystem =
        averageCustomersInQueue +
        (arrivalRate / serviceRate);


    result.setAverageCustomersInSystem(
        averageCustomersInSystem
        );



    // Calculate Ws
    double averageTimeInSystem =
        averageWaitingTime +
        serviceTimeMean;


    result.setAverageTimeInSystem(
        averageTimeInSystem
        );


    return result;
}