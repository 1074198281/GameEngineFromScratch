#include "Encoder/PPM.hpp"
#include "Image.hpp"
#include "Ray.hpp"
#include "geommath.hpp"
#include "random.hpp"
#include "portable.hpp"

#include "Box.hpp"
#include "Sphere.hpp"

#include <memory>
#include <vector>

using float_precision = float;

inline int to_unorm(float_precision f) { return My::clamp(f, decltype(f)(0.0), decltype(f)(0.999)) * 256; }

using ray = My::Ray<float_precision>;
using color = My::Vector3<float_precision>;
using point3 = My::Vector3<float_precision>;
using vec3 = My::Vector3<float_precision>;
using image = My::Image;
constexpr auto infinity = std::numeric_limits<float_precision>::infinity();
constexpr auto epsilon  = std::numeric_limits<float_precision>::epsilon();

My::IntersectableList<float_precision> world;

color ray_color(const ray& r, int depth) {
    My::Hit<float_precision> hit;

    if (depth <= 0) {
        return color({0, 0, 0});
    }

    if (world.Intersect(r, hit, 0.001, infinity)) {
        auto p = r.pointAtParameter(hit.getT());
        // true lambertian
        point3 target = p + hit.getNormal() + My::random_unit_vector<float_precision, 3>();

        // alternative
        // point3 target = p + My::random_in_hemisphere<float_precision, 3>(hit.getNormal());

        return 0.5 * ray_color(ray(p, target - p), depth - 1);
    }

    // background
    auto unit_direction = r.getDirection();
    auto t = 0.5 * (unit_direction[1] + 1.0);
    return (1.0 - t) * color({1.0, 1.0, 1.0}) + t * color({0.5, 0.7, 1.0});
}

template <class T>
class camera {
   public:
    camera() {
        auto aspect_ratio = 16.0 / 9.0;
        T viewport_height = 2.0;
        T viewport_width = aspect_ratio * viewport_height;
        T focal_length = 1.0;

        origin = point3({0, 0, 0});
        horizontal = vec3({viewport_width, 0, 0});
        vertical = vec3({0, viewport_height, 0});
        lower_left_corner =
            origin - horizontal / 2.0 - vertical / 2.0 - vec3({0, 0, focal_length});
    }

    ray get_ray(T u, T v) const {
        return ray(origin,
                   lower_left_corner + u * horizontal + v * vertical - origin);
    }

   private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
};

int main(int argc, char** argv) {
    world.emplace_back(std::make_shared<My::Sphere<float_precision>>(
        0.5, point3({0, 0, -1.0}), color({1.0, 0, 0})));
    world.emplace_back(std::make_shared<My::Sphere<float_precision>>(
        100, point3({0, -100.5, -1.0}), color({0, 0.5, 0})));

    // Image
    const float_precision aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 64;
    const int max_depth = 16;

    // Camera
    camera<float_precision> cam;

    // Image
    image img;
    img.Width = image_width;
    img.Height = image_height;
    img.bitcount = 24;
    img.bitdepth = 8;
    img.pixel_format = My::PIXEL_FORMAT::RGB8;
    img.pitch = (img.bitcount >> 3) * img.Width;
    img.compressed = false;
    img.compress_format = My::COMPRESSED_FORMAT::NONE;
    img.data_size = img.Width * img.Height * (img.bitcount >> 3);
    img.data = new uint8_t[img.data_size];

    // Render
    for (auto j = 0; j < img.Height; j++) {
        std::cerr << "\rScanlines remaining: " << img.Height - j << ' ' << std::flush;
        for (auto i = 0; i < img.Width; i++) {
            color pixel_color(0);
            for (auto s = 0; s < samples_per_pixel; s++) {
                auto u = (i + My::random_f<float_precision>()) / (img.Width - 1);
                auto v = (j + My::random_f<float_precision>()) / (img.Height - 1);
                auto r = cam.get_ray(u, v);
                pixel_color += ray_color(r, max_depth);
            }

            pixel_color = pixel_color * (1.0 / samples_per_pixel);
            
            // Gamma-correction for gamma = 2.0
            pixel_color = My::sqrt(pixel_color);

            img.SetR(i, j, to_unorm(pixel_color[0]));
            img.SetG(i, j, to_unorm(pixel_color[1]));
            img.SetB(i, j, to_unorm(pixel_color[2]));
        }
    }

    std::cerr << "\r";

    My::PpmEncoder encoder;
    encoder.Encode(img);

    return 0;
}
