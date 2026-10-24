#include <iostream>
constexpr int image_width  = 256;
constexpr int image_height = 256; //use constexpre instead of macros.

void renderImage()
{
    std::cout << "P3\n"
              << image_width << " " << image_height << "\n255\n"; // p3 -> ppm file, last 255 tells the max value for r,g,b
    for (int row = 0; row < image_height; row++)
    {
        std::clog << "\rScanlines remaining: " << (image_height - row) <<' '<< std::flush; // clog ->logging stream. \r brings the cursor back to the start of the line. 
        //Formula is easy. std::flush forces clog to immediately flush out the text. (clog buffers data). the trailing space after the formula is neccessary to remove the older digits, as the number gets smaller.
        for (int col = 0; col < image_width; col++)
        {
            double r = double(col) / (image_width - 1);  // from left to right red slowly intensifies
            double g = double(row) / (image_height - 1); // from top to bottom
            double b = 0;                              // no blue for now :(

            int int_r = int(255.999 * r); // normalizing for values to range between 0-255
            int int_g = int(255.999 * g); // 255.999 maps the value upto 255 in a more cleaner manner.
            int int_b = int(255.999 * b);

            std::cout << int_r << ' ' << int_g << ' ' << int_b << '\n';
        }
    }
    std::clog << "\rDone                        \n" << std::flush; 
}

int main()
{
    renderImage();
}