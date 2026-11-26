#include "ImageTexture.h"
#include "rendering/HitInfo.h"
#include "rendering/Hittable.h"
#include <algorithm>

ImageTexture::ImageTexture(unsigned id, const std::string name, const std::filesystem::path& filename):
    Texture(id, name),
    image(filename),
    filename(filename) {}

const std::filesystem::path& ImageTexture::getFilename() const {
    return filename;
}

Color ImageTexture::get(const HitInfo& rec) const {
    const sf::Vector2u size = image.getSize();
    const Vec2& uv = rec.tri_uv.x >= 0 && rec.tri_uv.y >= 0 ? rec.tri_uv : rec.hittable->getUV(rec);
    unsigned x = std::clamp((unsigned)(uv.x * (size.x - 1)), 0u, size.x - 1u);
    unsigned y = std::clamp((unsigned)(uv.y * (size.y - 1)), 0u, size.y - 1u);
    return Color::fromRGBA(image.getPixel({x, y}).toInteger());
    // NOTE:
    // conversion from sf::Color to uint32 is constexpr
    // i still convert it to integer then convert it back to vec3 again because
    // this method is universal to all image loader
}
