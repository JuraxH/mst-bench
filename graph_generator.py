import random
from itertools import combinations
import sys

def generate_graph(size, density, weight_range=(1, 10), file=sys.stdin):
    clique_m = size * (size - 1) // 2    
    desired_m = int(density * clique_m)
    available_edges = list(combinations(range(size), 2))
    #make this a numpy matrix
    #print(available_edges)

    #build spanning tree
    spanning_tree = list()
    visited = list()
    not_visited = list(range(0, size))
    curr_v = not_visited.pop(random.randint(0, size-1))
    while len(not_visited) > 0:
        next_v = not_visited.pop(random.randint(0, len(not_visited)-1))
        visited.append(next_v)
        edge = [curr_v, next_v]
        edge.sort()
        edge = tuple(edge)
        spanning_tree.append(edge)
        available_edges.remove(edge)
        curr_v = visited[random.randint(0, len(visited)-1)]

    while len(spanning_tree) < desired_m:
        a, b = available_edges.pop(random.randint(0,len(available_edges)-1))
        spanning_tree.append((a,b))

    generated_weights = set()

    print(f"desired_density = {density}\nreal density = {len(spanning_tree)/clique_m}", file=sys.stderr)
    print(size, len(spanning_tree), file=file)
    weight = random.uniform(*weight_range)

    for (u,v) in spanning_tree:
        while weight in generated_weights:
            weight = random.uniform(*weight_range)
        
        generated_weights.add(weight)

        print(f"{u} {v} {weight}", file=file)

nums = range(50, 1250, 50)

densities = [0, 0.2, 0.4, 0.6, 0.8, 1]

for n in nums:
    for d in densities:
        with open(f"./graphs/random/random_{n}_{d}.txt", "w") as f:
            generate_graph(n, d, file=f)