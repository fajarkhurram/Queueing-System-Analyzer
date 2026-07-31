#ifndef GG1_H
#define GG1_H

#include "QueueResult.h"

class GG1
{
private:

    double arrivalRate;
    double serviceRate;
    double arrivalTimeVariance;
    double serviceTimeVariance;

public:

    // Constructor
    GG1(double arrivalRate,
        double serviceRate,
        double arrivalTimeVariance,
        double serviceTimeVariance);

    // Calculates queue performance measures
    QueueResult calculate();
};

#endif