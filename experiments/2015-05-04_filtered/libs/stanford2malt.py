#!/usr/bin/env python2

import sys

if __name__ == "__main__":
    for line in sys.stdin:
		if line.strip() == "":
			sys.stdout.write("\n")
		else:
			split = line.split("\t")
			split = split[:-1]  # don't need the last line
			
			split[4] = split[3]
			sys.stdout.write("\t".join(split) + "\n")

