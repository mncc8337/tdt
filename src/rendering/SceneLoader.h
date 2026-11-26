#pragma once

#include <filesystem>

class Camera;
class Scene;

// Saves and restores a Scene. The file layout is:
//   "TDSC" magic | uint32 json size (LE) | json scene tree | binary blobs
// The json tree stores each NamedElement once (id, name, type, props) and
// wires the dependency tree object -> material -> texture/medium via ids.
// Camera settings live in the same header. Mesh and image files are stored
// verbatim as blobs after the header; load() extracts them to the system
// temp directory and Scene owns those paths until it is cleared or destroyed.
class SceneLoader {
public:
    // camera is optional: files saved without one leave the camera untouched
    static bool save(const Scene& scene, const std::filesystem::path& path, const Camera* camera = nullptr);
    static bool load(Scene& scene, const std::filesystem::path& path, Camera* camera = nullptr);
};
