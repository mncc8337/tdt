#include "Mesh.h"
#include "floatcmp.h"
#include "misc/meshloader.h"

Mesh::Mesh(const std::string name, Material* material, const std::vector<RawTriangle>& src_tris):
    Hittable(name, material), tris(src_tris) {
    if(tris.empty()) return;
    computeAABB();
    bvh_root = std::make_unique<BVHNode>(tris, 0, tris.size());
}

Mesh::Mesh(const std::string name, Material* material, std::string filename):
    Hittable(name, material) {
    loadMeshFrom(filename, tris);
    if(tris.empty()) return;
    computeAABB();
    bvh_root = std::make_unique<BVHNode>(tris, 0, tris.size());
}

void Mesh::computeAABB() {
    aabb = getNodeAABB(tris[0]);
    for(size_t i = 1; i < tris.size(); i++) {
        aabb.extend(getNodeAABB(tris[i]));
    }
}

HitInfo Mesh::hit(const Ray& ray) const {
    if(!bvh_root) return {};

    Ray local_ray = ray;
    local_ray.origin = transform.pointApplyInverse(local_ray.origin);
    local_ray.direction = transform.dirApplyInverse(local_ray.direction);

    auto hitFunc = [&](const int primitive_idx, HitInfo& current_closest) {
        const auto& tri = tris[primitive_idx];

        float tri_t_max = DID_HIT(current_closest) ? current_closest.distance : FAR_DISTANCE;

        Vec3 edge1 = tri.v1 - tri.v0;
        Vec3 edge2 = tri.v2 - tri.v0;

        Vec3 pvec = local_ray.direction.cross(edge2);
        float det = edge1.dot(pvec);
        if(fequal0(det)) return;

        float inv_det = 1.0f / det;
        Vec3 tvec = local_ray.origin - tri.v0;

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
        current_closest.uv.x = w * tri.uv0.x + u * tri.uv1.x + v * tri.uv2.x;
        current_closest.uv.y = w * tri.uv0.y + u * tri.uv1.y + v * tri.uv2.y;
    };

    HitInfo closest_hit = bvh_root->traverse(local_ray, hitFunc);

    if(DID_HIT(closest_hit)) {
        closest_hit.material = material;
        // convert to world space coordinates
        closest_hit.hit_point = transform.pointApply(closest_hit.hit_point);
        // project the hp-origin vector to the ray direction
        // since ray direction is normalized, it simply equals to its world space distance
        closest_hit.distance = (closest_hit.hit_point - ray.origin).dot(ray.direction);
        closest_hit.normal = transform.normalApply(closest_hit.normal);
    }
    return closest_hit;
}

AABB Mesh::getLocalAABB() const {
    return aabb;
}
