#ifndef GGS_H
#define GGS_H

#include "QueueResult.h"

class GGS
{
private:

    double arrivalRate;
    double serviceRate;
    int numberOfServers;
    double arrivalTimeVariance;
    double serviceTimeVariance;

public:

    // Constructor
    GGS(double arrivalRate,
        double serviceRate,
        int numberOfServers,
        double arrivalTimeVariance,
        double serviceTimeVariance);

    // Calculates queue performance measures
    QueueResult calculate();
};

#endif