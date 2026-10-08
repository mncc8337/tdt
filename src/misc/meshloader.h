#pragma once

#include <string>
#include <vector>
#include "RawTriangle.h"

void loadMeshFrom(
    std::string filename,
    std::vector<RawTriangle>& tris
);
