#ifndef MATERIAL_H
#define MATERIAL_H

#include <algorithm>
#include "hittable.h"
/*read 10.4ish to understand this stuff. i forgot to add comments*/
class Material {
public:
    virtual ~Material() = default;
    virtual bool scatter(const Ray& r_in, Ray& r_out, const Hit_Record& rec, Color& attenuation ) const =0; 
};

class Lambertian: public Material {
public: 
    Lambertian(const Color& albedo): albedo(albedo) {}
    bool scatter(const Ray& r_in, Ray& r_out, const Hit_Record& rec, Color& attenuation) const override {
        /*
        why is t_min 0.001 instead of 0? Because it provides a basic workaround for the shadow acne problem. 
        P is calculated using floating point arithmetic, meaning that the intersection point of the ray could be 
        slighlty above or below the surface of the hittable object. by using t=0.001 we try to mitigate this small floating 
        point inaccuracy.  */
        Vec3 scatter_direction = rec.normal + random_vector_on_hemisphere(rec.normal);
        /*why add the normal vector? Because we are using lambartian distribution. Can i claim that i completely understand 
        this? No. But lambartian distribution claims that a reflected ray is more likely to be closer to the surface normal. 
        lambartian distribution better renders diffuse objs*/
        if (scatter_direction.near_zero()) {
            /*if random unit vector is exactly opposite to normal vector, resultant vector can be a zero vector. very bad*/
            scatter_direction = rec.normal;
        }
        attenuation = albedo;
        r_out = Ray(rec.p, scatter_direction);
        return true;
    }
private:
    const Color albedo;
};

class Metal: public Material {
public:
    Metal (const Color& albedo, double fuzz): albedo(albedo), fuzz(fuzz < 1 ? fuzz: 1) {}
    
    bool scatter(const Ray& r_in, Ray& r_out, const Hit_Record& rec, Color& attenuation) const override  {
        Vec3 reflected_r = reflect(r_in.direction(), rec.normal);
        reflected_r = unit(reflected_r) + fuzz * random_unit_vector();
        r_out = Ray(rec.p, reflected_r);
        attenuation = albedo; 
        return (dot(r_out.direction(), rec.normal) > 0.0);
    }
private:
    const Color albedo;
    const double fuzz;
};
#endif