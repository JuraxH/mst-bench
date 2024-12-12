// This code is taken from: https://www.cs.tau.ac.il/~zwick/grad-algo-0910/mst-verify.pdf
// and: https://doi.org/10.1007/978-3-642-11409-0_16
// modified to fit our graphs and work in c++

#pragma once

#include "graph.h"
#include "lca.h"

// the query must be about leaf and its proper ancestor
struct BottomUpQuery {
    Vertex leaf;
    Vertex ancestor;
};

constexpr size_t None = std::numeric_limits<size_t>::max();

struct TreePathMaxima {
    GraphType& tree; // must be a fully branching tree
    Vertex root;
    size_t depth;
    LCA& lca;
    std::vector<BottomUpQuery> queries;
    // leafs to queries
    std::vector<size_t> first_query; // first query in leaf
    std::vector<size_t> next_query; // links to the next query for the same leaf

    std::vector<size_t> query_sets;
    std::vector<size_t> answer_sets;
    // here will be the answer to each query, encoded as vertex whose edge
    // to parent is the maximal
    std::vector<Vertex> answers;

    std::vector<std::vector<Vertex>> rows; // rows of vetexes in each depth
    std::vector<size_t> median_table; // precomputed medians
    std::vector<size_t> T;
    std::vector<size_t> visit_stack;
    std::vector<double> weight_to_parent;

    TreePathMaxima(std::vector<BottomUpQuery> queries, LCA& lca);

    double weight(Vertex u) {
        return weight_to_parent[u];
    }

    size_t down(size_t a, size_t b) {
        return b & (~(a | b) ^ (a + (a | ~b)));
    }

    void compute_parent_weights();
    size_t binary_search(double w, size_t S);
    void visit(Vertex v, size_t S);
    void assign_queries_to_leafs();
    size_t subsets(size_t n,size_t k,size_t p);
    void compute_median_table(size_t h);
    void propagate_query_sets_up();

    // just for debugin of the efficient representation
    std::vector<std::vector<size_t>> query_per_leaf();

};
