
# -*- coding: latin-1 -*-

import sklearn
from sklearn.pipeline import Pipeline
import numpy as np
from sklearn.datasets import fetch_20newsgroups
from sklearn import metrics
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.pipeline import FeatureUnion
from sklearn.naive_bayes import MultinomialNB
from sklearn.linear_model import Perceptron
from sklearn import linear_model
from sklearn.svm import SVC
from sklearn.preprocessing import Normalizer
from sklearn.linear_model import LogisticRegression
from sklearn import preprocessing
import os
import re
import random
import numpy
from sklearn.neural_network import BernoulliRBM 
from sklearn.metrics import confusion_matrix
from nltk.tag import pos_tag 

import numpy as np
from scipy import sparse

from sklearn.metrics import hamming_loss
from sklearn.datasets import fetch_mldata
from sklearn.metrics import mutual_info_score
from scipy.sparse.csgraph import minimum_spanning_tree

from pystruct.learners import OneSlackSSVM
from pystruct.models import MultiLabelClf
from pystruct.datasets import load_scene
from nltk.tag import HiddenMarkovModelTrainer
from nltk.tokenize import word_tokenize
import nltk.tag.hmm



data = []


def process(joined_again, defline):

    definition = defline.split(":")[1].strip()
    
    # if no defn, just return all dashes.
    if definition == "":
        data.append([(u,"-") for u in joined_again.split()])
        return
    
    definition = definition.replace("``", "\"")
    definition = definition.replace("''", "\"")
    definition = definition.replace("/", " / ")
    
    definition = word_tokenize(definition)
    
    text = joined_again.split()
    
    j = 0
    for i in range(len(text)):
        if text[i:i + len(definition)] == definition:
        
        
            arr = []
            unbefore = text[:i]
            within = text[i:i + len(definition)]
            post = text[i + len(definition):i + len(definition) + 1]
            after = text[i + len(definition) + 1:]
            
            
            arr += [(u,"-") for u in unbefore]

            arr += [(w, "TARGET") for w in within]
            arr += [(p, '-') for p in post]
            arr += [(a,"-") for a in after]
            
            data.append(arr)            
            break

i = 0
with open("../../data/definitions_extracted.txt") as definitions:
    text = definitions.read()
    defs = text.split("---\n")
    
    for definition in defs:
        if not definition:
            continue
        i += 1
        
        lines = definition.split("\n")
        if len(lines) != 10:
            print("fail")
            print definition
            exit()

        raw = lines[0]
        cleaned = lines[1]
        joined_again = lines[2]
        pos_tagged = lines[3]
        hmm_tagged = lines[4]
        
        termline = lines[7]
        defline = lines[8]
        
        if termline.split(":")[1].strip() == "":
            continue
        
        print "---"
        print termline
        print defline
        
        
        definition = defline.split(":")[1].strip()
        term = termline.split(":")[1].strip()
        
        # if no defn, continue
        if definition == "" or term == "":
            continue
            
        def cleanup(definition):
            definition = definition.replace("``", "\"")
            definition = definition.replace("''", "\"")
            definition = definition.replace("/", " / ")
            definition = definition.replace("``", "\"")
            definition = definition.replace("''", "\"")
            definition = definition.replace("/", " / ")
            return definition
        
        definition = cleanup(definition)
        term = cleanup(term)
        definition = word_tokenize(definition)
        term = word_tokenize(term)
    
        text = joined_again.split()
        pos  = pos_tag(text)
                
        for i in range(len(text)):
            if text[i:i + len(definition)] == definition:
                
                print "definition:"
                print pos[i:i + len(definition)]
            if text[i:i + len(term)] == term:
                
                print "term:"
                print pos[i:i + len(term)]   
                
