
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
        
        process(joined_again, termline)
        '''
        definition = defline.split(":")[1].strip()
        
        definition = definition.replace("``", "\"")
        definition = definition.replace("''", "\"")
        definition = definition.replace("/", " / ")
        
        definition = word_tokenize(definition)
        
        text = joined_again.split()
        
        
        j = 0
        for i in range(len(text)):
            if text[i:i + len(definition)] == definition:
            
            
                arr = []
                unbefore = text[:i-2]
                pre = text[i - 2:i]
                within = text[i:i + len(definition)]
                post = text[i + len(definition):i + len(definition) + 1]
                after = text[i + len(definition) + 1:]
                
                
                arr += [(u,"-") for u in unbefore]
                arr += [(p,"PRE") for p in pre]
                arr += [(w, "TARGET") for w in within]
                arr += [(p, 'POST') for p in post]
                arr += [(a,"-") for a in after]
                
                data.append(arr)
                
                
                
                break
        
        
        #datasplit.append() 
        '''
        

train = data[:int(len(data) * .8)]
test = data[int(len(data) * .8):]

class InParenTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
    
        ret = [[0]] * len(X)
        
        inside = False
        for i, c in enumerate(X):
            if X[i] == "(":
                inside = True
            elif X[i] == ")":
                inside = False
            
            ret[i] = [1] if inside else [0]
            
        return np.array(ret)
    
    def fit(self, X, y=None, **fit_params):
        return self

class CommaTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
        ret = [[1] if x in "[.,:;?!\[\]\"'()]" else [0] for x in X]
        return np.array(ret)
    
    def fit(self, X, y=None, **fit_params):
        return self


class CapsTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
        return np.array([[t[0].isupper()] for t in X])
    
    def fit(self, X, y=None, **fit_params):
        return self

class POSTransform(sklearn.base.TransformerMixin):
    def __init__(self):
        self.le = preprocessing.LabelEncoder()
    def transform(self, X, **transform_params):
        pos = pos_tag(X)
        tags = [t[1] for t in pos]
        pos = self.le.fit_transform(tags)
        return np.array([[t] for t in pos])
    
    def fit(self, X, y=None, **fit_params):
        return self


if __name__ == "__main__":
        
    text_clf = Pipeline([
        ('features', FeatureUnion([
            ("caps",CapsTransform()),
            ("comma", CommaTransform()),
            ("pos", POSTransform()),
            ("paren", InParenTransform()),
            ("vect", CountVectorizer(binary=True, ngram_range=(1,2)))
        ])),
        #('svc', MultinomialNB())
        ('maxent', LogisticRegression(class_weight="auto"))
        #('maxent', SVC(class_weight="auto"))
    ])
    
    for t in train:
        trn, tcls = [d[0] for d in t], [d[1] for d in t]
        text_clf = text_clf.fit(trn, tcls)  
      

    
    crct_total = 0
    total = 0
    

    testcls = []
    predicted = []
    
    # development
    for t in test:
        trn, tcls = [d[0] for d in t], [d[1] for d in t]
        testcls += tcls
        pd = text_clf.predict(trn)
        
        predicted += pd
        crct_total += sum(pd == tcls)
        total += len(pd)
    
    print "correct total: {}".format(crct_total)
    print "Correct mean: {}".format(crct_total * 1.0 / total)
    
    print(metrics.classification_report(testcls, predicted))
    


    means = []
    for t in test:
    
        x,y = [d[0] for d in t], [d[1] for d in t]
        predicted = text_clf.predict(x)
        mean = np.mean(predicted == y)
        means.append(mean)
        
        if mean < 40:
            print "Incorrect: {}".format(mean)
            print
            print x
            print y
            print predicted
            print "----"
        
    print "Correct {} / {}".format(len(filter(lambda x: x == 1, means)), len(means))
    
    for i in range(0, 11):
        num = 0
        for x in means:
            if x >= .1 * i and x < .1 * (i + 1):
                num += 1
                
        print "{}%-{}%: {} / {}".format(i * 10, (i + 1) * 10, num, len(means))
    
    
    try:
        import bashplotlib.histogram as h
        h.plot_hist(means, height=15, bincount=30, xlab=True)
    except ImportError:
        pass
        '''
    # test
    
    predicted = text_clf.predict(test)
    print "Correct mean: {}".format(np.mean(predicted == testcls))            
    print(metrics.classification_report(testcls, predicted))
    
    # Compute confusion matrix
    #print confusion_matrix(testcls, predicted)

    
'''
