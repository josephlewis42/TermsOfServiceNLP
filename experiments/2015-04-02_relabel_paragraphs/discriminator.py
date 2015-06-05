#!/usr/bin/env python
''' This discriminator should extract dictionary sentences from TOS agreements
after being trained on some extracted definitions.

'''

import re
import string
import random

from sklearn import metrics
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.pipeline import FeatureUnion
from sklearn.naive_bayes import MultinomialNB
from sklearn.linear_model import Perceptron
from sklearn import linear_model
from sklearn.svm import SVC
from sklearn.preprocessing import Normalizer
from sklearn.pipeline import Pipeline
import sklearn

import numpy as np


plain_defns_file = "data/plain_defns.txt"
other_lines_file = "data/corpus_first_5k_lines_sents.txt"

def fuzzy_clean(text):
    '''Cleans text for a "fuzzy" match
    '''
    
    pattern = re.compile('[\W]+')
    text = text.lower()
    return pattern.sub("", text)


def read_plain_defns():
    with open(plain_defns_file) as fd:
        doc = fd.read()
        doc = ''.join([i if ord(i) < 128 else '' for i in doc])
        return doc.split("\n")
        
        

def read_non_defns(defns):
    deftable = set(map(fuzzy_clean, defns))
    
    sents = []
    with open(other_lines_file) as fd:
        doc = fd.read()
        doc = ''.join([i if ord(i) < 128 else '' for i in doc])
        sents = doc.split("\n")
    
    # remove any existing definitions
    sents = [s for s in sents if not fuzzy_clean(s) in deftable]
    return sents


def get_train_and_test():
    # get training and test data from the corpus sentences, checking if they're in the raw along the line
    defns = read_plain_defns()
    non   = read_non_defns(defns)
    non = non[: len(defns)]
    
    data = [(t, 1) for t in defns] + [(t, 0) for t in non]
    
    random.shuffle(data)
    
    return data




def split_data(split_train, split_test, data):
    total = (split_train + split_test) * 1.0
    
    # normalize
    split_train = split_train / total
    split_test = split_test / total
    
    dlen = len(data)
    tlen = int(dlen * split_train)
    
    train = data[:tlen]
    test = data[tlen:]
    
    return train, test
    



class ParenTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
        return np.array([[1] if "(" in x else [0] for x in X])
        
    def fit(self, X, y=None, **fit_params):
        return self
        
        

class QuoteParenTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
        import re
        a = re.compile("\\(.+\\\".+\\)")

        out = []
        for x in X:
            if a.search(x):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self

class DictTransform(sklearn.base.TransformerMixin):
    dictterms = ["meaning", "means", "defined","definition", "collectively", "aka", "a.k.a.", "e.g.", "eg.", "eg ", "(ie ", "(i.e.", "also known as", "such as"]
    def transform(self, X, **transform_params):
        out = []
        for x in X:
            if any([d in x.lower() for d in self.dictterms]):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self


class SingleLetterTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
    
        import re
        a = re.compile("\\([A-Za-z0-9]\\)")

        out = []
        for x in X:
            if a.search(x):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self

class PhoneTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
    
        import re
        a = re.compile("\\([0-9\\+\\- ]+\\)")

        out = []
        for x in X:
            if a.search(x):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self


class CapsQuoteTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
    
        import re
        a = re.compile("\\([A-Z0-9 ]+\\)")

        out = []
        for x in X:
            if a.search(x):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self

class OneWordTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
    
        import re
        a = re.compile("\\([A-Za-z][A-Za-z]+\\)")

        out = []
        for x in X:
            if a.search(x):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self

class RomanTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
    
        import re
        a = re.compile("\\([ivxIVXLl]+\\)")

        out = []
        for x in X:
            if a.search(x):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self
        
class IncludingTransform(sklearn.base.TransformerMixin):
    dictterms = ["(including", "(\"", ".com)", ".org)", ".net)"]
    def transform(self, X, **transform_params):
        out = []
        for x in X:
            if any([d in x.lower() for d in self.dictterms]):
                out.append([1])
            else:
                out.append([0])
            
        return np.array(out)
        
    def fit(self, X, y=None, **fit_params):
        return self
    
    

def get_discriminator():
    
    data = get_train_and_test()
    train, test = split_data(.8, .2, data)
    
    
    text_clf = Pipeline([('union', 
                            FeatureUnion([
                                #('vect', CountVectorizer(ngram_range=(1,2), binary=True)),
                                ('inc', IncludingTransform()),
                                ('phone', PhoneTransform()),
                                #('paren', ParenTransform()),
                                ('caps', CapsQuoteTransform()),
                                ('dict', DictTransform()),
                                ('single_transform', SingleLetterTransform()),
                                ('quote', QuoteParenTransform()),
                                ('roman', RomanTransform()),
                                ('one',OneWordTransform())
                            ])
                         ),
                         ('perceptron', SVC())])#(class_weight='auto'))])

    traindata, traintarget = zip(*train)
    #traindata = map(list, traindata)
    #print traindata
    #print traintarget
    algorithm = text_clf.fit(list(traindata), list(traintarget))
    
    datatest, targettest = zip(*test)
    datatest = list(datatest)
    targettest = list(targettest)
    
    predicted = text_clf.predict(datatest)
    
    print "Correct mean: {}".format(np.mean(predicted == targettest))            
    print(metrics.classification_report(targettest, predicted))
    
    for i, v in enumerate(predicted):
        if targettest[i] == 1 and v != 1:
            #print "Wrong negative":
            print datatest[i]
    
    return algorithm
    
def get_output_with_scores():
    '''Returns a set of tuples (definition, score) for chance that they are a defn.
    Scores are in real numbers
    '''
    d = get_discriminator()
    
    others = read_non_defns(read_plain_defns())
    
    #p = d.predict(others)
    c = d.decision_function(others)
    
    return zip(others, c)
    
if __name__ == "__main__":
    data = get_output_with_scores()
    for k, score in data:
        #if score > .9:
        print k, score

    exit()
    d = get_discriminator()
    
    others = read_non_defns(read_plain_defns())
    
    p = d.predict(others)
    c = d.decision_function(others)
    
    print sum(p) / (1.0 * len(p))
    
    for i, v in enumerate(p):
        if v == 1 and c[i] > .9:
            print "-" * 10
            print c[i]
            print others[i]

