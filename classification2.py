#!/usr/bin/env python2
'''
Do classification on the documents we have already converted to JSON
'''
import nltk
import re
import collections
import json
import os
import random
import pickle
import lru
from nltk.tokenize import word_tokenize
from sklearn.svm import SVC
import datetime
import itertools
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
import os
import re
import random
import numpy
from sklearn.neural_network import BernoulliRBM 
import finder2
import tos

from nltk.stem.porter import PorterStemmer

NUM_MOST_FREQUENT = 50000
BIGRAM_FEATURES = False
BIGRAM_FEATURES_STR = "bigrams" if BIGRAM_FEATURES else "unigrams"

frequencies = tos.get_word_frequencies()
        
def remove_values_lt(dictionary, count=1):
    return {k:v for k, v in dictionary.items() if v > count}

#print len(frequencies)
#print len(remove_values_lt(frequencies, 30))


frequencies = tos.get_n_most_frequent(NUM_MOST_FREQUENT, frequencies) 
'''if BIGRAM_FEATURES:
    print len(frequencies.keys())
    fout = []
    for i, j in itertools.permutations(set(frequencies.keys()), 2):
        fout.append(i + "|" + j)
    print "finished frequencies"
    frequencies = fout'''

frequencies = set(frequencies)

def find_json():
    jsons = []
    for path, subdirs, files in os.walk("tosback2/crawl"):
        for name in files:
            if name.endswith(".json"):
                jsons.append(os.path.join(path, name))
    random.shuffle(jsons)
    return jsons


#print find_json()


def featuregen(paragraph):
    features = {}
    
    unigrams = nltk.word_tokenize(paragraph.lower())
    if BIGRAM_FEATURES:
        words = set()
        for i, word in enumerate( unigrams):
            if i == 0:
                continue
            #if w1 and w2 in frequencies:
            words.add(unigrams[i-1] + "|" + unigrams[i])
            
        return {"contains({})".format(k) : True for k in words}
        
    else:
        words = set(unigrams)
        
        for word in frequencies:
            features["containsword({})".format(word)] = word in words
    
    return features

def extract_examples(classname):
    jsondocs = find_json()
    examples = []
    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            for paragraph in marked['paragraphs']:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                if classname in classes:
                    examples.append(text)
    return examples

def prepare_examples(classname='l'):
    '''
    
    Parameters:
        classname - the name of the class that should be tagged as 
        positive examples
    Returns:
        
    '''
    jsondocs = find_json()
    
    examples = []
    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            
            for paragraph in marked["paragraphs"]:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                result = classname if classname in classes else "not " + classname
                
                examples.append((featuregen(text), result))
    
    return examples
    
def train_svm_network_tfidf(classname='l'):
    jsondocs = find_json()
    
    pos = []
    neg = []
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            
            for paragraph in marked["paragraphs"]:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                if classname in classes:
                    pos.append(text)
                else:
                    neg.append(text)
    
    idf = tos.get_idf()
    svm = tos.train_svm_tfidf(pos, neg, idf, classname)
    
    return svm


def construct_classifier_name(base, classification):
    return "pickles/{}_{}.pickle".format(base, classification)
    
def dump_classifier(classifier, classification):
    with open(construct_classifier_name(classifier.__class__.__name__, classification), 'wb') as out:
        pickle.dump(classifier, out)
    

def bayes_train(train_set):
    classifier = nltk.NaiveBayesClassifier.train(train_set)
    classifier.show_most_informative_features(20)
    
    return classifier


def decision_tree_train(train_set):
    classifier = nltk.classify.DecisionTreeClassifier.train(train_set)
    return classifier
    

def logistic_reg_train(train_set):
    classifier = nltk.classify.WekaClassifier.train(train_set, classifier="log_regression")
    return classifier


def maxent_train(train_set, algorithm="IIS"):
    '''Train using maximum entropy regression
    
    algorithm is one of the algorithms supported.
    
    GIS, IIS, MEGAM, TADM
    '''
    
    # from http://www.nltk.org/howto/classify.html
    return nltk.classify.MaxentClassifier.train(train_set, algorithm, trace=0, max_iter=100)


