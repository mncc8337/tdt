#include "BVHNode.h"

bool BVHNode::isLeaf() const {
    return !left and !right;
}
