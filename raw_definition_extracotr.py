#!/usr/bin/env python2

import fileinput
import re
from nltk.tokenize import word_tokenize
import string
import nltk

num_allowed = 200

def strip_html(text):
    return re.sub('(<[^<]+?>|\\t\\r)'," ", text)

unclosed_line = ""
# read from stdinput
for line in fileinput.input():

    if num_allowed <= 0:
        continue
    
    unclosed_line += line
    
    unclosed_line = strip_html(unclosed_line)
    
    unclosed_line = filter(lambda x: x in string.printable, unclosed_line)

    unclosed_line = re.sub(r'\(\s?\w\s?\)', "", unclosed_line) # replace single letters with nothing
    unclosed_line = unclosed_line.replace("a.k.a.", "aka")
    unclosed_line = unclosed_line.replace("i.e.", "ie")
    unclosed_line = unclosed_line.replace("e.g.", "eg")
    
    lines = re.split(r"(\.\s+|\n)", unclosed_line)
    unclosed_line = lines[-1]
    
    for line in lines[:-1]:
        #print line

        # remove lines with only roman numerals
        tmpline = re.sub(r'\(\s?[\s?\.iIvVxX]+\s?\)', "", line) 
        
        # remove lines with only phone numbers
        tmpline = re.sub(r'\(\s?[\d\.-]+\s?\)', "", tmpline)
        
        if "(" in tmpline or "collectiv" in tmpline or "referred" in tmpline:
            
            print "---"
            print line
            line = " ".join(word_tokenize(line))
            line = line.replace("``", "\"")
            line = line.replace("''", "\"")
            line = line.replace("/", " / ")
            
            print line
            ls = line.split()
            l2 = " ".join(ls)
            print l2
            postup = ["{}/{}".format(a, b) for a, b in nltk.pos_tag(ls)]
            print " ".join(postup)
            print "/out ".join(ls) + "/out"
            print ""
            print "tos_definition"
            print "term: "
            print "definition: "
            
            num_allowed -= 1
