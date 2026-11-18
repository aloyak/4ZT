#pragma once

class Interval {
public:
    double min, max;

    Interval() : min(0), max(0) {}
    Interval(double min, double max) : min(min), max(max) {}

    double Size() const { return max - min; }

    bool Contains(double value) const {
        return value >= min && value <= max;
    }

    bool Surrounds(double value) const {
        return value > min && value < max;
    }
};