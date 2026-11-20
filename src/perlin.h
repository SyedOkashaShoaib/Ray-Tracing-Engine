#ifndef PERLIN_H
#define PERLIN_H

#include "core_utilities.h"
#include "vec3.h"
class Perlin {
public:
    Perlin() {
        for(int i = 0; i < point_count; i++) {
            randvector[i] = unit(random_vector(-1,1));
        }
        perlin_generate_perm(perm_x);
        perlin_generate_perm(perm_y);
        perlin_generate_perm(perm_z);
    }
    double noise(const Point3 &p) const {
        Vec3 c[2][2][2];
        /*a unit cube. each index of the array represents a corner of that cube. each index can have a value 0-1
        The point P is inside this unit cube. in order to assign it a color, we are going to take a weighted average
        of all corners values*/
        auto u = p.x() - std::floor(p.x()); //These values tell us where P lies inside its unit cube.
        auto v = p.y() - std::floor(p.y());
        auto w = p.z() - std::floor(p.z());
 
        
        auto i = int(std::floor(p.x()));
        auto j = int(std::floor(p.y()));
        auto k = int(std::floor(p.z()));
        /*i,j,k are used to identify which unit cube our point p is inside(we divide the whole scene into unit cubes).
        This why we are using unit cubes; they make computation simpler*/
        for (int di=0; di < 2; di++)
            for (int dj=0; dj < 2; dj++)
                for (int dk=0; dk < 2; dk++)
                    c[di][dj][dk] = randvector[
                        perm_x[(i+di) & 255] ^
                        perm_y[(j+dj) & 255] ^
                        perm_z[(k+dk) & 255]
                    ]; //assiging random values to all the corners. 
        return perlin_interpolatoin(c, u, v, w);

    }

    double turbulence(const Point3 &p, int depth) const {
        /*this method allows us to superimpose different frequencies of perlin noise variations to render 
        even finer details on our objects.
        depth refers to the number of noise layers we are creating*/
        auto accum = 0.0;
        auto temp_p = p;
        auto weight = 1.0;
        for (int i =0; i< depth; i++) {
            /*
            the first layer creates the large scale variations wuth subsequent layeres creating finer details. 
            that is why the weight is max at i = 0. because we want large scale variations with progressively 
            finer details. that is why at each iteratoin the weight(1/frequency) is halved*/
            accum+= weight *noise(temp_p);
            temp_p *= 2.0;
            weight *= 0.5;
        }

        return std::fabs(accum);
    }
private:
    static const int point_count = 256;
    Vec3 randvector[point_count];
    int perm_x[point_count];
    int perm_y[point_count];
    int perm_z[point_count];

    static void permute(int* p, int n) {
        for (int i = n-1; i > 0; i--) {
            int target = random_int(0, i);
            int temp = p[i];
            p[i] = p[target];
            p[target] = temp;

        }
    }
    static void perlin_generate_perm(int *p) {
        for (int i = 0; i < point_count; i++) {
            p[i] = i;
        }
        permute(p, point_count);
    }
    static double perlin_interpolatoin(Vec3 c[2][2][2], double u, double v, double w) {
        /*hermitian smoothing to further smoothen the result of trilinear interp. eq f(t) = t^2(3-2t)*/
        auto uu = u*u*(3-2*u);
        auto vv = v*v*(3-2*v);
        auto ww = w*w*(3-2*w);   
        /*linear interpolation is used to provide a smooth transition of values between two points a and b 
        in a 2d space. trilinear interp extends that to 3d space
        When a ray intersects a surface at point P, we find the unit cube (we divide the scenes into unit cubes) 
        containing P, retrieve the random values at that cube's eight corners, and interpolate between them.*/
        auto accum = 0.0;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++){
                Vec3 displacement(u-i, v-j, w-k);
                    accum +=
                        (i*uu + (1-i)*(1-uu)) *
                        (j*vv + (1-j)*(1-vv)) *
                        (k*ww + (1-k)*(1-ww)) *
                        dot(c[i][j][k], displacement);
                    }

        return accum;
    }
};

#endif