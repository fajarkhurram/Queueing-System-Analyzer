#ifndef TIMEUNITCONVERTER_H
#define TIMEUNITCONVERTER_H

#include <QString>

// ------------------------------------------------------------------
// TimeUnitConverter
//
// Converts queueing-theory quantities between hours and minutes.
//
// This exists because a "rate" (customers/hour) and a "mean time"
// (hours/customer) do NOT convert the same way when you change time
// units, even though they are just reciprocals of one another:
//
//   RATE-WISE   conversion (hours -> minutes): DIVIDE by 60
//       e.g. lambda = 120 customers/hour  ->  2 customers/minute
//
//   MEAN-WISE   conversion (hours -> minutes): MULTIPLY by 60
//       e.g. W = 0.5 hours/customer       ->  30 minutes/customer
//
//   VARIANCE-WISE conversion (hours^2 -> minutes^2): MULTIPLY by 60^2
//       because variance has units of TIME SQUARED.
//
// Mixing these up is a classic queueing-theory mistake, so this class
// keeps the three conversions clearly separate and named accordingly.
// ------------------------------------------------------------------
class TimeUnitConverter
{
public:
    enum class TimeUnit
    {
        Hours,
        Minutes
    };

    // How many minutes are in one of this unit (Hours -> 60, Minutes -> 1)
    static double minutesPerUnit(TimeUnit unit);

    // Rate-wise conversion: e.g. arrival/service rates (events per unit time)
    static double convertRate(double rate, TimeUnit from, TimeUnit to);

    // Mean-wise conversion: e.g. W, Wq, mean service/inter-arrival time
    // (time per event) -- this is the INVERSE direction of a rate.
    static double convertMean(double meanTime, TimeUnit from, TimeUnit to);

    // Variance-wise conversion: units of time^2 (arrival/service variance)
    static double convertVariance(double variance, TimeUnit from, TimeUnit to);

    // Reciprocal conversion between a RATE and its MEAN TIME
    // (mean = 1/rate). This is a different operation from the hours<->
    // minutes conversions above: those change the time UNIT, this
    // changes the FORM (rate vs. mean) while staying in the same unit.
    // A value of exactly 0 has no defined reciprocal and is passed
    // through as 0 so callers can validate it explicitly instead of
    // getting +inf/NaN back.
    static double toMean(double rate);
    static double toRate(double meanTime);

    // Human-readable suffixes for UI labels, e.g. " /hr", " /min", " hr", " min"
    static QString rateSuffix(TimeUnit unit);
    static QString timeSuffix(TimeUnit unit);
    static QString unitName(TimeUnit unit);
};

#endif // TIMEUNITCONVERTER_H
