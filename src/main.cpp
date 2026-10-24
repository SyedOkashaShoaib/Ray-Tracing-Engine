#include <iostream>
#define image_width 256
#define image_height 256

void renderImage() {
    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n"; // p3 -> ppm file, last 255 tells the max value for r,g,b
    for (int i = 0; i < image_height; i++) {
        for (int j = 0; j < image_height; j++) {
            double r = double(j) / (image_width - 1); //from left to right red slowly intensifies
            double g = double(i) / (image_height - 1); // from top to bottom
            double b = 0; //no blue for now :(

            int int_r = int(255.999 * r); //normalizing for values to range between 0-255
            int int_g = int(255.999 * g); // 255.999 maps the value upto 255 in a more cleaner manner.
            int int_b = int(255.999 * b);

            std::cout << int_r << ' ' << int_g << ' ' << int_b << '\n';
        }
    }
}


int main() {
   renderImage();
}