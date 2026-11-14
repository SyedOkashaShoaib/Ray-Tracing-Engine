#ifndef CAMERA_H
#define CAMERA_H
#include "hittable_list.h"


class Camera {
public:
    double aspect_ratio = 1.0;
    int image_width = 100; //pixel units
    int samples_per_pixel = 10; //how many rays intersect one pixel square?
    Camera() {}

    Vec3 sample_square() {
        return Vec3(random_double() - 0.5, random_double() - 0.5, 0);
        /*
        random_double returns between [0,1). subtract 0.5 and your range becomes [-.5,.5). so for each pixel "square"
        the offset lies 0.5 units away from the center. z axis is 0 cause the viewport is flat...This is all done for anti-aliasing.*/
    }
    Vec3 ray_color(const Ray& r, Hittable& world) const {
        Hit_Record rec;
        if (world.Hit(r, 0, infinity, rec)){
            Vec3 direction = random_vector_on_hemisphere(rec.normal);
            return 0.5 * (ray_color(Ray(rec.p, direction), world));
        }
        Vec3 unit_direction = unit(r.direction());
    /*
    Basic implementation where the higher y coordinate, the bluer the pixel, and the lower the y-cord, the whiter the pixel
    y = 1 means blue
    y = 0 means white
    but after normalizing -1 <= y >= 1. We need to linearly scale it to 0-1:
    a =( y + 1 )/ 2
    */
        auto a = 0.5 * (unit_direction.y() + 1.0 );
        return (1 - a) * Color(1.0,1.0,1.0) + a * Color(0.5, 0.7, 1.0); //c1 is white
    }

    void renderImage(Hittable_List& world) {   
        initialize();
        std::cout << "P3\n" << image_width << " " << image_height << "\n255\n"; // p3 -> ppm file, last 255 tells the max value for r,g,b
        for (int row = 0; row < image_height; row++)
        {
            std::clog << "\rScanlines remaining: " << (image_height - row) <<' '<< std::flush; // clog ->logging stream. \r brings the cursor back to the start of the line. 
        //Formula is easy. std::flush forces clog to immediately flush out the text. (clog buffers data). the trailing space after the formula is neccessary to remove the older digits, as the number gets smaller.
            for (int col = 0; col < image_width; col++)
            {
                // auto pixel_center = pixel00_loc + (col * pixel_delta_u) + (row * pixel_delta_v);
                // auto ray_direction = pixel_center - camera_center;
                // Ray r(camera_center, ray_direction);
                // Color pixel_color= ray_color(r, world);
                // write_color(std::cout, pixel_color);
                Color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    Ray r = get_ray(col, row);
                    pixel_color += ray_color(r, world);
                /*
                why are summing up the ray colors? previously, we would pass a ray through the center of a pixel "square" only. one ray per square. This 
                caused jitering, or the stairs effect on object's boundaries. now we are passing multiple rays accross a pixel's square, summing up their
                colors, and dividing by the number of rays(samples) passed. this gives us a degree of smoothness.*/
                }
                write_color(std::cout, pixel_color * pixel_samples_scale);
            }
        }
        std::clog << "\rDone                        \n" << std::flush; 
    }

    Ray get_ray(int i, int j) {
        Vec3 offset= sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x() ) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
        auto ray_direction = pixel_sample - camera_center;
        Ray r(camera_center, ray_direction); 
        return r;
    }

private:
    int    image_height;   
    Point3 camera_center;         
    Point3 pixel00_loc;    
    Vec3   pixel_delta_u;  
    Vec3   pixel_delta_v;
    double pixel_samples_scale ; // division is expensive. do it once. then multiply with this where you were supposed to divide.big brain stuff.
    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height; 
        pixel_samples_scale = 1.0 / samples_per_pixel;
        auto viewport_height = 2.0; //arbitrarily chosen 
        auto viewport_width = viewport_height * (double(image_width) / image_height); 
    /*
    camera center = where the rays originate from. cordinates: (0,0,0)
    focal length =  distance between camera center and viewport = set to 1
    y-axis goes up, x-axis goes right, z axis goes forward (from the camera center)
    But, in pp3 format, y axis needs to point downwards.
    viewport_u moves from column to column horizontally.
    viewport_v moves from row to row vertically. 
     */
        camera_center = Point3(0,0,0);
        auto focal_length = 1;
        auto viewport_u = Vec3(viewport_width, 0, 0);
        auto viewport_v = Vec3(0, -viewport_height, 0); //neg cuz we need to invert our y-coordinate to match the pixel grid

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;
    
        auto viewport_upperleft = camera_center - Vec3(0, 0 , focal_length) - (viewport_u / 2 + viewport_v / 2);
        pixel00_loc = viewport_upperleft + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

};

#endif