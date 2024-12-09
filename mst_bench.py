#!/bin/python3
import os
import json
import subprocess
import pandas as pd
import argparse

binary_path = "build/mst-bench"

def run_and_collect_json(command):
    try:
        result = subprocess.run(
            command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True
        )
        json_output = json.loads(result.stdout)
        return json_output

    except subprocess.CalledProcessError as e:
        print(f"Error running binary: {e.stderr}")
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

    return None


def check_if_graphs_valid(graphs):
    ok = True
    for graph in graphs:
        info = run_and_collect_json([binary_path, 'info', graph])
        if not info['connected']:
            ok = False
            print(f'{graph} is not connected')
        if not info['unique_weights']:
            ok = False
            print(f'weights in {graph} are not unique')
    return ok

def test_if_correct(graphs):
    ok = True
    for graph in graphs:
        algs = run_and_collect_json([binary_path, 'test', graph])
        for name, valid in algs.items():
            if not valid:
                ok = False
                print('{name} fails on {graph}')
    return ok
            
def collect_info(graphs):
    infos = []
    for graph in graphs:
        info = run_and_collect_json([binary_path, 'info', graph])
        info['path'] = graph
        infos.append(info)
    return pd.DataFrame(infos)

def collect_runtime(graphs):
    runtimes = []
    for graph in graphs:
        res = run_and_collect_json([binary_path, 'bench', graph])
        for name, time in res.items():
            runtimes.append({'path' : graph, 'alg' : name, 'time' : time})
    return pd.DataFrame(runtimes)


def load_xavierwoo_dataset():
    graph_dir = "graphs/xavierwoo"
    graphs_file = os.path.join(graph_dir, "graphs.json")

    graphs = []
    with open(graphs_file, 'r') as f:
        graphs = json.load(f);
    for d in graphs:
        d['path'] = os.path.join(graph_dir, d['path'])

    return pd.DataFrame(graphs);


def main():
    parser = argparse.ArgumentParser(description='Runner script for mst-bench')
    parser.add_argument('action', help='[test|bench|info|check] what action to perform on graphs')
    parser.add_argument('graph_dir', help='directory with graph files')
    parser.add_argument('outfile', help='where to store csv', default='')

    args = parser.parse_args()

    if args.outfile == '':
        args.outfile = f'{args.action}.csv'

    random_graphs = list(map(lambda x: os.path.join(args.graph_dir, x), os.listdir(args.graph_dir)))
    df = pd.DataFrame({'path': random_graphs})

    if args.action == 'test':
        if test_if_correct(df['path']):
            print('all algorithms correct')
        else:
            print('not all algorithms computed valid mst')
    elif args.action == 'info':
        res = collect_info(df['path'])
        res.to_csv(args.outfile)
    elif args.action == 'bench':
        res = collect_runtime(df['path']);
        res.to_csv(args.outfile)
    else:
        print('not valid action')

if __name__ == '__main__':
    main()

# graph_dir = 'graphs/random'
# random_graphs = list(map(lambda x: os.path.join(graph_dir, x), os.listdir(graph_dir)))
# df = pd.DataFrame({'path': random_graphs})
#
# infos = collect_info(df['path'])
# infos.to_csv('random_graphs_stats.csv')

# if not check_if_graphs_valid(df['path']):
#     print('not all graphs are valid')


# if test_if_correct(df['path']):
#     print('all algorithms correct')
# else:
#     print('not all algorithms computed valid mst')

# info = []
# for graph in graphs:
#     print(f'runing on {graph}')
#     info.append(run_and_collect_json([binary_path, 'info', graph['path']]))


