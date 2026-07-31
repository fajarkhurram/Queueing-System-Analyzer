#include "InputValidator.h"

// Checks if the value is greater than zero
bool InputValidator::isPositive(double value)
{
    return value > 0;
}

// Checks if the value is zero or greater
bool InputValidator::isNonNegative(double value)
{
    return value >= 0;
}

// Prevents extremely large values
bool InputValidator::isReasonableValue(double value)
{
    return value <= 1000000;
}

// Checks if the number of servers is between 1 and 1000
bool InputValidator::isValidServerCount(int servers)
{
    return servers >= 1 && servers <= 1000;
}

// Stability condition for single-server models
// (M/M/1, M/G/1, G/G/1)
bool InputValidator::isSingleServerStable(double lambda, double mu)
{
    return lambda < mu;
}

// Stability condition for multi-server models
// (M/M/S, M/G/S, G/G/S)
bool InputValidator::isMultiServerStable(double lambda, double mu, int servers)
{
    double rho = lambda / (servers * mu);

    return rho < 1;
}

bool InputValidator::isValidVariance(double variance)
{
    return variance >= 0;
}