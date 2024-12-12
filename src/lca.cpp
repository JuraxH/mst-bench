// This code is taken from and only slightly modified to fit our graph:
// https://cp-algorithms.com/graph/lca_farachcoltonbender.html

#include "lca.h"
#include "utils.h"

LCA::LCA(GraphType& graph, Vertex root)
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

Vertex LCA::parrent(Vertex u) {
    if (u == root) {
        return boost::graph_traits<GraphType>::null_vertex();
    } else {
        return euler_tour[first_visit[u] - 1];
    }
}

size_t LCA::lca(size_t u, size_t v) {
    auto l = first_visit[u];
    auto r = first_visit[v];
    if (l > r) {
        using std::swap;
        swap(l, r);
    }
    size_t bl = l / block_size; 
    size_t br = r / block_size;
    if (bl == br) {
        return euler_tour[lca_in_block(bl, l % block_size, r % block_size)];
    }
    size_t l_min = lca_in_block(bl, l % block_size, block_size - 1);
    size_t r_min = lca_in_block(br, 0, r % block_size);
    auto min = min_by_height(l_min, r_min);
    if (bl + 1 < br) { // interval longer than 2 blocks
        size_t half_log_len = log2(br - bl - 1);
        size_t first_half = bl + 1;
        size_t second_half = br - (1 << half_log_len);
        l_min = sparse_table[first_half][half_log_len];
        r_min = sparse_table[second_half][half_log_len];
        min = min_by_height(min, min_by_height(l_min, r_min));
    }
    return euler_tour[min];
}

size_t LCA::lca_in_block(size_t block_index, size_t in_block_index, size_t interval_length) {
    auto block_offset = block_index * block_size;
    return blocks[block_mask[block_index]][in_block_index][interval_length] + block_offset;
}

void LCA::build_euler_tour() {
    euler_tour.reserve(euler_size);
    using AdjIterator = boost::graph_traits<GraphType>::adjacency_iterator;
    // cur, from, height, begin, end
    std::vector<std::tuple<Vertex, Vertex, size_t, AdjIterator, AdjIterator>> stack{};
    auto [begin, end] = boost::adjacent_vertices(root, graph);
    stack.push_back({root, std::numeric_limits<Vertex>::max(), 0, begin, end});
    while (!stack.empty()) {
        auto [cur, prev, h, begin, end] = stack.back();
        stack.pop_back();
        if (boost::adjacent_vertices(cur, graph).first == begin) {
            first_visit[cur] = euler_tour.size();
            height[cur] = h;
        }
        euler_tour.push_back(cur);
        if (begin == end) {
            continue;
        }
        auto next = *begin;
        ++begin;
        if (next == prev) {
            if (begin == end) {
                continue;
            }
            next = *begin;
            begin++;
        }
        stack.push_back({cur, prev, h, begin, end});
        auto [next_begin, next_end] = boost::adjacent_vertices(next, graph);
        stack.push_back({next, cur, h + 1, next_begin, next_end});
    }
}

void LCA::build_sparse_table() {
    for (size_t i = 0, block_index = 0, cur_block = 0; i < euler_size; i++, block_index++) {
        if (block_index == block_size) {
            block_index = 0;
            cur_block++;
        }
        // find the minimum of each block
        if (block_index == 0 || (min_by_height(i, sparse_table[cur_block][0]) == i)) {
            sparse_table[cur_block][0] = i;
        }
        // building mask for the current block
        // testing if height of current possition is prev + 1 if true add +
        if (block_index > 0 && (min_by_height(i - 1, i) == i - 1)) {
            size_t bit_index = block_index - 1;
            block_mask[cur_block] |= 1 << bit_index; // marking as +
        }
    }
    // compute mins for squares of blocks
    for (size_t log_length = 1; log_length <= log2(block_cnt); log_length++) {
        size_t prev_length = log_length - 1;
        for (size_t i = 0; i < block_cnt; i++) {
            size_t next_block = i + (1 << prev_length);
            size_t cur_min = sparse_table[i][prev_length];
            if (next_block >= block_cnt) {
                sparse_table[i][log_length] = cur_min;
            } else {
                sparse_table[i][log_length] = min_by_height(cur_min, sparse_table[next_block][prev_length]);
            }
        }
    }
}

