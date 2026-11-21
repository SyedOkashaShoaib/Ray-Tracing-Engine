#ifndef CAMERA_H
#define CAMERA_H
#include "hittable_list.h"
#include "material.h"
#include "rng.h"
class Camera {
public:
    double aspect_ratio = 1.0;
    int image_width = 100; //pixel units
    int samples_per_pixel = 10; //how many rays intersect one pixel square?
    int max_depth=10; //ray_color() is recursive, it stops when a ray stops hitting a surface. but that could be a lloooong time.
    double vertical_fov = 45; //vertical field of view. Angle between the top and bottom rays originating from the camera center to the viewports ends 
    Point3 look_from = Point3(0,0,0); //point from where the cam looks from 
    Point3 look_at = Point3(0,0,-1); // point where we look at (can also be changed into the direction we are looking at)
    Vec3 v_up = Vec3(0,1,0); //points up relative to the camera (look at the diag in the book) Orthogonal to the view direction
    double  defocus_angle = 0.0; //angle of the cone with apex at viewport center and base (defocus disk) at the camera center. used to control the defocus blur 
    double focus_dist = 1.0; //distance between camera and focus plane. 
    Color background_color = Color(0,0,0); 
    std::uint64_t rng_seed = 42;
    Camera() {}

    Vec3 sample_square(RNG& gen) {
        return Vec3(gen.uniform() - 0.5, gen.uniform() - 0.5, 0);
        /*
        random_double returns between [0,1). subtract 0.5 and your range becomes [-.5,.5). so for each pixel "square"
        the offset lies 0.5 units away from the center. z axis is 0 cause the viewport is flat...This is all done for anti-aliasing.*/
    }
    void renderImage(const Hittable_List& world) {
        std::vector<Color> framebuffer = render(world);
        writeImage(std::cout, framebuffer);
}
    Vec3 ray_color(const Ray& r, const Hittable& world, int depth, RNG& gen) const {
        Hit_Record rec;
        if (depth <= 0) {
            return Color(0,0,0); //black.
        }
        if (!world.Hit(r, Interval(0.001, infinity), rec)){
            /*did the ray hit an object? if it didnt return the background color*/
            return background_color;
        }
        Ray scattered;
        Color attenuation;
        Color color_from_emition = rec.mat->emit(rec.u, rec.v, rec.p); //collect all the light emitted by the material
        if (!rec.mat->scatter(r, scattered,rec, attenuation, gen)) {
            /*does the material incident rays? if not, then its a light emitting material, return its emitted light
            The recursion stops here, and the ray carries the emitted radiance back toward the camera. 
            This is what allows the sphere to act as a source of light.*/
            return color_from_emition;
        }
        Color color_from_scatter = attenuation * ray_color(scattered, world, depth-1, gen);
        return color_from_emition + color_from_scatter;
        /*
        mathematically: 
            Radiance returned by r = (Emition returned by the current material) + (Attenuation from scattering rays) * (radiance returned by the recursive scattered ray)*/
    }

    std::vector<Color> render(const Hittable_List& world) {   
        initialize();
        std::vector<Color> frame_buffer(image_width * image_height);
   // p3 -> ppm file, last 255 tells the max value for r,g,b
        for (int row = 0; row < image_height; row++)
        {
            std::clog << "\rScanlines remaining: " << (image_height - row) <<' '<< std::flush; // clog ->logging stream. \r brings the cursor back to the start of the line. 
        //Formula is easy. std::flush forces clog to immediately flush out the text. (clog buffers data). the trailing space after the formula is neccessary to remove the older digits, as the number gets smaller.
            for (int col = 0; col < image_width; col++)
            {
                std::uint64_t pixel_id = static_cast<std::uint64_t>(row) * static_cast<std::uint64_t>(image_width) + static_cast<std::uint64_t>(col);
                RNG gen(rng_seed + pixel_id);
                Color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    Ray r = get_ray(col, row, gen);
                    pixel_color += ray_color(r, world, max_depth, gen);
                /*
                why are summing up the ray colors? previously, we would pass a ray through the center of a pixel "square" only. one ray per square. This 
                caused jitering, or the stairs effect on object's boundaries. now we are passing multiple rays accross a pixel's square, summing up their
                colors, and dividing by the number of rays(samples) passed. this gives us a degree of smoothness.*/
                }
                frame_buffer[row * image_width + col] = pixel_color * pixel_samples_scale;
                
            }
        }
        std::clog << "\rDone                        \n" << std::flush; 
        return frame_buffer;
    }
    void writeImage(std::ostream& out, const std::vector<Color>& framebuffer) const {
    out << "P3\n"
        << image_width << " "
        << image_height << "\n255\n";

    for (const Color& pixel : framebuffer) {
        write_color(out, pixel);
    }
}

    Ray get_ray(int i, int j, RNG& gen) {
        Vec3 offset= sample_square(gen);
        auto pixel_sample = pixel00_loc + ((i + offset.x() ) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
        auto ray_origin = (defocus_angle  <= 0.0) ? camera_center : defocus_disk_sample(gen);
        auto ray_direction = pixel_sample - ray_origin;
        Ray r(ray_origin, ray_direction); 
        return r;
    }

private:
    int    image_height;   
    Point3 camera_center;         
    Point3 pixel00_loc;    
    Vec3   pixel_delta_u;  
    Vec3   pixel_delta_v;
    Vec3 cam_up, cam_right, cam_backward; //axes descirbing the cameras orientation. each vector has length 1
    Vec3 defocus_disk_u, defocus_disk_v; 
    double pixel_samples_scale ; // division is expensive. do it once. then multiply with this where you were supposed to divide.big brain stuff.
    
    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height; 
        pixel_samples_scale = 1.0 / samples_per_pixel;
        double theta  = degree_to_radian(vertical_fov);
        double h = std::tan(theta/2);
        auto viewport_height = 2*h*focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height); 
    /*
    camera center = where the rays originate from. cordinates: (0,0,0)
    focal length =  distance between camera center and viewport = set to 1
    y-axis goes up, x-axis goes right, z axis goes forward (from the camera center)
    But, in pp3 format, y axis needs to point downwards.
    viewport_u moves from column to column horizontally.
    viewport_v moves from row to row vertically. 
     */
        camera_center = look_from;
        cam_backward = unit(look_from - look_at);
        cam_right = unit(cross(v_up, cam_backward)); //cross product produces a vector that is perpendicular to v1, v2
        cam_up = cross(cam_backward, cam_right);
        auto viewport_u = viewport_width * cam_right;
        auto viewport_v = viewport_height * (-cam_up); //neg cuz we need to invert our y-coordinate to match the pixel grid

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;
    
        auto viewport_upperleft = camera_center - (focus_dist * cam_backward) - (viewport_u / 2 + viewport_v / 2);
        pixel00_loc = viewport_upperleft + 0.5 * (pixel_delta_u + pixel_delta_v);
        auto defocus_radius = focus_dist * std::tan(degree_to_radian(defocus_angle / 2));
        defocus_disk_u = defocus_radius * cam_right;
        defocus_disk_v = defocus_radius *cam_up;
    }
    Point3 defocus_disk_sample(RNG& gen) const {
        // Returns a random point in the camera defocus disk.
        auto p = random_on_unit_disk(gen);
        return (camera_center + (p.x() * defocus_disk_u) + (p.y() * defocus_disk_v));
    }
};

#endif