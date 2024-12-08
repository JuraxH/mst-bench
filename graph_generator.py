import random
from itertools import combinations
import sys

def generate_graph(size, density, weight_range=(1, 10)):
    clique_m = size * (size - 1) // 2    
    desired_m = int(density * clique_m)

    #build spanning tree
    spanning_tree = list()
    not_visited = list(range(0, size))
    curr_v = not_visited.pop(random.randint(0, size-1))
    while len(not_visited) > 0:
        next_v = not_visited.pop(random.randint(0, len(not_visited)-1))
        spanning_tree.append((curr_v, next_v))
        curr_v = next_v

    while len(spanning_tree) < desired_m:
        a = b = random.randint(0,size-1)
        while a == b:
            b = random.randint(0, size-1)
        if (a,b) not in spanning_tree and (b, a) not in spanning_tree:
            spanning_tree.append((a,b))

    print(f"desired_density = {density}\nreal density = {len(spanning_tree)/clique_m}", file=sys.stderr)
    print(size, len(spanning_tree))
    for (u,v) in spanning_tree:
        #TODO: what distribution? try multiple and perhaps even measure multiple
        print(f"{u} {v} {random.uniform(*weight_range)}")

generate_graph(10, 0.1)