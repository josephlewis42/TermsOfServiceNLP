#!/usr/bin/env python


import random
import json
import re
import os

import sys
sys.path.insert(1,'/home/joseph/Desktop/tos3/')
import lib.definition_extractor

if len(sys.argv) == 1:
    print "Usage: {} classid".format(sys.argv[0])
    print "\tExtracts all paragraphs corresponding to a character and prints to stdout"
    print {'a': 'Anonynimity', 
            'c': 'Data Copyright', 
            'd': 'Definitions', 
            'h': 'Heading', 
            'i': 'Personal Information', 
            'j': 'Jurisdiction', 
            'l': 'Law Enforcement', 
            'm': 'Hold Harmless', 
            'o': 'Data Collection', 
            'p': 'Policy/Business Changes', 
            'r': 'Rights/Responsabilities', 
            's': 'Suspension', 
            't': 'Third Party', 
            'v': 'Leaving Service'}
    exit(1)

desired_class = sys.argv[1]


def find_json():
    jsons = []
    for path, subdirs, files in os.walk("../../tosback2/crawl"):
        for name in files:
            if name.endswith(".json"):
                jsons.append(os.path.join(path, name))
    random.shuffle(jsons)
    return jsons

import collections

pos = collections.defaultdict(list)


jsondocs = find_json()
examples = []

import itertools

for jdoc in jsondocs:
    with open(jdoc) as fd:
        marked = json.loads(fd.read())
        plength = len(marked["paragraphs"])
        for i, paragraph in enumerate(marked['paragraphs']):
            if desired_class not in paragraph["classes"]:
                continue
            
            sys.stdout.write(paragraph["paragraph"])
            sys.stdout.write("\n")
