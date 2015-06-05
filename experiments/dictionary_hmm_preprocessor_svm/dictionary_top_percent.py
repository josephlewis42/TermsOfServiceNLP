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


def extract_examples(classname, proportion=1.0, minprop = 0):
    jsondocs = find_json()
    examples = []

    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            numparagraphs = 1.0 * len(marked['paragraphs'])
            for i, paragraph in enumerate(marked['paragraphs']):
                if i / float(numparagraphs) < minprop:
                    continue
                
                # make sure we don't do too many lookups if requested
                if i / float(numparagraphs) > proportion:
                    break
                
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                if classname in classes:
                    examples.append(text)

    return examples


def do_processing(proportion, printall, proportionmin = 0):
    extracted_raw = extract_examples('d', proportion, proportionmin)

    concat = []

    found = 0
    unfound = 0

    for c in extracted_raw:
        if len(lib.definition_extractor.definition_extractor(c)) > 0:
            found += 1
            if printall:
                print "-" * 80
                print c
            continue
        else:
            unfound += 1
            print "-" * 80
            print c

    print "_" * 80
    print "Summary for definition extraction over first {}% of document:".format(proportion * 100)
    print "Unfound: {}".format(unfound)
    print "Found:   {}".format(found)
    print "-----------"
    print "Total:   {}".format(found + unfound)
    
    print "\n" * 2
    print "=" * 80


do_processing(1.0, False)


do_processing(.3, True, .2)
do_processing(.2, True, .1)
do_processing(.1, True)