def svm_train(train_set):
    '''Train using support vector machines.'''
    # from http://www.nltk.org/howto/classify.html
    return nltk.classify.SklearnClassifier(SVC()).train(train_set)
    
class BoostedClassifier(nltk.classify.api.ClassifierI):
    
    def classify(self, featureset):
        b = self.bayes.classify(featureset)
        dt = self.dt.classify(featureset)
        svm = self.svm.classify(featureset)
        
        if b == dt:
            return b
        if b == svm:
            return svm
        if dt == svm:
            return svm
        
        return svm
    
    def labels(self):
        # todo make this better
        return self.bayes.labels()
    
    def train(self, train_set):
        self.train_set = train_set
        
        self.svm = svm_train(train_set)
        self.dt = decision_tree_train(train_set)
        self.bayes = bayes_train(train_set)


def boosted_train(train_set):
    bc = BoostedClassifier()
    bc.train(train_set)
    
    return bc
    
'''
def perceptron_train(train_set):
    pass
sklearn.linear_model.Perceptron

>>> 
>>> from sklearn.naive_bayes import BernoulliNB
>>> import SVC
>>> train_data = [({"a": 4, "b": 1, "c": 0}, "ham"),
...               ({"a": 5, "b": 2, "c": 1}, "ham"),
...               ({"a": 0, "b": 3, "c": 4}, "spam"),
...               ({"a": 5, "b": 1, "c": 1}, "ham"),
...               ({"a": 1, "b": 4, "c": 3}, "spam")]
>>> classif = SklearnClassifier(BernoulliNB()).train(train_data)
'''
    
def train(c, trainfuncs, getclassifiers=False):
    examples = prepare_examples(c)
    
    print "Positives: {}".format(sum(["not" not in e[1] for e in examples]))
    print "Negatives: {}".format(sum(["not" in e[1] for e in examples]))
    posset = [e for e in examples if "not" not in e[1]]
    negset = [e for e in examples if "not" in e[1]]
    
    print "Positives: {}".format(len(posset))
    print "Negatives: {}".format(len(negset))
    
    random.shuffle(posset)
    random.shuffle(negset)
    
    
    tspos = int(len(posset) * .8)
    tsneg = int(len(negset) * .8)
    
    #trainsize = int(len(examples) * .8)
    
    #train_set, test_set = examples[:trainsize], examples[trainsize:]
    train_set = posset[:tspos] + negset[:tsneg]
    random.shuffle(train_set)
    test_set = posset[tspos:] + negset[tsneg:]
    random.shuffle(test_set)
    
    
    # accuracy metrics for different training systems  
    trained_classifiers = [func(train_set) for func in trainfuncs]
    
    for tc in trained_classifiers:
        dump_classifier(tc, c)
        
    def accuracy(classifier, test_set):
        try:
            print "accuracy check of: " + classifier.__class__.__name__
            
            postest = [e for e in test_set if "not" not in e[1]]
            negtest = [e for e in test_set if "not" in e[1]]
            
            pos_correct = 0
            neg_correct = 0
            pos_incorrect = 0
            neg_incorrect = 0
            
            for item in test_set:
                observed = classifier.classify(item[0])
                if item in postest:
                    if observed == item[1]:
                        pos_correct += 1
                    else:
                        pos_incorrect += 1
                else:
                    if observed == item[1]:
                        neg_correct += 1
                    else:
                        neg_incorrect += 1
         
            precision = pos_correct / (pos_correct + neg_incorrect + 0.0000000000001)
            recall = pos_correct / (len(postest) + 0.0000000000001)
            fscore = 2 * (precision * recall) / (precision + recall)
            
            print "Correct Positive: {} Correct Negative: {} False Positive: {} False Negative: {}".format(pos_correct, neg_correct, pos_incorrect, neg_incorrect)
            print "fmeasure of {} = {}".format(classifier.__class__.__name__, fscore)

            return fscore #nltk.classify.accuracy(classifier, test_set) 
        except:
            return "err"
    
    if getclassifiers:
        return trained_classifiers
    
    return [accuracy(classifier, test_set) for classifier in trained_classifiers]

