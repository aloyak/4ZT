#pragma once

#include <algorithm>
#include <limits>

class Interval {
public:
    double min, max;

    // Default empty
    Interval() : min(+infinity), max(-infinity) {}
    Interval(double min, double max) : min(min), max(max) {}

    // Constructs an interval that surrounds both a and b
    Interval(const Interval& a, const Interval& b)
        : min(std::min(a.min, b.min)), max(std::max(a.max, b.max)) {}

    double Size() const { return max - min; }

    bool Contains(double value) const { 
        return value >= min && value <= max; 
    }

    bool Surrounds(double value) const { 
        return value > min && value < max; 
    }

private:
    static constexpr double infinity = std::numeric_limits<double>::infinity();
};