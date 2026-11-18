#ifndef INTERVAL_H
#define INTERVAL_H

class Interval {
public:
    double min;
    double max;
    Interval():min(-infinity), max(+infinity) {}
    Interval(double min, double max): min(min), max(max) {}
    Interval(const Interval& a, const Interval& b)  {
        min = (a.min <= b.min) ? a.min : b.min;
        max = (a.max >= b.max) ? a.max : b.max;
    }
    bool surrounds(double x) const {
        return min < x && x < max;
    }
    double clamp(double x) const{
        if (x < min) {
            return min;
        }
        else if( x > max) {
            return max;
        }
        return x;
    }
    Interval padding(double delta) {
    auto padding = delta / 2;
    return Interval(min - padding, max + padding);
    }
    static const Interval empty, universe;

};
#endif