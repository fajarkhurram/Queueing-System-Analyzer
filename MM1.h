#ifndef MM1_H
#define MM1_H

#include "QueueResult.h"

class MM1
{
private:
    double arrivalRate;
    double serviceRate;

public:

    // Constructor
    MM1(double arrivalRate, double serviceRate);

    // Calculates all queue performance measures
    QueueResult calculate();
};

#endif