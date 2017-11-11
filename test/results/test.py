import os
import re
import math

root = '/home/veenstraaj/results'

step = 8589934592

files = os.listdir(root)
files = filter(lambda x: x.startswith('r') and not x.endswith('control') and os.path.isfile(os.path.join(root, x)),
               files)
options = {}
for f in files:
    props = re.match('^r(?P<size>\d+)(?P<hosts>[a-z_]+)(?P<iteration>\d)$', f).groupdict()
    size_dict = options.get(int(props['size']), {})
    hosts_dict = size_dict.get(props['hosts'], {})
    file = open(os.path.join(root, f), 'r')
    lines = file.readlines()
    file.close()
    time = float(re.match('.*Time elapsed \(seconds\) = (?P<time>[0-9.]+).*', lines[1]).groups()[0])
    hosts_dict[int(props['iteration'])] = time
    size_dict[props['hosts']] = hosts_dict
    options[int(props['size'])] = size_dict

lists = {x: [] for x in list(options.values())[0].keys()}

for size in sorted(options.keys()):
    for host in options[size].keys():
        iteration_dict = options[size][host]
        iteration_dict['average'] = time = sum(iteration_dict.values()) / len(iteration_dict)
        lists[host].append((size, time))

base = """
\\begin{{tikzpicture}}
\\begin{{axis}}[
    %title={{{}}},
    xlabel={{{}}},
    ylabel={{{}}},
    xmin=0, xmax={},
    ymin={}, ymax={},
    legend pos={},
    ymajorgrids=true,
    grid style=dashed,
    scaled x ticks={{real:10000000000}},
    width=0.47\linewidth,
]

{}

\\legend{{{}}}
\\end{{axis}}
\\end{{tikzpicture}}
"""

plot = """
\\addplot[
    color={},
    smooth,{}
    ]
    coordinates {{
    {}
    }};
"""

square = """
\\addplot[
    color=black,
    mark=none,
] coordinates {{({x1},{y1}) ({x1},{y2}) ({x2},{y2}) ({x2},{y1}) ({x1},{y1})}};

\pgfplotsset{{
    after end axis/.code={{
        \\node[above] at (axis cs:{x2},{y2}){{\\contour{{white}}{{Fig.~\\ref{{{zoom}}}}}}};
    }}
}}
"""

colors = {'ave': 'blue', 'slb': 'red', 'ave_slb': 'green'}
shapes = {'ave': 'mark=diamond,', 'slb': 'mark=triangle,', 'ave_slb': 'mark=square,'}

x1=0
x2=10000000000
y1=0
y2=12
file = open("/home/veenstraaj/Documents/editdistance/text/paper/parts/result_graph.tex", 'w')
file.write(base.format(
    "Average time to compute edit distance",
    "Number of cells",
    "Average time (s)",
    125000000000,
    0,
    150,
    'south east',
    ''.join(
        plot.format(
            colors[host],
            # shapes[host],
            '',
            ''.join(str(i) for i in lists[host])
        ) for host in ['ave', 'slb', 'ave_slb']
    ) + square.format(x1=x1,x2=x2,y1=y1,y2=y2,zoom='result_graph_zoom'),
    'Node 1, Node 2, Nodes 1 and 2',
))
file.close()

file = open("/home/veenstraaj/Documents/editdistance/text/paper/parts/result_graph_zoom.tex", 'w')
file.write(base.format(
    "Average time to compute edit distance",
    "Number of cells",
    "Average time (s)",
    x2,
    y1,
    y2,
    'south east',
    ''.join(
        plot.format(
            colors[host],
            # shapes[host],
            '',
            ''.join(str(i) for i in lists[host])
        ) for host in ['ave', 'slb', 'ave_slb']
    ),
    'Node 1, Node 2, Nodes 1 and 2',
))
file.close()

x1=0
x2=12000000000
y1=0.9 / 1000000000
y2=1.4 / 1000000000
file = open("/home/veenstraaj/Documents/editdistance/text/paper/parts/result_graph_cell.tex", 'w')
file.write(base.format(
    "Average time per cell",
    "Number of cells",
    "Average time (s)",
    20000000000,
    0.7 / 1000000000,
    7.0 / 1000000000,
    'north east',
    ''.join(
        plot.format(
            colors[host],
            # shapes[host],
            '',
            ''.join(
                str((i[0], i[1] / i[0])) for i in lists[host]
            )
        ) for host in ['ave', 'slb', 'ave_slb']
    ) + square.format(x1=x1,x2=x2,y1=y1,y2=y2,zoom='result_graph_cell_zoom'),
    'Node 1, Node 2, Nodes 1 and 2',
))
file.close()

file = open("/home/veenstraaj/Documents/editdistance/text/paper/parts/result_graph_cell_zoom.tex", 'w')
file.write(base.format(
    "Average time per cell",
    "Number of cells",
    "Average time (s)",
    x2,
    y1,
    y2,
    'north east',
    ''.join(
        plot.format(
            colors[host],
            shapes[host],
            ''.join(
                str((i[0], i[1] / i[0])) for i in lists[host]
            )
        ) for host in ['ave', 'ave_slb']
    ),
    'Node 1, Nodes 1 and 2',
))
file.close()
