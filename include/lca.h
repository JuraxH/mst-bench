// This code is taken from and only slightly modified to fit our graph:
// https://cp-algorithms.com/graph/lca_farachcoltonbender.html

#pragma once

#include "graph.h"
#include <string>

class LCA {
    public:
    GraphType& graph;
    size_t root;
    size_t edges;
    size_t euler_size;
    size_t block_size;
    size_t block_cnt;
    // the order of vertexes in euler tour and their heights
    std::vector<Vertex> euler_tour;
    std::vector<size_t> height;
    // the index in euler_tour of first occurence of vertex
    std::vector<size_t> first_visit;
    std::vector<std::vector<size_t>> sparse_table;
    std::vector<size_t> block_mask;
    // TODO allocate as continuous memory and remove redundant fields
    // the sizes of the last vector decrease with the index of the second field
    // current impl still takes up that space
    // WARN the meaning of the last index would have to change to length instead of end of interval
    std::vector<std::vector<std::vector<size_t>>> blocks;
    


    LCA(GraphType& graph, Vertex root);

    void build_euler_tour();
    void build_sparse_table();
    void build_rmq();

    size_t lca(size_t u, size_t v);
    size_t lca_in_block(size_t block_index, size_t in_block_index, size_t interval_length);

    size_t min_by_height(size_t a, size_t b) const {
        return height[euler_tour[a]] < height[euler_tour[b]] ? a : b;
    }
    size_t depth(Vertex u) {
        return height[u];
    }

    Vertex parrent(Vertex u);


    // for testing
    size_t max_depth() {
        return *std::max_element(height.begin(), height.end());
    }

    // this is used for tests, in real problem the leafs are already know
    std::vector<Vertex> leafs();

    // debuging utils
    std::string dump();
    std::string dump_blocks();
    std::string dump_block(size_t block_index);
    std::string dump_sparse_table();
};

