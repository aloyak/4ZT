#pragma once

#include <vector>

#include "4ZT/bvh/aabb.h"
#include "4ZT/world.h"

namespace FourZT {

// primitive = object wrapper that has a bounding box and a centroid
struct Prim {
    FourZT::AABB box;
    Point3 centroid;
    int index; // index of the object in the vector that world holds
};

class BVHNode {
public:
    AABB bounds;

    // leftOrFirst has double duty
    // node is interior: holds the index of the left child node, right child is index + 1
    // node is leaf: holds the index of the first object in the objectIndices vector, and objectCount is the number of objects in this leaf
    int leftOrFirst;
    int objectCount;
};

// Data structure only
class BVH {
public:
    std::vector<BVHNode> nodes;
    std::vector<int> objectIndices;
    
    static BVH Build(const World& world);

private:
    static void BuildRange(BVH& bvh, std::vector<Prim>& prims, int nodeIndex, int start, int end);
};

} // 4zt