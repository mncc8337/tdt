#include "Mesh.h"
#include "dsa/BVHNode.h"
#include "floatcmp.h"
#include "misc/meshloader.h"
#include "misc/RNG.h"

Mesh::Mesh(const std::string name, Material* material, const std::vector<RawTriangle>& src_tris):
    Hittable(name, material), tris(src_tris) {
    if(tris.empty()) return;
    refreshAABB();
    bvh_root = std::make_unique<BVHNode>(tris, 0, tris.size());
}

Mesh::Mesh(const std::string name, Material* material, std::string filename):
    Hittable(name, material) {
    loadMeshFrom(filename, tris);
    if(tris.empty()) return;
    refreshAABB();
    bvh_root = std::make_unique<BVHNode>(tris, 0, tris.size());
}

AABB Mesh::calculateLocalAABB() const {
    AABB box = getNodeAABB(tris[0]);
    for(size_t i = 1; i < tris.size(); i++) {
        box.extend(getNodeAABB(tris[i]));
    }
    return box;
}

HitInfo Mesh::hit(const Ray& ray) const {
    if(!bvh_root) return {};

    Ray local_ray = ray;
    local_ray.origin = transform.pointApplyInverse(local_ray.origin);
    local_ray.direction = transform.dirApplyInverse(local_ray.direction);

    auto hitFunc = [&](const int primitive_idx, HitInfo& current_closest) {
        const auto& tri = tris[primitive_idx];

        float tri_t_max = current_closest.didHit() ? current_closest.distance : FAR_DISTANCE;

        const Vec3& edge1 = tri.edge1;
        const Vec3& edge2 = tri.edge2;

        Vec3 pvec = local_ray.direction.cross(edge2);
        float det = edge1.dot(pvec);
        if(fequal0(det)) return;

        float inv_det = 1.0f / det;
        Vec3 tvec = local_ray.origin - tri.origin;

        float u = tvec.dot(pvec) * inv_det;
        if(u < 0.0f || u > 1.0f) return;

        Vec3 qvec = tvec.cross(edge1);
        float v = local_ray.direction.dot(qvec) * inv_det;
        if(v < 0.0f || u + v > 1.0f) return;

        float dist = edge2.dot(qvec) * inv_det;
        if(dist < EPSILON or dist > tri_t_max) return;

        current_closest.distance = dist;
        current_closest.hit_point = local_ray.point(dist);

        Vec3 outward_normal = edge1.cross(edge2).normalized();

        current_closest.front_face = local_ray.direction.dot(outward_normal) < 0.0f;
        current_closest.normal = current_closest.front_face ? outward_normal : -outward_normal;

        float w = 1.0f - u - v;
        current_closest.tri_uv.x = w * tri.uv0.x + u * tri.uv1.x + v * tri.uv2.x;
        current_closest.tri_uv.y = w * tri.uv0.y + u * tri.uv1.y + v * tri.uv2.y;
    };

    HitInfo closest_hit;

    // use linear search if tri count is small
    if(tris.size() < MESH_BVH_THRESHOLD) {
        for(int i = 0; i < tris.size(); i++)
            hitFunc(i, closest_hit);
    } else {
        closest_hit = bvh_root->traverse(local_ray, hitFunc);
    }

    if(closest_hit.didHit()) {
        closest_hit.material = material;
        closest_hit.hittable = this;
        // convert to world space coordinates
        closest_hit.hit_point = transform.pointApply(closest_hit.hit_point);
        // project the hp-origin vector to the ray direction
        // since ray direction is normalized, it simply equals to its world space distance
        closest_hit.distance = (closest_hit.hit_point - ray.origin).dot(ray.direction);
        closest_hit.normal = transform.normalApply(closest_hit.normal);
    }
    return closest_hit;
}

// no one should ever use this
// and should use rec.tri_uv directly unless rec.tri_uv.x < 0
Vec2 Mesh::getUV(const HitInfo& rec) const {
    return rec.tri_uv;
}

bool Mesh::samplePoint(const Vec3&, SurfaceSample& sample) const {
    if(tris.empty())
        return false;

    // World-space vertices of a triangle. Using world space makes the area
    // correct even when the mesh has a non-uniform scale.
    auto worldTriangle = [&](const RawTriangle& tri, Vec3& a, Vec3& b, Vec3& c) {
        a = transform.pointApply(tri.origin);
        b = transform.pointApply(tri.origin + tri.edge1);
        c = transform.pointApply(tri.origin + tri.edge2);
    };

    float total_area = 0.0f;
    for(const RawTriangle& tri : tris) {
        Vec3 a, b, c;
        worldTriangle(tri, a, b, c);
        total_area += 0.5f * (b - a).cross(c - a).length();
    }
    if(total_area <= EPSILON)
        return false;

    // Pick a triangle proportional to its area.
    float pick = RNG::uniform() * total_area;
    float accum = 0.0f;

    Vec3 a, b, c;
    worldTriangle(tris.back(), a, b, c);

    for(const RawTriangle& tri : tris) {
        Vec3 ta, tb, tc;
        worldTriangle(tri, ta, tb, tc);
        accum += 0.5f * (tb - ta).cross(tc - ta).length();
        if(pick <= accum) {
            a = ta;
            b = tb;
            c = tc;
            break;
        }
    }

    // Uniform barycentric sample inside the chosen triangle.
    float u = RNG::uniform();
    float v = RNG::uniform();
    if(u + v > 1.0f) {
        u = 1.0f - u;
        v = 1.0f - v;
    }

    sample.sample = a + (b - a) * u + (c - a) * v;
    sample.sample_normal = (b - a).cross(c - a).normalized();
    sample.area_pdf = 1.0f / total_area;
    return true;
}
