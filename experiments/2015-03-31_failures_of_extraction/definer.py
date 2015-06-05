#!/usr/bin/env python
''' Pulls out definitions from sentences deemed to be defined
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
import nltk.chunk
import nltk.tokenize
import nltk.tag
import nltk
from sklearn import preprocessing

import numpy as np


random.seed(8675309)


plain_defns_file = "data/plain_defns.txt"
other_lines_file = "data/corpus_first_5k_lines_sents.txt"

defns_extracted_file = "data/definitions_extracted.txt"

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
    

def sent_parse(text):
    '''
    parses a sentences returns:
    
    tokens, tags, chunks, and a list of named entities in list form
    '''
    tokens = nltk.tokenize.word_tokenize(text)
    tagged = nltk.tag.pos_tag(tokens)
    chunked = nltk.ne_chunk(tagged, binary=True)
    
    ne = []
    for i in chunked:
        if isinstance(i, nltk.tree.Tree):
            words = map(lambda x: x[0], i)
            ne.append(words)
        else:
            if i[1] == "PRP":
                ne.append([i[0]])
    
    #print text
    return tokens, tagged, chunked, ne


def find_sublist(sub, bigger):
    first, rest = sub[0], sub[1:]
    
    for i in range(0, len(bigger) - len(sub) + 1):
        #print bigger[i:i + len(sub)], sub, bigger[i:i + len(sub)] == sub
        if bigger[i:i + len(sub)] == sub:
            return i
    return -1
    
    
class DefinitionFeatureExtractor(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
        ret = []
        for line in X:
            ret.append(get_features(*line)) 
        
        return np.array(ret)
    
    def fit(self, X, y=None, **fit_params):
        return self

def get_features(termne, defnne, sentencetokens):

    termsub = find_sublist(termne, sentencetokens)
    if termsub == -1:
        print "error FIRST couldn't find {} in {}".format(termne, sentencetokens)
        return ""
    
    defsub = find_sublist(defnne, sentencetokens[termsub + len(termne):])
    if defsub < 0:
        defsub = find_sublist(defnne, sentencetokens[:termsub])
        if defsub < 0:
            print "error SECOND couldn't find {} in {}, firstword: {}".format(defnne, sentencetokens, termne)
            return ""

    defbefore = sentencetokens[:defsub]
    defafter = sentencetokens[len(defnne) + defsub:]
    
    termbefore = sentencetokens[:termsub]
    termafter = sentencetokens[len(termne) + termsub:]
    
    # get the positions of the between words
    between_begin = defsub + len(defnne) if defsub < termsub else termsub + len(termne)
    between_end = defsub if defsub > termsub else termsub
    
    between = sentencetokens[between_begin:between_end]
    
    features = []
    for word in between:
        features.append("between_word_{}".format(word))
    
    for word in defbefore[-2:]:
        features.append("def_pre_{}".format(word))
    
    for word in defafter[:2]:
        features.append("def_post_{}".format(word))
        
    for word in termbefore[-2:]:
        features.append("term_pre_{}".format(word))
    for word in termafter[:2]:
        features.append("term_post_{}".format(word))
    if len(between) <= 5:
        features.append("dist_lte_five")
    if len(between) > 5:
        features.append("dist_gt_five")
    if len(between) > 10:
        features.append("dist_gt_ten")
    if len(between) > 15:
        features.append("dist_gt_fifteen")
    
    for word in termne:
        if word in defnne:
            features.append("common_word")
    
    fuzzyterm = fuzzy_clean("".join(termne))
    fuzzydef = fuzzy_clean("".join(defnne))
    
    defcomb = "".join(defnne) 
    if defcomb == defcomb.upper():
        features.append("all_upper_case")
    
    dictterms = ["(including", "(\"", ".com)", ".org)", ".net)"]
    for d in dictterms:
        if d in defcomb:
            features.append("dictterm_in")
            break

    return " ".join(features)


def create_examples(sentence, term, definition):
    ''' Creates a set of training examples from a given sentence.
    
    '''
    st, sta, sc, sne = sent_parse(sentence)
    tt, tta, tc, tne = sent_parse(term)
    dt, dta, dc, dne = sent_parse(definition)
    
    tne = []
    dne = []
    
    for ex in sne:
        if find_sublist(ex, tt) >= 0:
            tne.append(ex)
        if find_sublist(ex, dt) >= 0:
            dne.append(ex)
    
    examples = []
    for t_entity in tne:
        for d_entity in dne:
            examples.append(([t_entity, d_entity, st],1))
    
    
    for i, ne1 in enumerate(sne):
        for ne2 in sne[i + 1:]:

            if ne1 in dne or ne2 in tne:
                continue
            if ne2 in dne or ne1 in tne:
                continue
                
            

            examples.append(([ne1, ne2, st], 0))
    
    random.shuffle(examples)
    
    return examples

def get_examples():
    examples = []
    with open(defns_extracted_file) as fd:
        text = fd.read()
        
        # clean it up
        text = ''.join([i if ord(i) < 128 else '' for i in text])
        defns = text.split("---\n")
        

        
        for defn in defns:
            if defn == "":
                continue
            if "term:" not in defn or "definition:" not in defn:
                print "failed"
                continue
            lines = defn.split("\n")
            termline = [l for l in lines if l.startswith("term:")][0]
            defline = [l for l in lines if l.startswith("definition:")][0]
            term = termline.split(": ")[1]
            definition = defline.split(": ")[1]
            
            raw = lines[0]
            
            examples += create_examples(raw, term, definition)
    return examples

def get_discriminator():
    examples = get_examples()
    
    #examples = filter(lambda x: isinstance(x[0], str), examples)
    train, test = split_data(.8, .2, examples)
    
    print "Train size: {}".format(len(train))
    print "Test size:  {}".format(len(test))
    
    text_clf = Pipeline([
          ('features', DefinitionFeatureExtractor()),
          ('counts', CountVectorizer(tokenizer=lambda x: x.split())),
    ('perceptron', Perceptron(class_weight="auto"))])#(class_weight='auto'))])

    traindata, traintarget = zip(*train)
    traintarget = [i for i in traintarget]

    algorithm = text_clf.fit(list(traindata), traintarget)
    
    datatest, targettest = zip(*test)
    datatest = list(datatest)
    targettest = list(targettest)
    
    predicted = algorithm.predict(datatest)

    
    print "Correct mean: {}".format(np.mean(predicted == targettest))            
    print(metrics.classification_report(targettest, predicted))
    
    try:
        predicted_certanty = algorithm.decision_function(datatest)
    except NotImplementedError: # for svcs, etc
        predicted_certanty = ["err"] * len(datatest)
    
    for i, v in enumerate(predicted):
        if targettest[i] == 1 and v != 1:
            print "------------"
            print "Term: {}".format(datatest[i][0])
            print "Defn: {}".format(datatest[i][1])
            print "Certanty: {}".format(predicted_certanty[i]) 
            print "Sentence"
            print datatest[i][2]
    
    return algorithm


if __name__ == "__main__":
    get_discriminator()

