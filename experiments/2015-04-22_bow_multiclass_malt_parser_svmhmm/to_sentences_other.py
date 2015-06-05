#!/usr/bin/env python2
'''
Do classification on the documents we have already converted to JSON
'''
import nltk.tokenize 
import json
import os
import re
import tos

def find_json():
    jsons = []
    for path, subdirs, files in os.walk("../../tosback2/crawl"):
        for name in files:
            if name.endswith(".json"):
                jsons.append(os.path.join(path, name))
    return jsons


def extract_examples():
    jsondocs = find_json()
    examples = []
    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            for paragraph in marked['paragraphs']:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                for classname in tos.classes.keys():
                	if classname in classes:
                		break
                else:
                	examples.append(text)
    return examples

def extract_sentences(examples):
	sentences = []
	for example in examples:
		sentences.extend(nltk.tokenize.sent_tokenize(example))
	return sentences

if __name__ == "__main__":
	examples = extract_examples()
	sentences = extract_sentences(examples)
	
	with open("other_sentences.txt", 'w') as fd:
		fd.write("\n".join(sentences))
