#ifndef QUEUERESULT_H
#define QUEUERESULT_H

class QueueResult
{
private:
    double utilization;
    double averageCustomersInSystem;
    double averageCustomersInQueue;
    double averageTimeInSystem;
    double averageWaitingTime;

public:
    // Constructor
    QueueResult();

    // Setters
    void setUtilization(double value);
    void setAverageCustomersInSystem(double value);
    void setAverageCustomersInQueue(double value);
    void setAverageTimeInSystem(double value);
    void setAverageWaitingTime(double value);

    // Getters
    double getUtilization() const;
    double getAverageCustomersInSystem() const;
    double getAverageCustomersInQueue() const;
    double getAverageTimeInSystem() const;
    double getAverageWaitingTime() const;
};

#endif