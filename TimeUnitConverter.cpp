#include "TimeUnitConverter.h"

double TimeUnitConverter::minutesPerUnit(TimeUnit unit)
{
    return (unit == TimeUnit::Hours) ? 60.0 : 1.0;
}

// Rate-wise: rate_to = rate_from * minutesPerUnit(to) / minutesPerUnit(from)
// A rate expressed "per hour" becomes a SMALLER number "per minute"
// (there are fewer minutes than hours worth of events in one minute).
double TimeUnitConverter::convertRate(double rate, TimeUnit from, TimeUnit to)
{
    if (from == to)
    {
        return rate;
    }

    return rate * (minutesPerUnit(to) / minutesPerUnit(from));
}

// Mean-wise: mean_to = mean_from * minutesPerUnit(from) / minutesPerUnit(to)
// This is the reciprocal direction of convertRate, because a MEAN TIME
// (time per event) is the inverse of a RATE (events per time).
double TimeUnitConverter::convertMean(double meanTime, TimeUnit from, TimeUnit to)
{
    if (from == to)
    {
        return meanTime;
    }

    return meanTime * (minutesPerUnit(from) / minutesPerUnit(to));
}

// Variance-wise: variance has units of TIME^2, so the conversion factor
// used for a mean time must be applied twice (squared).
double TimeUnitConverter::convertVariance(double variance, TimeUnit from, TimeUnit to)
{
    if (from == to)
    {
        return variance;
    }

    double factor = minutesPerUnit(from) / minutesPerUnit(to);
    return variance * factor * factor;
}

// Reciprocal conversion (rate <-> mean time), NOT a unit conversion.
// mean = 1 / rate, and vice versa.
double TimeUnitConverter::toMean(double rate)
{
    return (rate > 0.0) ? (1.0 / rate) : 0.0;
}

double TimeUnitConverter::toRate(double meanTime)
{
    return (meanTime > 0.0) ? (1.0 / meanTime) : 0.0;
}

QString TimeUnitConverter::rateSuffix(TimeUnit unit)
{
    return (unit == TimeUnit::Hours) ? QStringLiteral(" /hr") : QStringLiteral(" /min");
}

QString TimeUnitConverter::timeSuffix(TimeUnit unit)
{
    return (unit == TimeUnit::Hours) ? QStringLiteral(" hr") : QStringLiteral(" min");
}

QString TimeUnitConverter::unitName(TimeUnit unit)
{
    return (unit == TimeUnit::Hours) ? QStringLiteral("Hours") : QStringLiteral("Minutes");
}
