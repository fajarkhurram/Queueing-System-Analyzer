#ifndef MG1_H
#define MG1_H

#include "QueueResult.h"

class MG1
{
private:

    double arrivalRate;
    double serviceRate;
    double serviceTimeVariance;

public:

    // Constructor
    MG1(double arrivalRate,
        double serviceRate,
        double serviceTimeVariance);

    // Calculates queue performance measures
    QueueResult calculate();
};

#endif