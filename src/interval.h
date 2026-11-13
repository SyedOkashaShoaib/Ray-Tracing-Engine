#ifndef INTERVAL_H
#define INTERVAL_H

class Interval {
public:
    Interval(double min, double max): min(min), max(max) {}
    double clamp(double x) const{
        if (x < min) {
            return min;
        }
        else if( x > max) {
            return max;
        }
        return x;
    }
private:
    double min;
    double max;
};
#endif