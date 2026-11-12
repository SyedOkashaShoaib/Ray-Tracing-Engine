#include <iostream>
#include "vec3.h"
#include "color.h"
#include "ray.h"
constexpr int image_width  = 256;
constexpr int image_height = 256; //use constexpre instead of macros.
double hit_sphere(const Point3& sphere_center, double radius, const Ray& r) {
    /*
    if the spheres center is not at the origin, then its eq becomes: 
    (Cx−x)^2+(Cy−y)^2+(Cz−z)^2=r^2 (Cx, ..) is the center of the sphere.
    here (x,y,z) belongs to ray r. therefore
    (C - P). (C - P) = r^2 
    if r hits the sphere at any point, then there is a value of t that satisfies the above equation.
    => (C - (Q + t.d)).(C - (Q + t.d)) = r^2
    solving the equation gives us: 
    t^2d⋅d−2td⋅(C−Q)+(C−Q)⋅(C−Q)−r^2=0
    This is a quadratic equation. we can find t using the quadratic formula. 
    if b-4ac is negative -> the ray is not hitting the sphere. 
    if b-4ac is zero, the ray is a tangent
    else the ray intersects the sphere at 2 ponts
    */
    Vec3 oc = sphere_center - r.origin(); //(C - P)
    auto a = dot(r.direction(), r.direction());
    auto h = dot( r.direction(), oc);
    /*
    whats h?? its its an optimization. assume b = -2h. -2h = -2d.(C - Q) */
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = h*h - (a*c);
    if(discriminant < 0.0) {
        return -1.0;
    }
    else {
        return (h-std::sqrt(discriminant) ) / a; 
        /*
        Why not include -b + sqrt(dis) / 2a? because what we hae included gives the closest object that our ray hit.*/
    }
}
Color ray_color(const Ray& r) {
    auto t = hit_sphere(Point3(0,0,-1), 0.5, r);
    if (t > 0.0) {
        Vec3 N = unit(r.at(t) - Vec3(0, 0, -1));
        /*
        surface vector. perpendicular to the point of intersection. After normalizing x,y,z range betwen -1 to 1. 
        but write color expects them between 0-1. N + 1 / 2 does that. we map x to red, y to green, and z to blue for now */
        return 0.5 * Color(N.x() + 1, N.y() + 1, N.z() + 1);
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
void renderImage() {
    
    auto aspect_ratio = 16.0 / 9.0; //we want a rectangular image, and 16:9 is pretty common (apparaently)
    int image_width = 400; 
    int image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height; 

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
    auto camera_center = Point3(0,0,0);
    auto focal_length = 1;
    auto viewport_u = Vec3(viewport_width, 0, 0);
    auto viewport_v = Vec3(0, -viewport_height, 0); //neg cuz we need to invert our y-coordinate to match the pixel grid

    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;
    
    auto viewport_upperleft = camera_center - Vec3(0, 0 , focal_length) - (viewport_u / 2 + viewport_v / 2);
    auto pixel00_loc = viewport_upperleft + 0.5 * (pixel_delta_u + pixel_delta_v);

     std::cout << "P3\n" << image_width << " " << image_height << "\n255\n"; // p3 -> ppm file, last 255 tells the max value for r,g,b
    
    for (int row = 0; row < image_height; row++)
    {
        std::clog << "\rScanlines remaining: " << (image_height - row) <<' '<< std::flush; // clog ->logging stream. \r brings the cursor back to the start of the line. 
        //Formula is easy. std::flush forces clog to immediately flush out the text. (clog buffers data). the trailing space after the formula is neccessary to remove the older digits, as the number gets smaller.
        for (int col = 0; col < image_width; col++)
        {
            auto pixel_center = pixel00_loc + (col * pixel_delta_u) + (row * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            Ray r(camera_center, ray_direction);
            Color pixel_color= ray_color(r);
            write_color(std::cout, pixel_color);
            
        }
    }
    std::clog << "\rDone                        \n" << std::flush; 
}

int main() {
    renderImage();
}