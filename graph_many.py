import re
import sys
import collections
try:
    import plotly.express as px
except ImportError:
    print('You need to install plotly.express.')
    sys.exit(1)
try:
    import kaleido
except ImportError:
    print('You need to install kaleido.')
    sys.exit(1)

if len(sys.argv) != 2:
    print('Usage: %s results.txt' % sys.argv[0])
    print('Where results.txt is the output of the benchmark script. I.e.')
    print(' mimalloc-bench/out/bench/benchres.csv')
    print()
    print('The script is designed to visualise the results of running the main benchmark')
    print('script with multiple runs for each benchmark. For example running:')
    print('  ../../bench.sh -r=10 mi sn allt')
    print('from the mimalloc-bench/out/bench directory.')
    print()
    print('The script will create two graphs, one for time and one for memory.')
    print('The graphs will be saved as time.html, time.png and time.pdf, and')
    print('memory.html, memory.png and memory.pdf.')
    print()
    print('The graphs will be normalised by the mean time/memory for each benchmark.')
    print('The normalised time/memory uses a log scale.')
    sys.exit(1)

def parse_time(time_string):
    parts = time_string.split(':')
    if len(parts) == 3: # H:MM:SS.ss
        return int(parts[0]) * 3600 + int(parts[1]) * 60 + float(parts[2])
    elif len(parts) == 2: # MM:SS.ss
        return int(parts[0]) * 60 + float(parts[1])
    else:
        return float(parts[0])

parse_line = re.compile('^([^ ]+) +([^ ]+) +([0-9:.]+) +([0-9]+)')
data = []
test_names = set()

# read in the data
with open(sys.argv[1]) as f:
    for l in f.readlines():
        match = parse_line.search(l)
        if not match:
            continue
        test_name, alloc_name, time_string, memory = match.groups()
        time_taken = parse_time(time_string)
        test_names.add(test_name)
        data.append({"Benchmark":test_name, "Allocator":alloc_name, "Time":time_taken, "Memory":int(memory)})

# create a dictionary of means
time_means = collections.defaultdict(float)
memory_means = collections.defaultdict(float)
for test_name in test_names:
    # calculate the mean
    test_data_time = [d['Time'] for d in data if d['Benchmark'] == test_name]
    test_data_mem = [d['Memory'] for d in data if d['Benchmark'] == test_name]
    if test_data_time:
        time_means[test_name] = sum(test_data_time) / len(test_data_time)
    if test_data_mem:
        memory_means[test_name] = sum(test_data_mem) / len(test_data_mem)

# add normalised time and memory to each data point
for d in data:
    d['Normalised Time'] = d['Time'] / time_means[d['Benchmark']] if time_means[d['Benchmark']] != 0 else 1.0
    d['Normalised Memory'] = d['Memory'] / memory_means[d['Benchmark']] if memory_means[d['Benchmark']] != 0 else 1.0

# create the graph for time
fig = px.box(data, x="Benchmark", y="Normalised Time", color="Allocator", log_y=True)
fig.update_xaxes(showgrid=True)
fig.update_yaxes(title_text="Normalised Time (log(time/mean time))")
fig.update_layout(boxgroupgap=0.6)
fig.update_traces(line_width=0.5, marker_line_width=0.5, marker_size=2, marker_symbol='x-thin')
fig.write_html(f"time.html")
fig.write_image(f"time.png")
fig.write_image(f"time.pdf")

# create the graph for memory
fig = px.box(data, x="Benchmark", y="Normalised Memory", color="Allocator", log_y=True)
fig.update_xaxes(showgrid=True)
fig.update_yaxes(title_text="Normalised Memory (log(memory/mean memory))")
fig.update_layout(boxgroupgap=.6)
fig.update_traces(line_width=0.5, marker_line_width=0.5, marker_size=2, marker_symbol='x-thin')
fig.write_html(f"memory.html")
fig.write_image(f"memory.png")
fig.write_image(f"memory.pdf")