void LCA::build_rmq() {
    auto computed = std::vector<bool>(blocks.size(), false);
    for (size_t cur_block = 0; cur_block < block_cnt; cur_block++) {
        auto mask = block_mask[cur_block];
        if (computed[mask]) {
            continue;
        }
        computed[mask] = true;
        auto block_offset = cur_block * block_size;
        // compute the min of each sub interval 
        assert(mask < (1ul<<(block_size - 1)));
        for (size_t l = 0; l < block_size; l++) {
            assert(l < block_size);
            blocks[mask][l][l] = l; // the min of [l, l]
                                    // compute min for all interval sizes in block
            for (size_t r = l + 1; r < block_size; r++) {
                assert(r < block_size);
                blocks[mask][l][r] = blocks[mask][l][r - 1];
                auto cur_min_pos = block_offset + blocks[mask][l][r];
                auto orig_pos = r + block_offset;
                if (orig_pos < euler_size) {
                    blocks[mask][l][r] = min_by_height(cur_min_pos, orig_pos) - block_offset;
                }
            }
        }
    }
}

std::vector<Vertex> LCA::leafs() {
    auto res = std::vector<Vertex>{};
    auto leaf_depth = max_depth();
    for (auto v : boost::make_iterator_range(boost::vertices(graph))) {
        if (depth(v) == leaf_depth) {
            res.emplace_back(v);
        }
    }
    return res;
}

std::string LCA::dump() {
    auto res = std::string{};
    dump_as_dot(std::cout, graph);
    res += "graph with root: " + std::to_string(root) + '\n';
    res += "edges: " + std::to_string(edges) + '\n';
    res += "euler_size: " + std::to_string(euler_size) + '\n';
    res += "block_size: " + std::to_string(block_size) + '\n';
    res += "block_cnt: " + std::to_string(block_cnt) + '\n';
    res += "edges: " + std::to_string(edges) + '\n';
    res += "nodes:       ";
    res += dump_vector(euler_tour);
    res += "height:      ";
    res += dump_vector(height);
    res += "first_visit: ";
    res += dump_vector(first_visit);
    //res += dump_blocks();
    res += dump_sparse_table();
    return res;
}


std::string LCA::dump_blocks() {
    auto res = std::string{};
    for (size_t i = 0; i < block_cnt; i++) {
        res += dump_block(i);
    }
    return res;
}

std::string LCA::dump_block(size_t block_index) {
    auto block_offset = block_index * block_size;
    auto mask = block_mask[block_index];
    auto ref_h = height[euler_tour[block_offset]];
    auto res = std::string{};
    res += "block: " + std::to_string(block_index);
    res += " start_h: " + std::to_string(ref_h) + '\n';
    res += "mask: " + dump_bits(mask) + '\n';
    auto signs = std::string{};
    auto vals = std::string{};
    size_t cur_val = ref_h;
    for (size_t i = 1; i < block_size; i++) {
        size_t cur_bit = 1 << (i - 1);
        if (mask & cur_bit) {
            signs += '+';
            cur_val += 1;
        } else {
            signs += '-';
            cur_val -= 1;
        }
        vals += std::to_string(cur_val);
    }
    res += signs + '\n' + vals + '\n';
    // min of intervals
    for (size_t l = 0; l < block_size; l++) {
        res += "min [" + std::to_string(l) + ", ...]\n";
        for (size_t r = l; r < block_size; r++) {
            res += std::to_string(blocks[mask][l][r]) + " ";
        }
        res += '\n';
    }
    return res;
}

std::string LCA::dump_sparse_table() {
    auto res = std::string{};
    for (size_t log_length = 0; log_length <= log2(block_cnt); log_length++) {
        res += "min of " + std::to_string(log_length) + " blocks: ";
        for (size_t i = 0; i < block_cnt; i++) {
            auto cur_min = sparse_table[i][log_length];
            res += std::to_string(cur_min) + "(" + std::to_string(euler_tour[cur_min]) + "), ";
        }
        res += '\n';
    }
    return res;
}

