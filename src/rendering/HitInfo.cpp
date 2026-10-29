#include "HitInfo.h"

bool HitInfo::didHit() const {
    return distance > 0.0;
}
