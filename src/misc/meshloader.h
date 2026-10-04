#pragma once

#include <string>
#include <vector>
#include "RawTriangle.h"

void load_mesh_from(
    std::string filename,
    std::vector<RawTriangle>& tris
);
