#include "RayTracer.h"
#include "Camera.h"
#include "HitInfo.h"
#include "Vec3.h"

#define PIXEL2WORLD(px) (px/1000.0f)

RayTracer::RayTracer(
    unsigned viewport_width,
    unsigned viewport_height,
    Camera& camera,
    Scene& scene
):
    viewport_width(viewport_width),
    viewport_height(viewport_height),
    camera(camera),
    scene(scene),
    pixels(std::vector<std::uint32_t>(viewport_width * viewport_height)) {}

const std::uint8_t* RayTracer::getData() const {
    return reinterpret_cast<const std::uint8_t*>(pixels.data());
}

const Color RayTracer::trace(Ray ray) const {
    Color attenuation(1, 1, 1);
    Color ray_color(0, 0, 0);

    // test light src
    Vec3 light_pos(-3, 2, -5); 

    for(unsigned bounces = 0; bounces < max_bounces; bounces++) {
        HitInfo rec = scene.get_closest(ray);
        if(rec.object) {
            Vec3 light_dist = light_pos - rec.hit_point;
            Vec3 light_dir = light_dist.normalized();
            float diffuse_intensity = 1.3 * std::max(0.0f, rec.normal.dot(light_dir));
            ray_color = Color(1, 1, 1) * diffuse_intensity / light_dist.length_squared();
            break;
        }
    }

    return ray_color;
}

void RayTracer::render() {
    float viewport_h = 2.0f; 
    float viewport_w = viewport_h * ((float)viewport_width / viewport_height);

    for(unsigned j = 0; j < viewport_height; j++) {
        for(unsigned i = 0; i < viewport_width; i++) {
            float u = (i - viewport_width / 2.0f) / viewport_width;
            float v = (j - viewport_height / 2.0f) / viewport_height;

            float px = u * viewport_w;
            float py = -v * viewport_h;
            
            Color cl = trace(camera.getRayAt(px, py));
            pixels[i + j * viewport_width] = cl.toABGR();
        }
    }
}
