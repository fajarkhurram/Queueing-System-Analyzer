#ifndef MGS_H
#define MGS_H

#include "QueueResult.h"

class MGS
{
private:

    double arrivalRate;
    double serviceRate;
    int numberOfServers;
    double serviceTimeVariance;

public:

    // Constructor
    MGS(double arrivalRate,
        double serviceRate,
        int numberOfServers,
        double serviceTimeVariance);

    // Calculates queue performance measures
    QueueResult calculate();
};

#endif