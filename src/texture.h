#ifndef TEXTURE_H
#define TEXTURE_H

#include <memory>
#include <cmath>
#include "vec3.h"
#include "color.h"
#include "image_helper.h"
#include "perlin.h"
class Texture {
public:
    virtual ~Texture() = default;
    virtual Color value(double u, double v, const Point3 &p) const = 0;
    /*
    u and v are used as coordinates for 2d textures. p is for 3d texture mapping.
    spheres that are solid colored will not use any of the arguments*/
};

class Solid_Color: public Texture {
public:
    Solid_Color(const Color &albedo) {
        this->albedo = albedo;
    }
    Solid_Color(double r, double g, double b): Solid_Color(Color(r, g, b)) {}

    Color value(double , double , const Point3& ) const override {
        return albedo;
    }
private:
    Color albedo;
};

class Checkered_Texture: public Texture {
public:
    Checkered_Texture(double scale, std::shared_ptr<Texture> even, std::shared_ptr<Texture> odd) {
        inverse_scale = 1 / scale;
        this->even = even;
        this->odd = odd;
    }
    Checkered_Texture(double scale, const Color& even, const Color& odd): Checkered_Texture(scale, std::make_shared<Solid_Color>(even), std::make_shared<Solid_Color>(odd)) {}
    Color value( double u, double v, const Point3& p) const override {
        auto xInteger = int(std::floor(inverse_scale * p.x()));
        auto yInteger = int(std::floor(inverse_scale * p.y()));
        auto zInteger = int(std::floor(inverse_scale * p.z()));
        bool isEven = ((xInteger + yInteger + zInteger) % 2 == 0);
        return (isEven) ? even->value(u, v, p) : odd->value(u,v,p);

    }
private:
    std::shared_ptr<Texture> even;
    std::shared_ptr<Texture> odd;
    double inverse_scale;
};

class Image_Texture: public Texture {
public: 
    Image_Texture(const char* filename): image(filename) {}
    Color value( double u, double v, const Point3&) const override {
        if (image.height() <= 0) { return Color(0,1,1);} //if the image does not exist. return any color. were returninc cyan cuz we lowkey gay 
        /*THe image will obv not have coordinates between 0-1 like u,v do. we therefore need to map them:*/
        u = Interval(0,1).clamp(u); //this is an additional safeguard to ensure uv are in 0-1. even though uv are already within the range 0-1 if the came from spheres get uv coord method
        v = 1.0 - Interval(0,1).clamp(v);
        auto i = int(u * image.width());
        auto j = int(v * image.height());
        auto pixel = image.pixel_data(i,j); //returns r,g,b values within range 0-255
        auto color_scale = 1.0 / 255;//needed for normalizing pixel colors because Color class expects rgb values within 0-1
        return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);

 }
private:
    Image_Helper image;
};

class Noise_Texture: public Texture {
public: 
    Noise_Texture(double scale): scale(scale){}
    Color value(double , double, const Point3& p) const override {
        /*sin along z axis gives a color scale which transitions to different colors along the sinosoidal wave.
        but obviosuly the pattern made is deterministic and regular. to add randomness to it, we add turbulence.
        this equation gives us a marble like object. Note if you are changing Color() in the below eq, normalize 
        the remaining eq by multiplying by 0.5*/
        return Color(0.5, 0.5, 0.5)* (1 + std::sin(scale*p.z() + 10 * noise.turbulence(p, 7)));
        /*perlin class noise() gives us a double between 0-1. this class converts it into a color value */
 
    }
private:
    Perlin noise;
    double scale; //controls the frequency of the patterns
    /*consider two points p1, p2 that are only 0.1 units apart. without scaling, these two points are placed
    within the same unit cube inside the noise lattice. but after multiplying by scale, there displacement increases
    the actual distance is still 0.1 units, but noise function sees them further apart. this leads to more unit cells
    and more patterns*/
    // Color color1 = Color(0.1, 0.2, 0.8); // Blue
    // Color color2= Color(1.0, 0.5, 0.1); // Orange
};
#endif