#!/usr/bin/env python2

''' Collects the distribution of paragraph types.
'''

import json
import os

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
            'v': 'Leaving Service'} 


def find_json():
    jsons = []
    for path, subdirs, files in os.walk("tosback2/crawl"):
        for name in files:
            if name.endswith(".json"):
                jsons.append(os.path.join(path, name))
    return jsons


def location_for_class(classname):
    '''
    
    Parameters:
        classname - the name of the class that should be tagged as 
        positive examples
    Returns:
        The locations of the paragraphs tagged with the given classname
        as a float in (0, 1] representing a proportion of the way through 
        with respect to the number of paragraphs.
    '''
    jsondocs = find_json()
    
    examples = []
    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            paragraphs = marked["paragraphs"]
            num_paragraphs = len(paragraphs)
            
            for pos, text in enumerate(paragraphs):
                classes = text["classes"]
                
                if classname not in classes:
                    continue
                    
                examples.append((pos + 1.0) / num_paragraphs)
    
    return examples


def find_locations():
    '''Finds the locations of the paragrpahs'''
    
    for key, value in classes.items():
        locs = location_for_class(key)
        locs_strs = [str(l) for l in locs]
        value = value.replace(" ", "_")
        value = value.replace("/", "_")
        print "{} <- c({})".format(value, ",".join(locs_strs))
    
if __name__ == "__main__":
    find_locations()
