import sys

file = sys.argv[1].strip()
data = sys.argv[2].strip()

with open(file, 'w') as f:
    f.write(data)