#include "SceneLoader.h"
#include "Scene.h"
#include "Camera.h"
#include "hittable/Mesh.h"
#include "hittable/Sphere.h"
#include "material/Dielectric.h"
#include "material/FineParticle.h"
#include "material/Light.h"
#include "material/Matte.h"
#include "material/Metal.h"
#include "medium/Homogeneous.h"
#include "texture/ImageTexture.h"
#include "texture/SolidTexture.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <random>
#include <stdexcept>
#include <unordered_map>

namespace {

using json = nlohmann::json;

constexpr std::uint32_t SCENE_FORMAT_VERSION = 1;
constexpr char SCENE_MAGIC[4] = {'T', 'D', 'S', 'C'};

json vecToJson(const Vec3& v) {
    return json::array({v.x, v.y, v.z});
}

Vec3 vecFromJson(const json& value) {
    return Vec3(
        value.at(0).get<float>(),
        value.at(1).get<float>(),
        value.at(2).get<float>()
    );
}

std::optional<unsigned> refID(const json& node, const char* key) {
    if(!node.contains(key) or node.at(key).is_null())
        return std::nullopt;
    return node.at(key).get<unsigned>();
}

const char* mediumType(const Medium* medium) {
    if(dynamic_cast<const Homogeneous*>(medium)) return "Homogeneous";
    return "Medium";
}

const char* textureType(const Texture* texture) {
    if(dynamic_cast<const ColorTexture*>(texture)) return "ColorTexture";
    if(dynamic_cast<const ImageTexture*>(texture)) return "ImageTexture";
    return nullptr;
}

const char* materialType(const Material* material) {
    if(dynamic_cast<const Matte*>(material)) return "Matte";
    if(dynamic_cast<const Metal*>(material)) return "Metal";
    if(dynamic_cast<const Light*>(material)) return "Light";
    if(dynamic_cast<const Dielectric*>(material)) return "Dielectric";
    if(dynamic_cast<const FineParticle*>(material)) return "FineParticle";
    return nullptr;
}

const char* objectType(const Hittable* object) {
    if(dynamic_cast<const Sphere*>(object)) return "Sphere";
    if(dynamic_cast<const Mesh*>(object)) return "Mesh";
    return nullptr;
}

void appendU32(std::vector<std::uint8_t>& blob, std::uint32_t value) {
    for(int i = 0; i < 4; i++)
        blob.push_back(std::uint8_t(value >> (i * 8)));
}

std::uint32_t readU32(const std::vector<std::uint8_t>& blob, std::size_t offset) {
    std::uint32_t value = 0;
    for(int i = 0; i < 4; i++)
        value |= std::uint32_t(blob[offset + i]) << (i * 8);
    return value;
}

void appendFloat(std::vector<std::uint8_t>& blob, float value) {
    appendU32(blob, std::bit_cast<std::uint32_t>(value));
}

float readFloat(const std::vector<std::uint8_t>& blob, std::size_t offset) {
    return std::bit_cast<float>(readU32(blob, offset));
}

// read a file verbatim into the blob section and describe its location in json
bool appendAsset(
    std::vector<std::uint8_t>& blobs,
    const std::filesystem::path& filename,
    json& asset
) {
    std::ifstream file(filename, std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "scene save: cannot open asset " << filename << '\n';
        return false;
    }

    const std::vector<std::uint8_t> bytes{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    asset = {
        {"offset", blobs.size()},
        {"size", bytes.size()},
        {"filename", filename.filename().string()}
    };
    blobs.insert(blobs.end(), bytes.begin(), bytes.end());
    return true;
}

void removeTempAssets(const std::vector<std::filesystem::path>& assets) {
    std::error_code ec;
    for(const auto& path : assets)
        std::filesystem::remove(path, ec);
}

// std-only cross-platform temp file naming
std::filesystem::path makeTempPath(const std::string& extension) {
    static std::mt19937_64 rng(std::random_device{}());
    return std::filesystem::temp_directory_path() /
        ("tdt_scene_" + std::to_string(rng()) + extension);
}

std::string imageExtension(const std::string& original) {
    std::string ext = std::filesystem::path(original).extension().string();
    std::transform(
        ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return char(std::tolower(c)); }
    );
    // sf::Image needs a known extension to pick a decoder
    if(ext == ".png" or ext == ".jpg" or ext == ".jpeg" or ext == ".bmp" or
       ext == ".tga" or ext == ".gif" or ext == ".psd" or ext == ".hdr" or
       ext == ".pic" or ext == ".pnm" or ext == ".ppm" or ext == ".pgm")
        return ext;
    return ".png";
}

bool extractAsset(
    const json& asset,
    const std::vector<std::uint8_t>& blobs,
    const std::string& extension,
    std::filesystem::path& path,
    std::vector<std::filesystem::path>& temp_assets
) {
    const std::size_t offset = asset.at("offset").get<std::size_t>();
    const std::size_t size = asset.at("size").get<std::size_t>();
    if(offset > blobs.size() or size > blobs.size() - offset)
        return false;

    path = makeTempPath(extension);
    temp_assets.push_back(path);
    std::ofstream file(path, std::ios::binary);
    file.write(
        reinterpret_cast<const char*>(blobs.data() + offset),
        static_cast<std::streamsize>(size)
    );
    return bool(file);
}

std::vector<RawTriangle> readRawTriangles(
    const json& asset,
    const std::vector<std::uint8_t>& blobs
) {
    const std::size_t offset = asset.at("offset").get<std::size_t>();
    const std::size_t size = asset.at("size").get<std::size_t>();
    if(offset > blobs.size() or size > blobs.size() - offset or size % (15u * 4u) != 0)
        throw std::runtime_error("scene load: malformed raw mesh blob");

    const std::size_t count = size / (15u * 4u);
    std::vector<RawTriangle> tris;
    tris.reserve(count);

    for(std::size_t i = 0; i < count; i++) {
        const std::size_t base = offset + i * 15u * 4u;
        Vec3 origin(readFloat(blobs, base), readFloat(blobs, base + 4), readFloat(blobs, base + 8));
        Vec3 edge1(readFloat(blobs, base + 12), readFloat(blobs, base + 16), readFloat(blobs, base + 20));
        Vec3 edge2(readFloat(blobs, base + 24), readFloat(blobs, base + 28), readFloat(blobs, base + 32));
        Vec2 uv0(readFloat(blobs, base + 36), readFloat(blobs, base + 40));
        Vec2 uv1(readFloat(blobs, base + 44), readFloat(blobs, base + 48));
        Vec2 uv2(readFloat(blobs, base + 52), readFloat(blobs, base + 56));
        tris.emplace_back(origin, origin + edge1, origin + edge2, uv0, uv1, uv2);
    }
    return tris;
}

// elements created before the scene commit; ids let the next stages resolve
// their dependencies (object -> material -> texture/medium)
struct StagedScene {
    std::vector<std::unique_ptr<Medium>> mediums;
    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Hittable>> objects;
    std::unordered_map<unsigned, NamedElement*> by_id;
    std::vector<std::filesystem::path> temp_assets;
    Medium* global_medium = nullptr;
    unsigned next_id = 0;
};

template <typename Expected>
Expected* resolve(const StagedScene& staged, const json& node, const char* key) {
    const auto id = refID(node, key);
    if(!id) return nullptr;

    const auto found = staged.by_id.find(*id);
    if(found == staged.by_id.end())
        throw std::runtime_error("scene load: dangling dependency id");

    Expected* ptr = dynamic_cast<Expected*>(found->second);
    if(!ptr)
        throw std::runtime_error("scene load: dependency has the wrong type");
    return ptr;
}

} // namespace

