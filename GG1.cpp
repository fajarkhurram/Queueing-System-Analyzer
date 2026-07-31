#include "GG1.h"
#include "InputValidator.h"
#include <cmath>


// Constructor
GG1::GG1(double arrivalRate,
         double serviceRate,
         double arrivalTimeVariance,
         double serviceTimeVariance)
{
    this->arrivalRate = arrivalRate;
    this->serviceRate = serviceRate;
    this->arrivalTimeVariance = arrivalTimeVariance;
    this->serviceTimeVariance = serviceTimeVariance;
}


// Calculates queue performance measures
QueueResult GG1::calculate()
{
    QueueResult result;


    // Input checks
    if(arrivalRate <= 0 ||
        serviceRate <= 0 ||
        arrivalTimeVariance < 0 ||
        serviceTimeVariance < 0)
    {
        return result;
    }


    // Calculate utilization
    double utilization =
        arrivalRate / serviceRate;


    result.setUtilization(utilization);


    // Check system stability
    if(!InputValidator::isSingleServerStable(arrivalRate, serviceRate))
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



    // Kingman's approximation formula
    double averageWaitingTime =
        ((CaSquared + CsSquared) / 2)
        *
        (utilization / (1 - utilization))
        *
        serviceTimeMean;



    result.setAverageWaitingTime(
        averageWaitingTime
        );



    // Calculate average time in system
    double averageTimeInSystem =
        averageWaitingTime +
        serviceTimeMean;


    result.setAverageTimeInSystem(
        averageTimeInSystem
        );



    // Calculate average customers in queue
    double averageCustomersInQueue =
        arrivalRate *
        averageWaitingTime;


    result.setAverageCustomersInQueue(
        averageCustomersInQueue
        );



    // Calculate average customers in system
    double averageCustomersInSystem =
        averageCustomersInQueue +
        utilization;


    result.setAverageCustomersInSystem(
        averageCustomersInSystem
        );


    return result;
}