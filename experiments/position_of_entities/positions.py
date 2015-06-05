#!/usr/bin/env python
import os
import random
import json
import re


import sys
sys.path.insert(1,'/home/joseph/Desktop/tos3/')
import lib.definition_extractor

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
            prop = i / float(plength)
            text = paragraph["paragraph"]
            
            tags = []
            if "law enf" in text or "government" in text:
                tags.append("law")
            if "third party" in text or "afiliate" in text:
                tags.append("third")
            if "we" in text or "us" in text or "our" in text:
                tags.append("corp")
            if "you" in text:
                tags.append("person")
            
            for t in tags:
                pos[t].append(prop)
                
            for t in itertools.combinations(tags, 2):
                t = sorted(list(t))
                t = "_".join(t)
                pos[t].append(prop)
            
            for t in itertools.combinations(tags, 3):
                t = sorted(list(t))
                t = "_".join(t)
                pos[t].append(prop)
            
            for t in itertools.combinations(tags, 4):
                t = sorted(list(t))
                t = "_".join(t)
                pos[t].append(prop)




for key in pos.keys():

    print "a <- c({})".format(",".join(map(str,pos[key])))
    print "hist(a, main=\"{}\",  xlim=c(0,1))".format(key)

