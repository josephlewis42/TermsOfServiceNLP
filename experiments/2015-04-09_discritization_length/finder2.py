#!/usr/bin/env python

from __future__ import print_function
import glob
import os
import re
import string
import json
import textwrap
import tos

nparagraphs = 0
nsites = 0


class _Getch:
    """Gets a single character from standard input.  Does not echo to the
screen."""
    def __init__(self):
        try:
            self.impl = _GetchWindows()
        except ImportError:
            self.impl = _GetchUnix()

    def __call__(self): return self.impl()


class _GetchUnix:
    def __init__(self):
        import tty, sys

    def __call__(self):
        import sys, tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch


class _GetchWindows:
    def __init__(self):
        import msvcrt

    def __call__(self):
        import msvcrt
        return msvcrt.getch()


getch = _Getch()

classes = tos.classes

def find_documents():
    jsons = set()
    for path, subdirs, files in os.walk("tosback2/crawl"):
        for name in files:
            if name.endswith(".json"):
                jsons.add(os.path.join(path, name))
    
    unclassified = []
    for path, subdirs, files in os.walk("tosback2/crawl"):
        for name in files:
            if name.endswith(".txt"):
                partial = os.path.join(path, name)
                
                if partial + ".json" not in jsons:
                    unclassified.append(partial)
                else:
                    #print("Skipping {} already classified".format(partial))
                    pass


    # Re-populate list with filename, size tuples
    for i in xrange(len(unclassified)):
        unclassified[i] = (unclassified[i], os.path.getsize(unclassified[i]))

    # Sort list by file size, smallest to largest
    unclassified.sort(key=lambda filename: filename[1], reverse=False)
    
    # Re-populate list with just filenames
    for i in xrange(len(unclassified)):
        unclassified[i] = unclassified[i][0]
    
    return unclassified
    
    

class_markers = { 'a': ["anonymity", "anonymous", "authenticate", "authentication", "impersonate"], 
            'c': ['copyright', "unlimited", "derivative", "reproduce", "grant", "license"], 
            'd': ["mean", "means", "defined", "incorporated", "collectively", 'hereinafter','(',')'], 
            'i': ['ip','address', 'e-mail', 'email', 'name', 'identifying'], 
            'j': ['arise', 'country', 'reside'], 
            'l': ['safety', 'suspected', 'court', 'judicial', 'law', 'enforcement'], 
            'm': ['loss','breach','risk','indemnify', 'defects','damage','waive', 'liability', 'liable'], 
            'o': ['pixel', 'operating', 'beacon', 'beacons', 'patterns', 'gps', 'ip', 'analytic', 'record', 'collect'], 
            'p': ['assets', 'transferred', 'transfer', 'periodically', 'acquired', 'bankruptcy', 'version', 'versions'], 
            'r': ['promptly', 'reverse', 'decompile', 'right', "responsibility"], 
            's': ['vulgar', 'impersonate', 'repeated', 'survive', 'violates', 'repeat'], 
            't': ['vendors', 'javascript', 'administration', 'hosted', 'third'], 
            'v': ['cached', 'voluntarily', 'uploaded', 'archived', 'anonymize', 'discontinue', 'terminate']
            }

def auto_classify(num_paragraphs_search, classes=None):
    if classes == None:
        classes = class_markers
    
    results = {}
    
    for fname in find_documents():
        with open(fname) as fp:
            rawtext = fp.read()
            paragraphs = rawtext.split("<p>")
            
            paragraphs = [tos.clean_text(p) for p in paragraphs]
            
            for i, paragraph in enumerate(paragraphs):
                
                # make sure we don't get just whitespace
                paragraph = paragraph.strip()
                if not paragraph:
                    continue
                    
                enabled_classes = _auto_classify_paragraph(paragraph)
                if any([c in classes for c in enabled_classes]):
                    #print("found p")
                    paragraph = ''.join([i for i in paragraph if ord(i) < 128])
                    results[paragraph.decode()] = 1 #",".join(enabled_classes)
                

            num_paragraphs_search -= 1
            
            if num_paragraphs_search <= 0:
                return results
    return results

    

def _auto_classify_paragraph(paragraph):
    enabled_classes = set()
    pwords = set(paragraph.split())
    for k in classes.keys():
        for marker in class_markers[k]:
            if marker in pwords:
                enabled_classes.add(k)
    
    return enabled_classes


def user_classify(paragraph, proportion, name):
    enabled_classes = set()
    
    # check the text for any of our keywords so we can do smart enabling.
    pwords = set(paragraph.split())
    for k in classes.keys():
        for marker in class_markers[k]:
            if marker in pwords:
                enabled_classes.add(k)
    
    # headings mostly, remove any triggers
    if len(paragraph) < 80:
        enabled_classes = set()
    
    
    # repeat for continual user input
    while True:
        # clear the screen
        print("\n" * 80)
        print("[{}]".format(name))
        print("=" * int(80  * proportion + .001) + " {}%".format(proportion * 100))
        print(textwrap.fill(paragraph))
        print("-" * 80)
        
        print("Enabled:")
        
        for i in enabled_classes:
            print("\t%s %s" % (i, classes[i]))
            
        print("\n\nDisabled:")
        for k, v in classes.items():
            if k in enabled_classes:
                continue
            print("\t%s %s" % (k,v))
        
        print("Add class or return for done: ")
        char = getch()
        
        if char in "\n\r \t":
            break
        
        # ignore characters that we don't care about
        if char not in classes:
            continue
        
        # add/remove chars as needed
        if char in enabled_classes:
            enabled_classes.remove(char)
        else:
            enabled_classes.add(char)
    
    return enabled_classes
                

if __name__ == "__main__":
    nfiles = 0
    
    for fname in find_documents():
        nfiles += 1    
        
        print("=" * 80)
        print(fname)
        
        

        doc = []            
        pol = {}
        print(fname)
        site, policy = fname.split("/")[-2:]
        pol["site"] = site
        pol["policy"] = policy

        with open(fname) as fp:
            rawtext = fp.read()
            paragraphs = rawtext.split("<p>")
            
            paragraphs = [tos.clean_text(p) for p in paragraphs]
            
            for i, paragraph in enumerate(paragraphs):
                
                # make sure we don't get just whitespace
                paragraph = paragraph.strip()
                if not paragraph:
                    continue
                    
                enabled_classes = user_classify(paragraph, i / float(len(paragraphs)),fname)

                p = {}
                
                p["paragraph"] = paragraph   
                p["classes"] = ",".join(enabled_classes)
                
                doc.append(p)
                
                nparagraphs += 1
        pol["paragraphs"] = doc
        with open(fname + ".json", "w") as pjson:
            pjson.write(json.dumps(pol))
        
    
        print("\n" * 80)
        print("Finished document, continue? Y/N ")
        
        if getch().lower() == "n":
            print("You classified {} paragraphs in {} documents".format(nparagraphs, nfiles))
            exit()

