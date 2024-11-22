import os
import json

dirs = list(filter(os.path.isdir, os.listdir()))
i = 0
files = []
for dir in dirs:
    for file in os.listdir(dir):
        path = os.path.join(dir, file)
        id = i
        i = i + 1
        item = {"path" : path, "id" : id, "format" : 1}
        files.append(item)

with open('graphs.json', 'w') as f:
    json.dump(files, f, indent=4) 
