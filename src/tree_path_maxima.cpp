// This code is taken from: https://www.cs.tau.ac.il/~zwick/grad-algo-0910/mst-verify.pdf
// and: https://doi.org/10.1007/978-3-642-11409-0_16
// modified to fit our graphs and work in c++

#include "tree_path_maxima.h"
#include <bit>

TreePathMaxima::TreePathMaxima(std::vector<BottomUpQuery> queries, LCA& lca)
    : tree(lca.graph)
    , root(lca.root)
    , depth()
    , lca(lca)
    , queries(queries)
    , first_query(boost::num_vertices(tree), None)
    , next_query(queries.size(), None)
    , query_sets(boost::num_vertices(tree), 0ul)
    , answer_sets(boost::num_vertices(tree), 0ul)
    , answers(queries.size())
    , rows()
    , median_table()
    , T()
    , visit_stack()
      , weight_to_parent(boost::num_vertices(tree), -std::numeric_limits<double>::infinity())
{
    compute_parent_weights();
    depth = lca.depth(queries[0].leaf);
    rows.resize(depth + 1, {});
    T.resize((1ul << depth) + 1, 0ul);
    median_table.resize(1ul<<(depth + 1), 0ul);
    visit_stack.resize(depth + 1);
    compute_median_table(depth);
    assign_queries_to_leafs();
    propagate_query_sets_up();
    visit(root, 0ul);
}

void TreePathMaxima::compute_parent_weights() {
    auto weight_map = get(boost::edge_weight, tree);
    for (auto edge : boost::make_iterator_range(boost::edges(tree))) {
        auto u = boost::source(edge, tree);
        auto v = boost::target(edge, tree);
        auto u_parent = lca.parrent(u);
        auto v_parent = lca.parrent(u);
        auto weight = weight_map[edge];
        if (v == u_parent) {
            weight_to_parent[u] = weight;
        } else if (u == v_parent) {
            weight_to_parent[v] = weight;
        }
    }
}

size_t TreePathMaxima::binary_search(double w, size_t S) {
    // Returns max({j in S | weight[P[j]]>w} union {0})
    if (S == 0) return 0;
    size_t j = median_table[S];
    while (S != 1ul<<j) { // while |S|>1
        S &= (weight(visit_stack[j]) > w) ? ~((1ul << j) - 1ul) : (1ul << j) - 1ul;
        j = median_table[S];
    }
    return (weight(visit_stack[j]) > w) ? j : 0;
}


void TreePathMaxima::visit(Vertex v, size_t S) { // S = S of parent
    visit_stack[lca.depth(v)]=v; // push current node on stack
    size_t k = binary_search(weight(v), down(query_sets[v], S));

    S=down(query_sets[v], (S & ((1 << (k + 1)) - 1)) | (1 << lca.depth(v)));
    answer_sets[v] = S;
    for (size_t i = first_query[v]; i != None; i = next_query[i]) {
        auto tmp = S & (None<<(lca.depth(queries[i].ancestor) + 1));
        auto lsb_pos = std::countr_zero(tmp);
        answers[i] = visit_stack[lsb_pos];
    }
    for (auto child : boost::make_iterator_range(boost::adjacent_vertices(v, tree))) {
        if (child != lca.parrent(v)) {
            visit(child, S);
        }
    }
}

void TreePathMaxima::assign_queries_to_leafs() {
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

size_t TreePathMaxima::subsets(size_t n,size_t k,size_t p) {
    // Stores the subsets of size k of {0,...,n-1} in T,
    // starting in position p, and returns p plus their number.
    if (n < k || n > (1ul << 62)) {
        return p;
    }
    if (k == 0ul) {
        T[p] = 0ul;
        return p + 1;
    }
    size_t q = subsets(n - 1, k - 1, p);
    for (size_t i = p; i < q; i++) {
        T[i] |= 1ul<<(n-1);
    }
    return subsets(n - 1, k, q);
}

void TreePathMaxima::compute_median_table(size_t h) {
    for (size_t s = 0; s <= h; s++) {
        for (size_t k = 0; k <= s; k++) {
            size_t p = subsets(h - s, k, 0);
            size_t q = subsets(s, k, p);
            q = subsets(s, k + 1, q);
            for (size_t i = 0; i < p; i++) {
                size_t b = (1 << (s + 1)) * T[i] + (1<<s); // fixed high bits
                for (size_t j = p; j < q; j++) {
                    median_table[b + T[j]] = s; // variable low bits
                }
            }
        }
    }
}

void TreePathMaxima::propagate_query_sets_up() {
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

std::vector<std::vector<size_t>> TreePathMaxima::query_per_leaf() {
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
