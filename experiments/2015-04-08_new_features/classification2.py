#!/usr/bin/env python2
'''
Do classification on the documents we have already converted to JSON
'''
from nltk.tokenize import word_tokenize
from sklearn import linear_model
from sklearn import metrics
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.linear_model import Perceptron
from sklearn.pipeline import FeatureUnion
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import Normalizer
from sklearn.svm import SVC, LinearSVC
import sklearn.preprocessing as preprocessing
import json
import nltk
import numpy
import os
import random
import re
import scikit_pipeline_steps
import sklearn
import tos


random.seed(8675309) # Jenny's Constant



def find_json():
    jsons = []
    for path, subdirs, files in os.walk("../../tosback2/crawl"):
        for name in files:
            if name.endswith(".json"):
                jsons.append(os.path.join(path, name))
    random.shuffle(jsons)
    return jsons


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
        
        self.svm = nltk.classify.SklearnClassifier(SVC()).train(train_set)
        self.dt = nltk.classify.DecisionTreeClassifier.train(train_set)
        self.bayes = nltk.NaiveBayesClassifier.train(train_set)





def sklearn_classify(classname, text_clf, reportwrong=False):

    if type(classname) == str:
        classname = [x for x in classname]
    elif type(classname) == dict:
        classname = classname.keys()
    
    
    print "=" * 80
    jsondocs = find_json()
    
    examples = []
    
    for jdoc in jsondocs:
        with open(jdoc) as fd:
            try:
                marked = json.loads(fd.read())
            except Exception as e:
                print e
                print jdoc
                exit()
            for paragraph in marked["paragraphs"]:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                result = 1 if any([x in classes for x in classname]) else 0
                
                examples.append((text, result))
    
    posset = [e for e in examples if e[1] == 1]
    negset = [e for e in examples if e[1] == 0]

    print "Positives: {}".format(len(posset))
    print "Negatives: {}".format(len(negset))
    
    tspos = int(len(posset) * .8)
    tsneg = int(len(negset) * .8)
    
    

    test_set = posset[tspos:] + negset[tsneg:]
    train_set = posset[:tspos] + negset[:tsneg]
    
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
    print "Correct mean: {}".format(numpy.mean(predicted == targettest))            
    #0.834...

    print(metrics.classification_report(targettest, predicted))



def sklearn_classify_document(classname, preprocessor, text_clf, reportwrong=False, multiclass=False):

    if type(classname) == str:
        classname = [x for x in classname]
    elif type(classname) == dict:
        classname = classname.keys()
    
    
    print "=" * 80
    print ", ".join([tos.classes[k] for k in classname])
    jsondocs = find_json()
    
    examples = []
    
    for jdoc in jsondocs:
        doc = []
        with open(jdoc) as fd:
            try:
                marked = json.loads(fd.read())
            except Exception as e:
                print e
                print jdoc
                break
                
            for paragraph in marked["paragraphs"]:
                text = paragraph["paragraph"]
                classes = paragraph["classes"]
                
                if multiclass:
                    result = [x in classes for x in classname]
                    result = map(int, result)
                else:
                    result = 1 if any([x in classes for x in classname]) else 0
                
                doc.append((text, result))
        if len(doc) == 0:
            continue
        examples.append(doc)
    
    random.shuffle(examples)

    
    for example in examples:
        x, y = zip(*example)
        preprocessor = preprocessor.fit(x, y)
    
    rawpos = []
    processed_pos = []
    
    rawneg = []
    processed_neg = []
    for example in examples:
        x, y = zip(*example)
        features = text_clf6.transform(x)
        
        for i, x in enumerate(features):
            if any(y):
                processed_pos.append((x, y[i]))
                rawpos.append(example[i][0])
            else:
                processed_neg.append((x, y[i]))
                rawneg.append(example[i][0])
    
    posdiv = int(len(rawpos) * .8)
    negdiv = int(len(rawneg) * .8)
    
    
    test_set = processed_pos[posdiv:] + processed_neg[negdiv:]
    test_examples = rawpos[posdiv:] + rawneg[negdiv:]
    train_set = processed_pos[:posdiv] + processed_neg[:negdiv]
    
    x, y = zip(*train_set)
    text_clf = text_clf.fit(x, y)
    

    x, y = zip(*test_set)
    predicted = text_clf.predict(x)
    
    fname = "_".join([tos.classes[k] for k in classname])
    fname = fname.replace("/", "-")
    fname = fname.replace(" ", "_")
    fname = fname.lower()
    if reportwrong:
        with open("wrong_classifications/" + fname + ".txt", 'w') as fp:
            for i, b in enumerate(test_set):
                data, tar = b
                p = text_clf.predict([data])
                
                if p != tar:
                    fp.write("-" * 80)
                    fp.write("\n")
                    fp.write("Got {} expected {}\n\n".format(p, tar))
                    fp.write(test_examples[i])
                    fp.write("\n")
    

    print "Correct mean: {}".format(numpy.mean(predicted == y))

    print(metrics.classification_report(y, predicted))
    
    
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

        return numpy.array(res)

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

        return numpy.array(npa)
        
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
        return numpy.array(ret)

    def fit(self, X, y=None, **fit_params):
        for x in X:
            lx = len(x)
            if lx > self.maxlength:
                self.maxlength = lx
            if lx < self.minlength:
                self.minlength = lx
        return self


