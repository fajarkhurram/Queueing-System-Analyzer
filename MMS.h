#ifndef MMS_H
#define MMS_H

#include "QueueResult.h"

class MMS
{
private:

    double arrivalRate;
    double serviceRate;
    int numberOfServers;

    long long factorial(int number);

public:

    // Constructor
    MMS(double arrivalRate,
        double serviceRate,
        int numberOfServers);

    // Calculate queue measures
    QueueResult calculate();
};

#endif