bool SceneLoader::save(
    const Scene& scene,
    const std::filesystem::path& path,
    const Camera* camera
) {
    json root;
    root["version"] = SCENE_FORMAT_VERSION;
    root["next_id"] = scene.next_id;
    if(camera) {
        root["camera"] = {
            {"focal_length", camera->getFocalLength()},
            {"aperture", camera->getAperture()},
            {"position", vecToJson(camera->getPosition())},
            {"direction", vecToJson(camera->getDirection())}
        };
    }
    const Medium* owned_global = nullptr;
    for(const auto& medium : scene.mediums) {
        if(medium.get() == scene.global_medium) {
            owned_global = scene.global_medium;
            break;
        }
    }
    root["global_medium"] = owned_global ? json(owned_global->getID()) : json(nullptr);
    root["mediums"] = json::array();
    root["textures"] = json::array();
    root["materials"] = json::array();
    root["objects"] = json::array();

    std::vector<std::uint8_t> blobs;

    // write in dependency order so the json tree can be replayed directly:
    // mediums -> textures -> materials -> objects
    for(const auto& medium : scene.mediums) {
        json node = {
            {"id", medium->getID()},
            {"name", medium->getName()},
            {"type", mediumType(medium.get())},
            {"ior", medium->getIOR()}
        };
        if(const auto* h = dynamic_cast<const Homogeneous*>(medium.get())) {
            node["sigma_a"] = vecToJson(h->getSigmaA());
            node["sigma_s"] = vecToJson(h->getSigmaS());
            node["density"] = h->getDensity();
        }
        root["mediums"].push_back(std::move(node));
    }

    for(const auto& texture : scene.textures) {
        json node = {
            {"id", texture->getID()},
            {"name", texture->getName()},
            {"type", textureType(texture.get())}
        };
        if(const auto* color = dynamic_cast<const ColorTexture*>(texture.get())) {
            node["color"] = vecToJson(color->getColor());
        } else if(const auto* image = dynamic_cast<const ImageTexture*>(texture.get())) {
            if(!appendAsset(blobs, image->getFilename(), node["asset"]))
                return false;
        } else {
            std::cerr << "scene save: unsupported texture type\n";
            return false;
        }
        root["textures"].push_back(std::move(node));
    }

    for(const auto& material : scene.materials) {
        json node = {
            {"id", material->getID()},
            {"name", material->getName()},
            {"type", materialType(material.get())},
            {"texture", material->getTexture() ? json(material->getTexture()->getID()) : json(nullptr)},
            {"medium", material->getMedium() ? json(material->getMedium()->getID()) : json(nullptr)}
        };
        if(const auto* metal = dynamic_cast<const Metal*>(material.get())) {
            node["roughness"] = metal->getRoughness();
        } else if(const auto* light = dynamic_cast<const Light*>(material.get())) {
            node["emission_strength"] = light->getEmissionStrength();
            node["color"] = vecToJson(light->getColor());
        }
        root["materials"].push_back(std::move(node));
    }

    for(const auto& object : scene.objects) {
        json transform = {
            {"translation", vecToJson(object->getTransform().getTranslation())},
            {"rotation", vecToJson(object->getTransform().getRotation())},
            {"scale", vecToJson(object->getTransform().getSize())}
        };
        json node = {
            {"id", object->getID()},
            {"name", object->getName()},
            {"type", objectType(object.get())},
            {"material", object->getMaterial() ? json(object->getMaterial()->getID()) : json(nullptr)},
            {"transform", std::move(transform)}
        };

        if(dynamic_cast<const Sphere*>(object.get())) {
            node["geometry"] = {{"kind", "sphere"}};
        } else if(const auto* mesh = dynamic_cast<const Mesh*>(object.get())) {
            if(!mesh->getFilename().empty()) {
                json asset;
                if(!appendAsset(blobs, mesh->getFilename(), asset))
                    return false;
                node["geometry"] = {{"kind", "obj"}, {"asset", std::move(asset)}};
            } else {
                // meshes built from raw triangles have no source file; store
                // their geometry as a binary triangle blob too
                const auto& tris = mesh->getTriangles();
                const std::size_t offset = blobs.size();
                for(const auto& tri : tris) {
                    appendFloat(blobs, tri.origin.x);
                    appendFloat(blobs, tri.origin.y);
                    appendFloat(blobs, tri.origin.z);
                    appendFloat(blobs, tri.edge1.x);
                    appendFloat(blobs, tri.edge1.y);
                    appendFloat(blobs, tri.edge1.z);
                    appendFloat(blobs, tri.edge2.x);
                    appendFloat(blobs, tri.edge2.y);
                    appendFloat(blobs, tri.edge2.z);
                    appendFloat(blobs, tri.uv0.x);
                    appendFloat(blobs, tri.uv0.y);
                    appendFloat(blobs, tri.uv1.x);
                    appendFloat(blobs, tri.uv1.y);
                    appendFloat(blobs, tri.uv2.x);
                    appendFloat(blobs, tri.uv2.y);
                }
                node["geometry"] = {
                    {"kind", "raw"},
                    {"count", tris.size()},
                    {"asset", {{"offset", offset}, {"size", blobs.size() - offset}}}
                };
            }
        } else {
            std::cerr << "scene save: unsupported object type\n";
            return false;
        }
        root["objects"].push_back(std::move(node));
    }

    const std::string header = root.dump(2);

    std::ofstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "scene save: cannot open " << path << '\n';
        return false;
    }

    file.write(SCENE_MAGIC, sizeof(SCENE_MAGIC));
    const std::uint32_t header_size = static_cast<std::uint32_t>(header.size());

    std::array<std::uint8_t, 4> size_bytes{};
    for(int i = 0; i < 4; i++)
        size_bytes[i] = std::uint8_t(header_size >> (i * 8));
    file.write(reinterpret_cast<const char*>(size_bytes.data()), size_bytes.size());
    file.write(header.data(), static_cast<std::streamsize>(header.size()));
    if(!blobs.empty()) {
        file.write(
            reinterpret_cast<const char*>(blobs.data()),
            static_cast<std::streamsize>(blobs.size())
        );
    }

    if(!file) {
        std::cerr << "scene save: failed writing " << path << '\n';
        return false;
    }
    return true;
}