def get_classifiers():
    #training_funcs = [bayes_train] # svm performs pretty well
    
    classifiers = {}
    for k in tos.classes.keys():
        print "Training classifier: " + tos.classes[k]
        classifiers[k] = train_svm_network_tfidf(k) #train(k, training_funcs, True)[0]
        
    return classifiers

def old_classify():
    training_funcs_headers = ["naive_bayes","SVM",  "decision_tree", "boost", "Maxent"]
    training_funcs = [bayes_train]#, svm_train, decision_tree_train, boosted_train]#,maxent_train,]
    training_output = {}
    for k, v in tos.classes.items():
        print("=" * 80)
        print("{} - {}\n\n".format(k, v))
        training_output[k] = [str(v) for v in  train(k, training_funcs)]

    print("=" * 80)
    print("Results, F1 metric over {} {} on {}".format(len(frequencies), BIGRAM_FEATURES_STR,  datetime.datetime.today().isoformat()))
    print("=" * 80)
    
    print("\t{}".format("\t".join(training_funcs_headers)))
    
    for k, v in training_output.items():
        print("{}\t{}".format(k, "\t".join(v)))
    

def sklearn_classify(classname, text_clf=None, doublepos=True, supplement=0, reportwrong=False):

    if type(classname) == str:
        classname = [x for x in classname]
    elif type(classname) == dict:
        classname = classname.keys()
    
        
    if text_clf == None:
        text_clf = Pipeline([('vect', CountVectorizer(ngram_range=(1,2), binary=True)),
                     #('tfidf', TfidfTransformer()),
                     ('clf', MultinomialNB()),
        ])
    
    print "=" * 80
    jsondocs = find_json()
    
    examples = []
    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            marked = json.loads(fd.read())
            
            for paragraph in marked["paragraphs"]:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                result = 1 if any([x in classes for x in classname]) else 0
                
                examples.append((text, result))
    
    posset = [e for e in examples if e[1] == 1]
    negset = [e for e in examples if e[1] == 0]

    print "Positives: {}".format(len(posset))
    print "Negatives: {}".format(len(negset))

    
    random.shuffle(posset)
    random.shuffle(negset)
    
    tspos = int(len(posset) * .8)
    tsneg = int(len(negset) * .8)
    

    
    #trainsize = int(len(examples) * .8)
    
    #train_set, test_set = examples[:trainsize], examples[trainsize:]
        
    
    if doublepos:
        train_set = posset[:tspos] + posset[:tspos] + negset[:tsneg]
    else:
        train_set = posset[:tspos] + negset[:tsneg]
    
    if supplement > 0:
        train_set += finder2.auto_classify(supplement, classname).items()
    
    #for k, v in train_set:
    #    print k[:80], v
    
    #print "Positives: {}".format(len(posset))
    #print "Negatives: {}".format(len(negset))
    
    random.shuffle(train_set)
    if doublepos:
        test_set = posset[tspos:] + posset[tspos:] + negset[tsneg:]
    else:
        test_set = posset[tspos:] + negset[tsneg:]
    random.shuffle(test_set)
    
    
    
    random.shuffle(train_set)
    
    data, target = zip(*train_set)

    text_clf = text_clf.fit(data, target)
    
    datatest, targettest = zip(*test_set)
    predicted = text_clf.predict(datatest)
    
    
    fname = "_".join([tos.classes[k] for k in classname])
    fname = fname.replace("/", "-")
    fname = fname.replace(" ", "_")
    fname = fname.lower()
    if reportwrong:
        with open("wrong_classifications/" + fname + ".txt", 'w') as fp:
            for data, tar in test_set:
                p = text_clf.predict([data])
                
                if p != tar:
                    fp.write("-" * 80)
                    fp.write("\n")
                    fp.write("Got {} expected {}\n\n".format(p, tar))
                    fp.write(data)
                    fp.write("\n")


    print ", ".join([tos.classes[k] for k in classname])
    print "Correct mean: {}".format(np.mean(predicted == targettest))            
    #0.834...

    print(metrics.classification_report(targettest, predicted))
    
    
