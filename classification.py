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
from sklearn.svm import SVC

frequencies = collections.defaultdict(int)

try:
    with open("frequencies.json") as freqfile:
        frequencies = json.loads(freqfile.read())
        print("Loaded frequencies")
except IOError:
    # build a corpus using the legal documents, remember we should strip features.
    with open("corpus.txt") as rawfile:
        raw = re.sub('<[^<]+?>'," ", rawfile.read())
        
        raw = "".join([c for c in raw.lower() if ord(c) >= 0x20 and ord(c) <= 0x7e])
        
        tokens = nltk.word_tokenize(raw)
        
        #print tokens
        print len(tokens)
        
        for word in tokens:
            frequencies[word] += 1
        
        with open("frequencies.json", "w") as freqfile:
            freqfile.write(json.dumps(frequencies))
        
        print("Dumped frequencies")
        
def remove_values_lt(dictionary, count=1):
    return {k:v for k, v in dictionary.items() if v > count}

#print len(frequencies)
#print len(remove_values_lt(frequencies, 30))

frequencies = remove_values_lt(frequencies, 30)
        
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
    words = set(nltk.word_tokenize(paragraph.lower()))
    
    for word in frequencies:
        features["containsword({})".format(word)] = word in words
    
    return features


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

'''
import pickle
f = open('my_classifier.pickle')
classifier = pickle.load(f)
f.close()
'''

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
    
def train(c, trainfuncs):
    examples = prepare_examples(c)
    random.shuffle(examples)
    
    print "Positives: {}".format(sum(["not" not in e[1] for e in examples]))
    print "Negatives: {}".format(sum(["not" in e[1] for e in examples]))
    
    trainsize = int(len(examples) * .8)
    
    train_set, test_set = examples[:trainsize], examples[trainsize:]
    
    
    # accuracy metrics for different training systems  
    trained_classifiers = [func(train_set) for func in trainfuncs]
    
    for tc in trained_classifiers:
        dump_classifier(tc, c)
        
    def accuracy(classifier, test_set):
        try:
            return nltk.classify.accuracy(classifier, test_set) 
        except:
            return "err"
    
    return [accuracy(classifier, test_set) for classifier in trained_classifiers]


if __name__ == "__main__":
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
                #'x': 'Data Export'
    
    training_funcs_headers = ["SVM", "Maxent", "naive_bayes", "decision_tree"]
    training_funcs = [maxent_train]#svm_train]#, maxent_train]#, bayes_train, decision_tree_train]
    training_output = {}
    for k, v in classes.items():
        print("=" * 80)
        print("{} - {}\n\n".format(k, v))
        training_output[k] = [str(v) for v in  train(k, training_funcs)]

    print("=" * 80)
    print("Results")
    print("=" * 80)
    
    print("\t{}".format("\t".join(training_funcs_headers)))
    
    for k, v in training_output.items():
        print("{}\t{}".format(k, "\t".join(v)))
    


