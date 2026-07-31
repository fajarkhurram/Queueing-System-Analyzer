#include "QueueResult.h"

// Constructor
QueueResult::QueueResult()
{
    utilization = 0.0;
    averageCustomersInSystem = 0.0;
    averageCustomersInQueue = 0.0;
    averageTimeInSystem = 0.0;
    averageWaitingTime = 0.0;
}

// Setters

void QueueResult::setUtilization(double value)
{
    utilization = value;
}

void QueueResult::setAverageCustomersInSystem(double value)
{
    averageCustomersInSystem = value;
}

void QueueResult::setAverageCustomersInQueue(double value)
{
    averageCustomersInQueue = value;
}

void QueueResult::setAverageTimeInSystem(double value)
{
    averageTimeInSystem = value;
}

void QueueResult::setAverageWaitingTime(double value)
{
    averageWaitingTime = value;
}

// Getters

double QueueResult::getUtilization() const
{
    return utilization;
}

double QueueResult::getAverageCustomersInSystem() const
{
    return averageCustomersInSystem;
}

double QueueResult::getAverageCustomersInQueue() const
{
    return averageCustomersInQueue;
}

double QueueResult::getAverageTimeInSystem() const
{
    return averageTimeInSystem;
}

double QueueResult::getAverageWaitingTime() const
{
    return averageWaitingTime;
}