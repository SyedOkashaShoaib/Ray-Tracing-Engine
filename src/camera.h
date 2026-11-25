#ifndef CAMERA_H
#define CAMERA_H
#if defined(RT_USE_OPENMP) && !defined(_OPENMP)
#error "RT_USE_OPENMP requires compiling with -fopenmp"
#endif
/*RT_USE_OPENMP -> our defined macro to ask the compiler to use openmp.
_OPENMP-> macro predefined to enable openmp usage */
#if defined(RT_USE_MPI)
#include <mpi.h>
#endif
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <type_traits>
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
        #if defined(RT_USE_MPI)

        renderImageMPI(world);

        #else
            using Clock = std::chrono::steady_clock;
            const auto init_start = Clock::now();
            initialize();
            const auto init_end = Clock::now();
            const auto render_start = Clock::now();
            std::vector<Color> framebuffer = render(world);
            const auto render_end = Clock::now();
            const auto output_start = Clock::now();
            writeImageToConfiguredOutput(framebuffer);
            const auto output_end = Clock::now();
                    const double init_seconds =
            std::chrono::duration<double>( init_end - init_start).count();
            const double render_seconds = std::chrono::duration<double>( render_end - render_start).count();
            const double output_seconds = std::chrono::duration<double>( output_end - output_start).count();

       
            std::clog << std::fixed << std::setprecision(9)
                      << "RT_TIMING component=camera"
                      << " camera_init_s=" << init_seconds
                      << " render_s=" << render_seconds
                      << " gather_s=0.000000000"
                      << " output_s=" << output_seconds
                      << " aggregate=single_process"
                      << '\n';
        #endif
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
        
    return renderRowsInitialized( world, 0, image_height);

    }

    #if defined(RT_USE_MPI)
    void renderImageMPI(const Hittable_List& world) {
        int rank;
        int world_size;

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &world_size); //comm_size = world_size
        const auto init_start = MPI_Wtime();
        initialize(); //each mpi proc needs to initialize the same configs cause no shared memory
        const auto init_seconds = MPI_Wtime() - init_start;
         // Divide image rows among MPI ranks.
        const int base = image_height / world_size; //min rows each proc gets
        const int remainder = image_height % world_size;
        /*remainder can neber be greater than world_size - 1. therefore it cant be greater than the number of process. THe 
        first remaninder number of processes get one row extra.*/

        const int local_rows = base + (rank < remainder ? 1 : 0);

        const int start_row = rank * base + std::min(rank, remainder);

        const int end_row = start_row + local_rows;

        // Each rank renders only its assigned rows.
        const auto render_start = MPI_Wtime();
        std::vector<Color> local_pixels = renderRowsInitialized(world, start_row, end_row);
        const auto render_seconds = MPI_Wtime() - render_start;
    // Our Color (Vec3) contains three contiguous doubles.
        static_assert(std::is_standard_layout_v<Color>);
        static_assert(sizeof(Color) == 3 * sizeof(double));
        /*we are verifying that the colors memroy layout is 3 * double becaues we need to create a custom MPI datatype. */

        MPI_Datatype mpi_color;
        MPI_Type_contiguous(3, MPI_DOUBLE, &mpi_color);
        /*contigouus lays out 3 doubles back to back in memroy with no gaps in between*/
        MPI_Type_commit(&mpi_color); //we register the derived datatype

        std::vector<int> receive_counts;
        std::vector<int> displacements;
        std::vector<Color> full_image;

        if (rank == 0) {
            receive_counts.resize(world_size);
            /*how many pixels will rank 0 recieve from each rank?*/
            displacements.resize(world_size);
            /*at what index does does rank 0 have to place rank is first sent pixel in the final framebuffer?*/

            full_image.resize(static_cast<std::size_t>(image_width) * static_cast<std::size_t>(image_height));

        for (int r = 0; r < world_size; r++) {
            const int rows =
                base + (r < remainder ? 1 : 0);

            const int first_row =
                r * base + std::min(r, remainder);

            receive_counts[r] = rows * image_width;
            displacements[r] = first_row * image_width;
        }
    }

    // Collect all pixel blocks on rank 0.
        const double gather_start = MPI_Wtime();
        MPI_Gatherv(
            local_pixels.data(),
            static_cast<int>(local_pixels.size()),
            mpi_color,
            rank == 0 ? full_image.data() : nullptr,
            rank == 0 ? receive_counts.data() : nullptr,
            rank == 0 ? displacements.data() : nullptr,
            mpi_color,
            0,
            MPI_COMM_WORLD
        );
        const double gather_seconds = MPI_Wtime() - gather_start;
        /*why are we using mpi gatherv instead of mpi gather? 
        because in mpi gather, the recieve count is an integer. meaing that each sending process can send a constant recvcount amount 
        of data. in mpigatherv, recieve count is an array. discp is an additional array that specifiies where eachprocess's 
        contribution begins in the receiving buffer.*/

        MPI_Type_free(&mpi_color); //free the registered custom datatype

        double max_init_seconds = 0.0;
        double max_render_seconds = 0.0;
        double max_gather_seconds = 0.0;

    // Collect the maximum elapsed time across all ranks.
        MPI_Reduce(&init_seconds, &max_init_seconds,1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce( &render_seconds, &max_render_seconds, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&gather_seconds, &max_gather_seconds, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        if (rank == 0) {

        const double output_start = MPI_Wtime();

        writeImageToConfiguredOutput(full_image);

        const double output_seconds = MPI_Wtime() - output_start;

        std::clog << std::fixed << std::setprecision(9)
                  << "RT_TIMING component=camera"
                  << " camera_init_s=" << max_init_seconds
                  << " render_s=" << max_render_seconds
                  << " gather_s=" << max_gather_seconds
                  << " output_s=" << output_seconds
                  << " aggregate=rank_max"
                  << '\n';
        }
    }

    #endif
    void writeImage(std::ostream& out, const std::vector<Color>& framebuffer) const {
    out << "P3\n"
        << image_width << " "
        << image_height << "\n255\n";  // p3 -> ppm file, last 255 tells the max value for r,g,b

    for (const Color& pixel : framebuffer) {
        write_color(out, pixel);
    }
}

void writeImageToConfiguredOutput(
    const std::vector<Color>& framebuffer
) const {

    const char* path = std::getenv("RT_IMAGE_PATH");

    if (path != nullptr && *path != '\0') {

        std::ofstream file(path, std::ios::out | std::ios::trunc);

        if (!file) {
            throw std::runtime_error("Cannot open image output file");
        }

        writeImage(file, framebuffer);

        file.close();

        if (!file) {
            throw std::runtime_error("Failed to write image output file");
        }

    } else {

        writeImage(std::cout, framebuffer);
        std::cout.flush();

        if (!std::cout) {
            throw std::runtime_error("Failed to write image to stdout");
        }
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

    std::vector<Color> renderRowsInitialized( const Hittable_List& world, int start_row, int end_row) {
        const std::size_t count = static_cast<std::size_t>(end_row - start_row) * static_cast<std::size_t>(image_width);
        std::vector<Color> pixels(count);

        #if defined(RT_USE_OPENMP)
            #pragma omp parallel for schedule(runtime)
        #endif
        for (int row = start_row; row < end_row; row++) {

            for (int col = 0; col < image_width; col++) {

                std::uint64_t pixel_id = static_cast<std::uint64_t>(row) * static_cast<std::uint64_t>(image_width) + static_cast<std::uint64_t>(col);

                RNG gen(rng_seed + pixel_id);
                Color pixel_color(0, 0, 0);

                for (int sample = 0; sample < samples_per_pixel; sample++) {

                    Ray r = get_ray(col, row, gen);

                    pixel_color += ray_color( r, world, max_depth, gen);
            }

                const std::size_t local_index = static_cast<std::size_t>( row - start_row) * image_width + col;
                pixels[local_index] = pixel_color * pixel_samples_scale;
        }
    }

    return pixels;
}
};

#endif