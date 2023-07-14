import sys
from math import sqrt

print("\\documentclass{article}")
print("\\usepackage{multicol}")
print("\\begin{document}")
print("\\begin{multicols}{3}")

for lines in map(str.rstrip, sys.stdin):
    line = lines.strip('\n').split(',')
    n = int(sqrt(len(line)))
    rows_str = '|c'*n
    print("\\begin{displaymath}")
    print("\\begin{array}{" + rows_str + "|}")
    for i in range(0, n**2, n):
        print('&'.join(line[i:i+n]) + '\\\\')
    print("\\end{array}")
    print("\\end{displaymath}")

print("\\end{multicols}")
print("\\end{document}")

