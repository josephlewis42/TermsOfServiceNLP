#!/usr/bin/env python2

''' Contains project definitions for the terms of service parsing project.
'''

import collections
import re
import json
import nltk
import functools
import math
import lru
from sklearn.svm import SVC


classes = { 'a': 'Anonynimity', 
            'c': 'Data Copyright', 
            'd': 'Definitions', 
            #'h': 'Heading', # ignore these as they're fairly meaningless
            'i': 'Personal Information', 
            'j': 'Jurisdiction', 
            'l': 'Law Enforcement', 
            'm': 'Hold Harmless', 
            'o': 'Data Collection', 
            'p': 'Policy/Business Changes', 
            'r': 'Rights/Responsabilities', 
            's': 'Suspension', 
            't': 'Third Party', 
            'v': 'Leaving Service' 
            #'x': 'Data Export' # never used
            }

def clean_text(text):
    raw = text.lower()
    raw = re.sub('<[^<]+?>'," ", raw) # remove html        
    raw = re.sub('[^\\w\\s\\d]+', " ", raw) # remove extra weird characters and extra whitespace    
    
    return raw


STEMMER = nltk.stem.porter.PorterStemmer()    
@lru.lru_cache(maxsize=1000)
def stem_word(word):
    return STEMMER.stem(word)

def calculate_word_frequencies(text, stem=False):
    ''' Calculates word frequencies for a given text.'''
    frequencies = collections.defaultdict(int)
    
    raw = clean_text(text)
    
    tokens = raw.split() #nltk.word_tokenize(raw)
    
    for word in tokens:
        word = stem_word(word) if stem else word
        frequencies[word] += 1
    
    return frequencies
    
@lru.lru_cache()
def get_idf(corpusfile="corpus.txt"):
    '''Gets the idf of the corpus'''
    
    totalfreqs = collections.defaultdict(int)
    numdocs = 0.0
    
    with open(corpusfile) as corpus:
        for document in corpus.read().split("<p>"):
            numdocs += 1
            
            wf = calculate_word_frequencies(document, False)
            
            for key in wf.keys():
                totalfreqs[key] += 1
    
    idf_results = {}
    
    for term, val in totalfreqs.items():
        try:
            idf = math.log(numdocs / abs(val))
        except ZeroDivisionError:
            idf = 0
        
        idf_results[term] = idf
    
    return idf_results

def tf_idf(tf_document, idf):
    ''' Computes and returns tfidf scores for words in the document
    
    tf_corpus - the term frequency over all documents
    tf_document - the term frequency for the document
    
    '''
    output = {}
    for key in idf.keys():
        try:
            output[key] = tf_document[key] * idf[key]
        except KeyError:
            output[key] = 0
    
    return output
    
def train_svm_tfidf(positive_texts, negative_texts, idf, label):
    ''' Trains an SVM with annoted tfidf documents 
    
    tf_documents, an array of documents where the property is true or false
    '''
    
    train_set = []
    
    idf = get_idf()
    
    for text in positive_texts:
        tf = calculate_word_frequencies(text)
        feature_vector = tf_idf(tf, idf)
        
        train_set.append((feature_vector, label))
    
    for text in negative_texts:
        tf = calculate_word_frequencies(text)
        feature_vector = tf_idf(tf, idf)
        
        train_set.append((feature_vector, "not " + label))
    
    
    return nltk.classify.SklearnClassifier(SVC()).train(train_set)


def get_word_frequencies():
    ''' Returns the word frequencies of the corpus.'''
    try:
        with open("frequencies.json") as freqfile:
            return json.loads(freqfile.read())
    except IOError:
        # build a corpus using the legal documents, remember we should strip features.
        with open("corpus.txt") as rawfile:
            raw = rawfile.read()
            
            frequencies = calculate_word_frequencies(raw)
            
            with open("frequencies.json", "w") as freqfile:
                freqfile.write(json.dumps(frequencies))
    
    return frequencies

def remove_stopwords(frequency_map):
    from nltk.corpus import stopwords
    stops = stopwords.words('english')
    
    for stop in stops:
        if stop in frequency_map:
            del frequency_map[stop]


def get_n_most_frequent(n, wordfreq):
    '''Gets the n most frequent words in the set.
    
    n - the number to fetch
    wordfreq - the set of frequencies to fetch from
    '''

    od = collections.OrderedDict(sorted(wordfreq.items(), key=lambda t: -t[1]))
    
    mf = {}
    for i in range(n):
        if len(od) == 0:
            break
        k, v = od.popitem(False)
        mf[k] = v
    
    return mf
        