class TermTransformer(sklearn.base.TransformerMixin):
    def __init__(self, termlist):
        self.termlist = termlist
        
    def transform(self, X, **transform_params):
        output = []
        for text in X:
            #words = nltk.tokenize.word_tokenize(text)
            words = text.lower() #[w.lower() for w in words]
            
            # true/false if the given term is in the wordlist
            out = [t in words for t in self.termlist]
            
            # to numerical features
            out = map(int, out)
            
            output.append(out)

        return numpy.array(output)

    def fit(self, X, y=None, **fit_params):
        return self

class PositionTransform(sklearn.base.TransformerMixin):
    ''' position of the paragraph in the document'''

    def transform(self, X, **transform_params):
        output = []
        
        total_length = len(X)
        for i in range(total_length):
            output.append([float(i) / total_length])
        
        return numpy.array(output)

    def fit(self, X, y=None, **fit_params):
        return self

class HistoryTransform(sklearn.base.TransformerMixin):
    '''Searn for the last decisions'''
    
    def __init__(self, history=2):
        self.history = history
    
    def transform(self, X, **transform_params):
        xlen = len(X[0])
        
        out = []
        for i in range(len(X)):
            tmp = []
            for j in range(self.history):
                if i - j < 0:
                    tmp += [0] * xlen
                else:
                    tmp += list(X[i - j])
            out.append(tmp)
        return numpy.array(out)

    def fit(self, X, y=None, **fit_params):
        return self

pii_terms = [
    'mobile device type', 
    'location', 
    'personal information', 
    'personally identifiable information', 
    'name','address', 'email', 'phone number', 'zip code', 'credit card number',
    'payment information', 'drivers license number', 'social security number',
    'age', 'date of birth', 'credit information', 'demographic information',
    'lifestyle information', 'personal interests', 'product preferences',
    'survey questions', 'anonymous usage data', 'referring pages', 'platform type',
    'clickstream', 'ip address', 'cookie', 'log data']

tech_terms = ['location', 'network adapter', 'wi-fi', 'wifi', 'gps signals', 'cell tower', 'cookies', 'tracking', 'javascript', 'web beacons', 'lso', 'flash cookies', 'clear gifs', 'ip ', 'mac ','pixel tag', 'ssl']

business_terms = ['merger', 'acquisition', 'marketing', 'customer service', 'fruad prevention', 'public safety', 'legal purposes', 'sales']
message_terms = ['newsletters','announcements']
service_terms = ['advertising', 'social media plugins', 'widgets', 'behavioral advertising']

transfer_terms = ['asset', 'acquisition', 'merger', 'transferred', 'acquired', 'becoming', 'bankruptcy', 'periodically', 'transfers']
rights_terms = ['right', 'responsability', 'may', 'can', 'you can', 'you may', 'must']

if __name__ == "__main__":

    # fixing class weight tests
    text_clf6 = Pipeline([( 'union', FeatureUnion([
                            #('vect', CountVectorizer(ngram_range=(1,2), binary=True)),
                            ('dict', scikit_pipeline_steps.DictionaryTransformer()),
                            ('person', scikit_pipeline_steps.EntityTransform()),
                            ('jurtransform', TermTransformer(['giving','exclusively','courts','america','jurisdiction'])),
                            ('copytransform', TermTransformer(['derivative','worldwide','sublicense','derive','reproduce','works','non-exclusive', 'transferable','license', 'copyright', 'intellectual'])),
                            #('deftransform', TermTransformer(["meaning", "means", "defined","definition", "collectively", "aka", "a.k.a.", "e.g.", "eg.", "eg ", "(ie ", "(i.e.", "also known as", "such as"])),
                            ('persontransform', TermTransformer(["zip","age","name","date of birth", "credit card number", "email address", "e-mail address", "personal information", "personally identifiable"])),
                            ('hamless', TermTransformer(['implied', 'warranties','risk','guarantee','failure','neither','reliance','accuracy','liable'])),
                            ('lawtransform', TermTransformer(['enforcement','suspected','investigate','preserve','faith','taxes','judicial','pending','governmental'])),
                            ('collection', TermTransformer(['combine','operating','pixel','metrics','gifs'])),
                            
                            ('position_transform', TermTransformer(pii_terms)),
                            ('tech_transform', TermTransformer(tech_terms)),
                            ('business_transform', TermTransformer(business_terms)),
                            ('message_terms', TermTransformer(message_terms)),
                            ('service_terms', TermTransformer(service_terms)),
                            ('transfer_terms', TermTransformer(transfer_terms)),
                            ('rrterms', TermTransformer('rights_terms')),
                            #('svlb', Pipeline([
                            #    ('sv', scikit_pipeline_steps.SubjVerb()),
                            #    ('vect', CountVectorizer(binary=True, tokenizer=lambda x: x.split()))
                            #]))
                           

                            #('ner', NERTransformer()),
                            #('ari', ARITransformer()),
                            #('len', LengthTransformer())
                         ])),
                         #('hist',HistoryTransform(2))
                         ])
    text_clf6_classifier = Pipeline([('bayes', LinearSVC(class_weight={0:.5, 1:2}))]) # 
    
    for k in tos.classes.keys():
        #sklearn_classify(k, text_clf=text_clf6, reportwrong=True)
        sklearn_classify_document(k, preprocessor=text_clf6, text_clf=text_clf6_classifier, reportwrong=True)
        try:
            pass
        except Exception as e:
            print e
            print "Continuing..."
