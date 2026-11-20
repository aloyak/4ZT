#include "4ZT/bvh/bvh.h"

// Build a BVH tree from the objects in the world,
// returns a BVH object that contains the nodes and the object indices
// Recursively work the way down the tree, partitioning the objects into two groups at each node
FourZT::BVH FourZT::BVH::Build(const World& world) {
    std::vector<Prim> prims;
    int i = 0;
    for (const auto& obj : world.Get()) {
        AABB box = AABB::BoundingBox(*obj);
        prims.push_back({ box, box.Centroid(), i++ });
    }

    BVH bvh;
    if (prims.empty()) return bvh;

    bvh.nodes.reserve(2 * prims.size());
    bvh.nodes.emplace_back();   // root
    BuildRange(bvh, prims, 0, 0, static_cast<int>(prims.size()));

    bvh.objectIndices.resize(prims.size());
    for (size_t k = 0; k < prims.size(); k++) 
        bvh.objectIndices[k] = prims[k].index;

    return bvh;
}

void FourZT::BVH::BuildRange(FourZT::BVH& bvh, std::vector<Prim>& prims, int nodeIdx, int begin, int end) {
    // bounds of everything in this range, plus bounds of the centroids
    FourZT::AABB bounds;
    FourZT::AABB centroidBounds;
    for (int i = begin; i < end; i++) {
        bounds = FourZT::AABB(bounds, prims[i].box);
        centroidBounds = FourZT::AABB(centroidBounds, FourZT::AABB(prims[i].centroid, prims[i].centroid));
    }
    bvh.nodes[nodeIdx].bounds = bounds;

    int count = end - begin;
    if (count <= 2) { // 2 is how many objects per leaf, can be tuned
        bvh.nodes[nodeIdx].leftOrFirst = begin;
        bvh.nodes[nodeIdx].objectCount = count;
        return;
    }

    int axis = centroidBounds.LongestAxis();
    int mid = begin + count / 2;
    std::nth_element(prims.begin() + begin, prims.begin() + mid, prims.begin() + end,
        [axis](const Prim& a, const Prim& b) { return a.centroid[axis] < b.centroid[axis]; });

    int left = static_cast<int>(bvh.nodes.size());
    bvh.nodes.emplace_back();
    bvh.nodes.emplace_back();
    bvh.nodes[nodeIdx].leftOrFirst = left;
    bvh.nodes[nodeIdx].objectCount = 0;

    BuildRange(bvh, prims, left,     begin, mid);
    BuildRange(bvh, prims, left + 1, mid,   end);
}
/// TODO: review this!