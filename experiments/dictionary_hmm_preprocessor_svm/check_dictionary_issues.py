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


def extract_examples(classname):
    jsondocs = find_json()
    examples = []

    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            for paragraph in marked['paragraphs']:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                if classname in classes:
                    examples.append(text)

    return examples

extracted_raw = extract_examples('d')

concat = []

found = 0
unfound = 0

for c in extracted_raw:
    if len(lib.definition_extractor.definition_extractor(c)) > 0:
        found += 1
        continue
    else:
        unfound += 1
        print "-" * 80
        print c

print "=" * 80
print "Summary:"
print "Unfound: {}".format(unfound)
print "Found:   {}".format(found)