class NERTransformer(sklearn.base.TransformerMixin):
    
    words = []
    cache = {}
    
    def _process(self, p):
    
        if p in self.cache:
            return self.cache[p]
        
        sentences = tos.split_sentences(tos.tokenize(p))
        
        words = set()
        for sentence in sentences:
            try:
                words.update(tos.er_senna(sentence))
            except Exception as e:
                print e
                print sentence
        self.cache[p] = words
        return words
    
    
    def transform(self, X, **transform_params):
        res = []
        
        for x in X:
            ers = self._process(x)
            res.append([1 if w in ers else 0 for w in self.words])

        return np.array(res)

    def fit(self, X, y=None, **fit_params):
        for x in X:
            ers = self._process(x)
            
            for er in ers:
                if not er in self.words:
                    self.words.append(er)
        return self


class ARITransformer(sklearn.base.TransformerMixin):

    minari = float("Inf")
    maxari = float("-Inf")
    
    def _normalize(self, num):
        return float(num - self.minari)/(self.maxari - self.minari)
    
    def _ari(self, text):
            chars = len(text)
            words = text.count(" ")
            sentences = text.count(".") + text.count("!") + text.count("?")
            try:
                return 4.71 * float(chars) / float(words) + .5 * float(words) / float(sentences) - 21.43
            except ZeroDivisionError:
                return 0


    def transform(self, X, **transform_params):
        ''' A matrix of documents '''
        punc = set(".!?")
        
        npa = []
        
        for text in X:
            ari = self._ari(text)
            arin = self._normalize(ari)
            
            npa.append([arin])

        return np.array(npa)
        
    def fit(self, X, y=None, **fit_params):
        for x in X:
            lx = self._ari(x)
            if lx > self.maxari:
                self.maxari = lx
            if lx < self.minari:
                self.minari = lx
        return self

class LengthTransformer(sklearn.base.TransformerMixin):
    maxlength = float("-Inf")
    minlength = float("Inf")
    
    def _normalize(self, num):
        return float(num - self.minlength)/(self.maxlength - self.minlength)
    
    def transform(self, X, **transform_params):
        ret = [[self._normalize(len(x))] for x in X]
        return np.array(ret)

    def fit(self, X, y=None, **fit_params):
        for x in X:
            lx = len(x)
            if lx > self.maxlength:
                self.maxlength = lx
            if lx < self.minlength:
                self.minlength = lx
        return self


class PorterTokenizer(object):
    def __init__(self):
        self.wnl = PorterStemmer()
    def __call__(self, doc):
        return [self.wnl.stem_word(t) for t in word_tokenize(doc)]

if __name__ == "__main__":


    text_clf = Pipeline([('vect', CountVectorizer(ngram_range=(2,3), binary=True)),#, tokenizer=PorterTokenizer())),
                         #('tfidf', TfidfTransformer()),
                         ('clf', MultinomialNB(fit_prior=True)),
    ])
    
    text_clf2 = Pipeline([('ner', NERTransformer()),
                        ('bayes', MultinomialNB())])
    
    
    text_clf3 = Pipeline([('union', FeatureUnion([
                            ('vect', CountVectorizer(ngram_range=(1,2), binary=True)),
                            #('ner', NERTransformer()),
                            #('ari', ARITransformer()),
                            ('len', LengthTransformer())
                         ])),
                         ('bayes', Perceptron())])
    
    #sklearn_classify(['i','o'], text_clf=text_clf)#, doublepos=True)#, supplement=500)
    for k in tos.classes.keys():
        sklearn_classify(k, text_clf=text_clf, reportwrong=False)#, doublepos=True)#, supplement=500)'''
    '''    
    for k in ['i']:
        sklearn_classify(k, text_clf=text_clf, reportwrong=False)#, doublepos=True)#, supplement=500)
    '''
    '''for name, classset in tos.classsets.items():
        print name
        sklearn_classify(classset, text_clf = text_clf, reportwrong=False)
    '''
