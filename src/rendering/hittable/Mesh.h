#pragma once

#include "Hittable.h"
#include "math/RawTriangle.h"
#include "dsa/BVHNode.h"

#define MESH_BVH_THRESHOLD 4

class Mesh: public Hittable {
private:
    std::vector<RawTriangle> tris;
    std::unique_ptr<BVHNode> bvh_root;
    // kept for scene serialization: empty when built from raw triangles
    std::string filename;

    AABB calculateLocalAABB() const override;

public:
    Mesh(unsigned id, const std::string name, Material* material, const std::vector<RawTriangle>& tris);
    Mesh(unsigned id, const std::string name, Material* material, std::string filename);

    const std::string& getFilename() const;
    const std::vector<RawTriangle>& getTriangles() const;

    HitInfo hit(const Ray& ray) const override;
    Vec2 getUV(const HitInfo& rec) const override;
    bool samplePoint(const Vec3& viewpoint, SurfaceSample& sample) const override;
};
