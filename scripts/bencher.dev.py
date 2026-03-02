# This script converts the benchmark outputs to the format required by bencher.dev.
# It creates a file for each allocator with the results in JSON format.
# It generates file names of the form bencher.dev.<allocator>.json.
# Output files will contain the mean, high, and low values for memory and time for each benchmark:
# {
#    "<Benchmark>":{
#      "memory":{
#        "value": <memory-mean>
#        "high-value": <memory-high>
#        "low-value": <memory-low>
#      }
#      "time":{
#        "value": <time-mean>
#        "high-value": <time-high>
#        "low-value": <time-low>
#      }
#    }
# }
#
# These can be submitted to bencher.dev for analysis, e.g. a github action step like:
#    # Upload to graphing service
#    - uses: bencherdev/bencher@main
#    - name: Upload benchmark results to Bencher
#      run: |
#         bencher run \
#          --project snmalloc \
#          --token '${{ secrets.BENCHER_DEV_API_TOKEN }}' \
#          --branch ${{ github.ref_name }} \
#          --adapter json \
#          --err \
#          --file bencher.dev.sn.json

import re
import sys
import json
import collections

if len(sys.argv) != 2:
    print('Usage: %s benchres.csv' % sys.argv[0])
    print('Where benchres.csv is the output of the benchmark script. I.e.')
    print(' mimalloc-bench/out/bench/benchres.csv')
    print()
    print('The script generates a file per allocator for submission to bencher.dev.')
    sys.exit(1)

def parse_time(time_string):
    parts = time_string.split(':')
    if len(parts) == 3: # H:MM:SS.ss
        return int(parts[0]) * 3600 + int(parts[1]) * 60 + float(parts[2])
    elif len(parts) == 2: # MM:SS.ss
        return int(parts[0]) * 60 + float(parts[1])
    else:
        return float(parts[0])

# Match line format: benchmark allocator elapsed rss user sys faults reclaims
parse_line = re.compile(r'^([^ ]+)\s+([^ ]+)\s+([0-9:.]+)\s+([0-9]+)')
data = []
test_names = set()
alloc_names = set()

# read in the data
with open(sys.argv[1]) as f:
    for l in f.readlines():
        match = parse_line.search(l)
        if not match:
            continue
        test_name, alloc_name, time_string, memory = match.groups()
        time_taken = parse_time(time_string)
        test_names.add(test_name)
        alloc_names.add(alloc_name)
        data.append({"Benchmark":test_name, "Allocator":alloc_name, "Time":time_taken, "Memory":int(memory)})

for alloc in alloc_names:
    output = {}
    for test_name in test_names:
        alloc_test_data = [d for d in data if d["Benchmark"] == test_name and d["Allocator"] == alloc]
        if not alloc_test_data:
            continue

        mem_vals = [d["Memory"] for d in alloc_test_data]
        time_vals = [d["Time"] for d in alloc_test_data]

        output[test_name] = {
            "memory": {
                "value": sum(mem_vals) / len(mem_vals),
                "high-value": float(max(mem_vals)),
                "low-value": float(min(mem_vals)),
            },
            "time": {
                "value": sum(time_vals) / len(time_vals),
                "high-value": float(max(time_vals)),
                "low-value": float(min(time_vals)),
            }
        }

    if output:
        result = json.dumps(output, indent=2)
        with open(f"bencher.dev.{alloc}.json", "w") as f:
            f.write(result)
        print(f"Output written to bencher.dev.{alloc}.json")
