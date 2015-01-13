#!/usr/bin/env python

from __future__ import print_function
import glob
import os
import re
import string
import json
import textwrap

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

classes = { 'a': 'Anonynimity', 
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
            'v': 'Leaving Service', 
            'x': 'Data Export'}



os.chdir("/home/joseph/Desktop/lang_project/tosback2/crawl")
for fname in glob.glob("*"):
    
    nsites += 1
    
    print("=" * 80)
    print(fname)
    
    os.chdir(fname)
    for policy in glob.glob("*.txt"):
        doc = []            
        pol = {}
        pol["site"] = fname
        pol["policy"] = policy


        with open(policy) as fp:
            rawtext = fp.read()
            rawtext = rawtext.replace("<p>", "\n\n\n\n")
            rawtext = re.sub('<[^<]+?>', '', rawtext)
            rawtext = "".join(filter(lambda x: x in string.printable, rawtext))
            
            for paragraph in rawtext.split("\n\n\n\n"):
                paragraph = paragraph.strip()
                if not paragraph:
                    continue
                
                enabled_classes = set()
                while True:
                
                    print("\n" * 80)
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
                    if char not in classes:
                        continue
                    if char in enabled_classes:
                        enabled_classes.remove(char)
                    else:
                        enabled_classes.add(char)
                

                p = {}
                
                p["paragraph"] = paragraph   
                p["classes"] = ",".join(enabled_classes)
                
                doc.append(p)
                
                #print("-" * 80)
                #print(paragraph)
                nparagraphs += 1
            pol["paragraphs"] = doc
        with open(policy + ".json", "w") as pjson:
            pjson.write(json.dumps(pol))
        
    os.chdir("..")
    
    
    
    if nsites == 20:
        break

print(nparagraphs)
