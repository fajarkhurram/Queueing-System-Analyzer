#include "MM1.h"
#include "InputValidator.h"

// Constructor
MM1::MM1(double arrivalRate, double serviceRate)
{
    this->arrivalRate = arrivalRate;
    this->serviceRate = serviceRate;
}

// Calculate all queue measures
QueueResult MM1::calculate()
{
    QueueResult result;

    // Input validation
    if (!InputValidator::isPositive(arrivalRate))
    {
        // GUI Message:
        // "Arrival Rate must be greater than zero."
        return result;
    }

    if (!InputValidator::isPositive(serviceRate))
    {
        // GUI Message:
        // "Service Rate must be greater than zero."
        return result;
    }

    // Queue calculations
    double rho =
        arrivalRate / serviceRate;


    result.setUtilization(rho);


    if(!InputValidator::isSingleServerStable(arrivalRate, serviceRate))
    {
        return result;
    }


    double L = rho / (1 - rho);

    double Lq = (rho * rho) / (1 - rho);

    double W = L / arrivalRate;

    double Wq = Lq / arrivalRate;

    // Store results
    result.setAverageCustomersInSystem(L);
    result.setAverageCustomersInQueue(Lq);
    result.setAverageTimeInSystem(W);
    result.setAverageWaitingTime(Wq);

    return result;
}