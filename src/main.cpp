#include <iostream>
#include "vec3.h"
#include "color.h"
constexpr int image_width  = 256;
constexpr int image_height = 256; //use constexpre instead of macros.

void renderImage()
{
    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n"; // p3 -> ppm file, last 255 tells the max value for r,g,b
    
    for (int row = 0; row < image_height; row++)
    {
        std::clog << "\rScanlines remaining: " << (image_height - row) <<' '<< std::flush; // clog ->logging stream. \r brings the cursor back to the start of the line. 
        //Formula is easy. std::flush forces clog to immediately flush out the text. (clog buffers data). the trailing space after the formula is neccessary to remove the older digits, as the number gets smaller.
        for (int col = 0; col < image_width; col++)
        {
            auto pixel_color = Color(double(col) / (image_width - 1),double(row) / (image_height - 1), 0 );
            write_color(std::cout, pixel_color);

            
        }
    }
    std::clog << "\rDone                        \n" << std::flush; 
}

int main()
{
    renderImage();
}