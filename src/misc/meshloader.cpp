#include "meshloader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

void parse_obj_face_token(const std::string& token, int& v_idx, int& vt_idx) {
    size_t slash1 = token.find('/');
    v_idx = std::stoi(token.substr(0, slash1));

    if (slash1 != std::string::npos) {
        size_t slash2 = token.find('/', slash1 + 1);
        std::string vt_str = token.substr(slash1 + 1, slash2 - slash1 - 1);
        if (!vt_str.empty()) {
            vt_idx = std::stoi(vt_str);
        } else {
            vt_idx = 0;
        }
    } else {
        vt_idx = 0;
    }
}

void loadMeshFrom(std::string filename, std::vector<RawTriangle>& tris) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "failed to load file " << filename << std::endl;
        return;
    }

    std::vector<Vec3> verts;
    std::vector<Vec2> texs;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream s(line);
        std::string type;
        s >> type;

        if (type == "v") {
            Vec3 v;
            s >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } 
        else if (type == "vt") {
            Vec2 vt;
            s >> vt.x >> vt.y;
            vt.x = 1.0f - vt.x;
            vt.y = 1.0f - vt.y;
            texs.push_back(vt);
        }
        else if (type == "f") {
            std::string t1, t2, t3;
            s >> t1 >> t2 >> t3;

            int v[3], vt[3];
            parse_obj_face_token(t1, v[0], vt[0]);
            parse_obj_face_token(t2, v[1], vt[1]);
            parse_obj_face_token(t3, v[2], vt[2]);

            if (vt[0] > 0 && !texs.empty()) {
                tris.emplace_back(
                    verts[v[0] - 1], verts[v[1] - 1], verts[v[2] - 1],
                    texs[vt[0] - 1], texs[vt[1] - 1], texs[vt[2] - 1]
                );
            } else {
                tris.emplace_back(verts[v[0] - 1], verts[v[1] - 1], verts[v[2] - 1]);
            }
        }
    }
}
