#!/usr/bin/env python
import sys
import collections


items = []
individual = []
tris = []

# extract out named entities of some kind
for line in sys.stdin:
    
    if not line.startswith("0"):
        continue
        
    rhs = line.split("(", 1)[1]
    
    context = rhs.split(";")
    
    items.append(context[1])
    
    
    individual += context[1].split()
    
    one = context[0].split()
    two = context[1].split()
    three = context[2].split()
    
    for i in one:
        for j in two:
            for k in three:
                tris.append("{} - {} - {}".format( i,j,k))
    


counter = collections.Counter(items)

c = counter.items()  # (elem, count)
c = sorted(c, key=lambda x: x[1])

for s, val in c:
    sys.stdout.write("{} - {}\n".format(val, s))


sys.stdout.write("-" * 80)
sys.stdout.write("\n- Individual Tokens \n")


counter = collections.Counter(individual)

c = counter.items()  # (elem, count)
c = sorted(c, key=lambda x: x[1])

for s, val in c:
    sys.stdout.write("{} - {}\n".format(val, s))
    
counter = collections.Counter(tris)

c = counter.items()  # (elem, count)
c = sorted(c, key=lambda x: x[1])

for s, val in c:
    if val < 5:
        continue
    sys.stdout.write("{} - {}\n".format(val, s))


