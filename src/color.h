#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "utilities.h"
#include "interval.h"
using Color = Vec3;
//monkey write color.
inline void write_color(std::ostream& out, const Color& pixel_color) {
    double r = pixel_color.x();
    double g = pixel_color.y();
    double b = pixel_color.z();

    //converting into range of 0-255
/* 
Why are facing the need for this conversino? 
Because ray tracing engine calculates color in floating point numbers between 0-1
0.0 -> No contribution
1.0 -> Full contribution
PPM image format expects rgb values in the int range of 0-255 in our implementation */
    Interval interval(0.000, 0.999);
    int r_byte = int(255.999 * interval.clamp(r));
    int g_byte = int(255.999 * interval.clamp(g));
    int b_byte = int(255.999 * interval.clamp(b));

    out << r_byte << " " << g_byte << " " << b_byte << "\n";
/* Why not use std::cout << r_byte... like a good little boy? Because then we would only be able to write on the 
output stream. we would have to use ./raytracing > image.ppm to write to the ppm file. Right now we can either write
to the output stream, or to the ppm file directly.*/
}
#endif