#pragma once

#include "graph.h"
#include "lca.h"
#include "utils.h"
#include <boost/graph/subgraph.hpp>
#include <limits>


// both vertexes must be leafs
struct TreepathQuery {
    Vertex u;
    Vertex v;
};

// the query must be about leaf and its proper ancestor
struct BottomUpQuery {
    Vertex leaf;
    Vertex ancestor;
};

// the Vertexes with the heaviest edge
struct Result {
    Vertex from;
    Vertex to;
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
                                    //
    std::vector<size_t> query_sets;
    std::vector<size_t> answer_sets;
    std::vector<std::vector<Vertex>> rows;

    TreePathMaxima(std::vector<BottomUpQuery> queries, LCA& lca)
        : tree(lca.graph)
        , root(lca.root)
        , depth()
        , lca(lca)
        , queries(queries)
        , first_query(boost::num_vertices(tree), None)
        , next_query(queries.size(), None)
        , query_sets(boost::num_vertices(tree), 0)
        , answer_sets(boost::num_vertices(tree), 0)
        , rows()
    {
        depth = lca.depth(queries[0].leaf);
        rows.resize(depth + 1, {});
        assign_queries_to_leafs();
        propagate_query_sets_up();
    }

    // assigns each node query
    void assign_queries_to_leafs() {
        for (size_t i = 0; i < queries.size(); i++) {
            auto query = queries[i];
            if (first_query[query.leaf] == None) {
                rows[depth].push_back(query.leaf);
            }
            next_query[i] = first_query[query.leaf]; // prev query stored in next query
            first_query[query.leaf] = i; // the current query assigned to node

            // set the predecessor bit in the query set
            auto bit_index = 1ul<<(lca.depth(query.ancestor));
            query_sets[query.leaf] |= bit_index;
        }
    }

    void propagate_query_sets_up() {
        auto found = std::vector<size_t>(boost::num_vertices(tree), None);
        for (size_t cur_d = depth; cur_d > 0; cur_d--) {
            size_t parent_d = cur_d - 1;
            size_t parent_mask = ~(1ul<<parent_d);
            for (auto u : rows[cur_d]) {
                auto parent = lca.parrent(u);
                query_sets[parent] |= query_sets[u] & parent_mask;

                // save the parrent for the next depth iteration
                if (found[parent] != cur_d) {
                    rows[parent_d].push_back(parent);
                    found[parent] = cur_d;
                }
            }
        }
    }

    // just for debugin of the efficient representation
    std::vector<std::vector<size_t>> query_per_leaf() {
        auto leaf_to_queries = std::vector<std::vector<size_t>>{};
        auto leafs = lca.leafs();
        for (auto leaf : leafs) {
            auto qs = std::vector<size_t>{};
            auto cur = first_query[leaf];
            while (cur != None) {
                qs.push_back(cur);
                cur = next_query[cur];
            }
            leaf_to_queries.push_back(qs);
        }
        return leaf_to_queries;
    }

};
