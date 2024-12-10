#include "graph.h"
#include "utils.h"
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/subgraph.hpp>
#include <iostream>
#include <limits>
#include <string>

// This code is taken from and only slightly modified to fit our graph:
// https://cp-algorithms.com/graph/lca_farachcoltonbender.html

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
    


    LCA(GraphType& graph, Vertex root)
        : graph(graph)
        , root(root)
        , edges(boost::num_edges(graph))
        , euler_size(2 * edges)
        , block_size(std::max(1ul, log2(euler_size) / 2))
        , block_cnt((euler_size + block_size - 1) / block_size)
        , euler_tour()
        , height(boost::num_vertices(graph), 0)
        , first_visit(boost::num_vertices(graph), 0)
        , sparse_table(block_cnt, std::vector<size_t>(log2(block_cnt) + 1))
        , block_mask(block_cnt, 0)
        , blocks(1 << (block_size - 1), std::vector(block_size, std::vector<size_t>(block_size)))
    {
        build_euler_tour();
        build_sparse_table();
        build_rmq();
    }

    void build_euler_tour();
    void build_sparse_table();
    void build_rmq();

    size_t lca(size_t u, size_t v);
    size_t lca_in_block(size_t block_index, size_t in_block_index, size_t interval_length);
    size_t min_by_height(size_t a, size_t b) const {
        return height[euler_tour[a]] < height[euler_tour[b]] ? a : b;
    }

    Vertex parrent(Vertex u) {
        if (u == root) {
            return boost::graph_traits<GraphType>::null_vertex();
        } else {
            return euler_tour[first_visit[u] - 1];
        }
    }

    size_t depth(Vertex u) {
        return height[u];
    }

    // debuging utils
    std::string dump();
    std::string dump_blocks();
    std::string dump_block(size_t block_index);
    std::string dump_sparse_table();
};