bool SceneLoader::load(Scene& scene, const std::filesystem::path& path, Camera* camera) {
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "scene load: cannot open " << path << '\n';
        return false;
    }

    char magic[4];
    file.read(magic, sizeof(magic));
    if(!file or std::memcmp(magic, SCENE_MAGIC, sizeof(magic)) != 0) {
        std::cerr << "scene load: not a tdt scene file\n";
        return false;
    }

    std::array<std::uint8_t, 4> size_bytes{};
    file.read(reinterpret_cast<char*>(size_bytes.data()), size_bytes.size());
    if(!file) {
        std::cerr << "scene load: truncated header\n";
        return false;
    }
    std::uint32_t header_size = 0;
    for(int i = 0; i < 4; i++)
        header_size |= std::uint32_t(size_bytes[i]) << (i * 8);

    std::string header(header_size, '\0');
    file.read(header.data(), static_cast<std::streamsize>(header_size));
    if(!file) {
        std::cerr << "scene load: truncated json header\n";
        return false;
    }
    const std::vector<std::uint8_t> blobs{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    StagedScene staged;

    try {
        const json root = json::parse(header);
        if(root.at("version").get<std::uint32_t>() != SCENE_FORMAT_VERSION)
            throw std::runtime_error("scene load: unsupported version");

        for(const auto& node : root.at("mediums")) {
            const unsigned id = node.at("id").get<unsigned>();
            const std::string name = node.at("name").get<std::string>();
            const std::string type = node.at("type").get<std::string>();

            std::unique_ptr<Medium> medium;
            if(type == "Medium") {
                medium = std::make_unique<Medium>(id, name, node.value("ior", 1.0f));
            } else if(type == "Homogeneous") {
                medium = std::make_unique<Homogeneous>(
                    id,
                    name,
                    node.value("ior", 1.0f),
                    node.contains("sigma_a") ? vecFromJson(node.at("sigma_a")) : Color(0.0f),
                    node.contains("sigma_s") ? vecFromJson(node.at("sigma_s")) : Color(0.0f),
                    node.value("density", 1.0f)
                );
            } else {
                throw std::runtime_error("scene load: unknown medium type");
            }

            if(!staged.by_id.emplace(id, medium.get()).second)
                throw std::runtime_error("scene load: duplicate element id");
            staged.mediums.push_back(std::move(medium));
        }

        for(const auto& node : root.at("textures")) {
            const unsigned id = node.at("id").get<unsigned>();
            const std::string name = node.at("name").get<std::string>();
            const std::string type = node.at("type").get<std::string>();

            std::unique_ptr<Texture> texture;
            if(type == "ColorTexture") {
                texture = std::make_unique<ColorTexture>(
                    id,
                    name,
                    node.contains("color") ? vecFromJson(node.at("color")) : Color(1.0f)
                );
            } else if(type == "ImageTexture") {
                const auto& asset = node.at("asset");
                const std::string filename = asset.value("filename", std::string());
                std::filesystem::path temp_path;
                if(!extractAsset(asset, blobs, imageExtension(filename), temp_path, staged.temp_assets))
                    throw std::runtime_error("scene load: broken image asset");
                texture = std::make_unique<ImageTexture>(id, name, temp_path);
            } else {
                throw std::runtime_error("scene load: unknown texture type");
            }

            if(!staged.by_id.emplace(id, texture.get()).second)
                throw std::runtime_error("scene load: duplicate element id");
            staged.textures.push_back(std::move(texture));
        }

        for(const auto& node : root.at("materials")) {
            const unsigned id = node.at("id").get<unsigned>();
            const std::string name = node.at("name").get<std::string>();
            const std::string type = node.at("type").get<std::string>();

            Texture* texture = resolve<Texture>(staged, node, "texture");
            Medium* medium = resolve<Medium>(staged, node, "medium");

            std::unique_ptr<Material> material;
            if(type == "Matte") {
                material = std::make_unique<Matte>(id, name, texture);
            } else if(type == "Metal") {
                material = std::make_unique<Metal>(id, name, texture, node.value("roughness", 0.0f));
            } else if(type == "Light") {
                material = std::make_unique<Light>(
                    id,
                    name,
                    texture,
                    node.value("emission_strength", 1.0f),
                    node.contains("color") ? vecFromJson(node.at("color")) : Color(1.0f)
                );
            } else if(type == "Dielectric") {
                material = std::make_unique<Dielectric>(id, name, texture, medium);
            } else if(type == "FineParticle") {
                Homogeneous* homogeneous = dynamic_cast<Homogeneous*>(medium);
                if(!homogeneous and medium)
                    throw std::runtime_error("scene load: FineParticle needs a Homogeneous medium");
                material = std::make_unique<FineParticle>(id, name, texture, homogeneous);
            } else {
                throw std::runtime_error("scene load: unknown material type");
            }

            if(!staged.by_id.emplace(id, material.get()).second)
                throw std::runtime_error("scene load: duplicate element id");
            staged.materials.push_back(std::move(material));
        }

        for(const auto& node : root.at("objects")) {
            const unsigned id = node.at("id").get<unsigned>();
            const std::string name = node.at("name").get<std::string>();
            const std::string type = node.at("type").get<std::string>();

            Material* material = resolve<Material>(staged, node, "material");

            std::unique_ptr<Hittable> object;
            if(type == "Sphere") {
                object = std::make_unique<Sphere>(id, name, material);
            } else if(type == "Mesh") {
                const auto& geometry = node.at("geometry");
                const std::string kind = geometry.at("kind").get<std::string>();
                if(kind == "obj") {
                    const auto& asset = geometry.at("asset");
                    const std::string filename = asset.value("filename", std::string());
                    std::string ext = std::filesystem::path(filename).extension().string();
                    if(ext.empty()) ext = ".obj";
                    std::filesystem::path temp_path;
                    if(!extractAsset(asset, blobs, ext, temp_path, staged.temp_assets))
                        throw std::runtime_error("scene load: broken mesh asset");
                    object = std::make_unique<Mesh>(id, name, material, temp_path.string());
                } else if(kind == "raw") {
                    object = std::make_unique<Mesh>(
                        id, name, material,
                        readRawTriangles(geometry.at("asset"), blobs)
                    );
                } else {
                    throw std::runtime_error("scene load: unknown mesh geometry kind");
                }
            } else {
                throw std::runtime_error("scene load: unknown object type");
            }

            const auto& transform = node.at("transform");
            Transform t;
            t.scale(transform.contains("scale") ? vecFromJson(transform.at("scale")) : Vec3(1.0f));
            t.rotate(transform.contains("rotation") ? vecFromJson(transform.at("rotation")) : Vec3(0.0f));
            t.move(transform.contains("translation") ? vecFromJson(transform.at("translation")) : Vec3(0.0f));
            object->setTransform(t);

            if(!staged.by_id.emplace(id, object.get()).second)
                throw std::runtime_error("scene load: duplicate element id");
            staged.objects.push_back(std::move(object));
        }

        unsigned max_id = 0;
        for(const auto& [id, element] : staged.by_id) {
            (void)element;
            max_id = std::max(max_id, id);
        }
        staged.next_id = std::max(root.value("next_id", 0u), max_id + 1u);
        if(const auto global_id = refID(root, "global_medium")) {
            const auto found = staged.by_id.find(*global_id);
            if(found == staged.by_id.end())
                throw std::runtime_error("scene load: dangling global medium id");
            staged.global_medium = dynamic_cast<Medium*>(found->second);
            if(!staged.global_medium)
                throw std::runtime_error("scene load: global medium has the wrong type");
        }

        // restore the view only after the whole file validated, so a broken
        // scene does not move a perfectly good camera
        if(camera and root.contains("camera")) {
            const auto& cam = root.at("camera");
            const Vec3 position = cam.contains("position")
                ? vecFromJson(cam.at("position"))
                : camera->getPosition();
            camera->setFocalLength(cam.value("focal_length", camera->getFocalLength()));
            camera->setAperture(cam.value("aperture", camera->getAperture()));
            // setPosition does not rebuild the orientation, so apply the
            // direction afterwards via setDirection (which does)
            camera->setPosition(position);
            if(cam.contains("direction"))
                camera->setDirection(vecFromJson(cam.at("direction")));
        }
    } catch(const std::exception& error) {
        std::cerr << "scene load: " << error.what() << '\n';
        removeTempAssets(staged.temp_assets);
        return false;
    }

    // commit only after the whole file was parsed successfully
    scene.clear();
    scene.objects = std::move(staged.objects);
    scene.materials = std::move(staged.materials);
    scene.textures = std::move(staged.textures);
    scene.mediums = std::move(staged.mediums);
    scene.temp_assets = std::move(staged.temp_assets);
    scene.next_id = staged.next_id;
    scene.global_medium = staged.global_medium;

    for(const auto& object : scene.objects) {
        if(object->getMaterial() and object->getMaterial()->isEmissive())
            scene.lights.push_back(object.get());
    }
    scene.buildBVH();

    return true;
}
