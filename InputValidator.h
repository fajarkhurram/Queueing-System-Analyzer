#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

class InputValidator
{
public:

    //general checks
    static bool isPositive(double value);
    static bool isNonNegative(double value);
    static bool isReasonableValue(double value);

    //valid varience check to be use
    static bool isValidVariance(double variance);

    //server validation
    static bool isValidServerCount(int servers);

    //stability checks
    static bool isSingleServerStable(double lambda, double mu);
    static bool isMultiServerStable(double lambda, double mu, int servers);
};

#endif