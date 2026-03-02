import re
import sys
import collections

try:
    import pygal
except ImportError:
    print('You need to install pygal.')
    sys.exit(1)

if len(sys.argv) != 2:
    print('Usage: %s results.txt' % sys.argv[0])
    sys.exit(1)

def parse_time(time_string):
    parts = time_string.split(':')
    if len(parts) == 3: # H:MM:SS.ss
        return int(parts[0]) * 3600 + int(parts[1]) * 60 + float(parts[2])
    elif len(parts) == 2: # MM:SS.ss
        return int(parts[0]) * 60 + float(parts[1])
    else:
        return float(parts[0])

parse_line = re.compile('^([^ ]+) +([^ ]+) +([0-9:.]+)')
allocs = collections.defaultdict(lambda: collections.defaultdict(dict))

with open(sys.argv[1]) as f:
    for l in f.readlines():
        match = parse_line.search(l)
        if not match:
            continue
        test_name, alloc_name, time_string = match.groups()
        time_taken = parse_time(time_string)
        allocs[test_name][alloc_name] = time_taken

for test_name, results in allocs.items():
    line_chart = pygal.Bar(logarithmic=True)
    line_chart.title = test_name + ' (in seconds)'
    for k, t in results.items():
        line_chart.add(k, t)
    with open('out-' + test_name + '.svg', 'wb') as f:
        f.write(line_chart.render())
