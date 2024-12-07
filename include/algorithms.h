#pragma once

#include "graph.h"
#include "utils.h"

#include <algorithm>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>


class Kruskal : public MSTAlgorithm {
    public:
    Kruskal(Graph& g) : MSTAlgorithm(g, "Kruskal") { }

    MST compute_mst() override;
};

class RandomKKT : public MSTAlgorithm {
    RandomKKT(Graph &g) : MSTAlgorithm(g, "randomKKT") { }

    MST compute_mst() override;
};

class Boruvka : public MSTAlgorithm {
    public:
    Boruvka(Graph &g) : MSTAlgorithm(g, "boruvka") { }

    MST compute_mst() override;
};
