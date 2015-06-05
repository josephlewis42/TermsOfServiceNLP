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
from unidecode import unidecode
from sklearn.multiclass import OneVsRestClassifier
from sklearn.preprocessing import MultiLabelBinarizer
import sklearn.metrics
import sentences_broken
import other_sentences
import lru

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

def sklearn_multiclass_document(classnames, preprocessor, text_clf, reportwrong=False, ignorenone=False):

    print "=" * 80
    print "Multiclass Classification over {}".format(", ".join([tos.classes[k] for k in classnames]))
    labels = [tos.classes[k] for k in classnames]
    if not ignorenone:
     labels += ["None"]

    examples = []

    for jdoc in find_json():
     doc = []
     with open(jdoc) as fd:
         try:
          marked = json.loads(fd.read())
         except Exception as e:
          print "Error: {}, doc {}".format(e, jdoc)
          break


         for paragraph in marked["paragraphs"]:
          text = paragraph["paragraph"]
          classes = paragraph["classes"]


          result = [p for p,x in enumerate(classnames) if x in classes]
          if result == []:
              if ignorenone:
               continue
              else:
               result = [10]
          #result = map(int, result)


          doc.append((text, result))

     if len(doc) != 0:
         examples.append(doc)

    # this should be constant
    random.shuffle(examples)

    # fit to everything so we don't come across an edge case later
    for example in examples:
     x, y = zip(*example)
     preprocessor = preprocessor.fit(x, numpy.array(y))

    # raw and processed positives
    rawpos = []
    processed_pos = []

    # raw and processed negatives
    rawneg = []
    processed_neg = []

    for example in examples:
     x, y = zip(*example)

     isPosDoc = False
     for tmp in y:
         if any(tmp):
          isPosDoc = True
          break

     features = text_clf6.transform(x)

     for i, x in enumerate(features):
         if isPosDoc:
          processed_pos.append((x, y[i]))
          rawpos.append(example[i][0])
         else:
          processed_neg.append((x, y[i]))
          rawneg.append(example[i][0])

    posdiv = int(len(rawpos) * .8)
    negdiv = int(len(rawneg) * .8)

    test_set      = processed_pos[posdiv:] + processed_neg[negdiv:]
    test_examples = rawpos[posdiv:]     + rawneg[negdiv:]
    train_set     = processed_pos[:posdiv] + processed_neg[:negdiv]

    # train multilabel
    mlb = MultiLabelBinarizer()
    _, y1 = zip(*train_set)
    _, y2 = zip(*test_set)
    mlb.fit(y1 + y2)


    # training
    x, y = zip(*train_set)
    y = mlb.transform(y)
    text_clf = text_clf.fit(x, y)

    # prediction
    x, y = zip(*test_set)

    y = mlb.transform(y)
    predicted = text_clf.predict(x)

    fname = "_".join([tos.classes[k] for k in classnames])
    fname = fname.replace("/", "-")
    fname = fname.replace(" ", "_")
    fname = fname.lower()

    print "Correct mean: {}".format(numpy.mean(predicted == y))
    print(metrics.classification_report(y, predicted, target_names=labels))



    for i in range(len(y)):
        pred = predicted[i]
        real = y[i]

        confused = []
        for j, lbl in enumerate(labels):
            if pred[j] != real[j]:
                if pred[j] == 1:
                    confused.append("has: " + labels[j])
                else:
                    confused.append("should_have: " + labels[j])
        if confused != []:
            print "-" * 80
            print test_examples[i]
            print "\n".join([label for i, label in labels if pred[i]])
            print "====="
            print " ".join(confused)


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



we_cannon = set(['we','us','our'])
you_cannon = set(['you','your'])
gov_cannon = set(['government', "enforcement", "agency"])
third_cannon = set(['party','third-party'])

from nltk.corpus import wordnet
class SentenceTransformer(sklearn.base.TransformerMixin):
    def __init__(self, subj=True, verb=True, argument=True, neg=True, cannonical=True, syns=True, processneg=False):
        self.subj = subj
        self.verb = verb
        self.argument = argument
        self.neg = neg
        self.cannonical = cannonical
        self.syns = syns
        self.processneg = processneg

    @lru.lru_cache(maxsize=10000)
    def synonyms(self, word):
        return [s.lemmas()[0].name() for s in wordnet.synsets(word)]

    def get_senses(self, subj, verb, arg):
        out = []
        out.append(subj + "_" + verb + "_" + arg)
        out.append(subj + "_" + verb)
        out.append(subj + "_" + arg)
        out.append(verb + "_" + arg)
        return out

    def to_cannon(self, subj):
        if self.cannonical:
            if subj in we_cannon:
                subj = "we"
            elif subj in you_cannon:
                subj = "you"
            elif subj in gov_cannon:
                subj = "gov"
            elif subj in third_cannon:
                subj = "third"
        return subj

    def transform(self, X, **transform_params):
        output = []
        for text in X:
            out = []
            sentences = nltk.tokenize.sent_tokenize(text)

            for sent in sentences:
                if sent not in sentences_broken.pairings and sent not in other_sentences.other_pairings:
                    continue

                else:
                    #print "found sentence"
                    try:
                        term = sentences_broken.pairings[sent]
                    except:
                        term = other_sentences.other_pairings[sent]

                    term = term.lower()

                    # do synonym adding
                    if self.syns:
                        subj, verb, arg = term.split(":",2)
                        subj = self.to_cannon(subj)
                        neg = False
                        if verb.startswith("not_") and self.processneg:
                            verb = verb[4:]
                            neg = True

                        for synonym in self.synonyms(verb):
                            out.extend(self.get_senses(subj, synonym, arg))
                            if neg:
                                out.extend(self.get_senses(subj, "not_" + synonym, arg))

                    subj, verb, arg = term.split(":",2)
                    subj = self.to_cannon(subj)
                    out.extend(self.get_senses(subj, verb, arg))

                    if verb.startswith("not_") and self.processneg:
                        verb = verb[4:]
                        out.extend(self.get_senses(subj, verb, arg))

            output.append(out)

        return output

    def fit(self, X, y=None, **fit_params):
        return self


def get_sentence_transformer(*args, **kwargs):
    return Pipeline([('st', SentenceTransformer(*args, **kwargs)),
                     ('binariz', scikit_pipeline_steps.BinaryFeatureTransform())])

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
    'clickstream', 'ip address', 'cookie', 'log data', 'peraonally identifiable']

tech_terms = ['location', 'network adapter', 'wi-fi', 'wifi', 'gps signals', 'cell tower', 'cookies', 'tracking', 'javascript', 'web beacons', 'lso', 'flash cookies', 'clear gifs', 'ip ', 'mac ','pixel tag', 'ssl']

business_terms = ['merger', 'acquisition', 'marketing', 'customer service', 'fruad prevention', 'public safety', 'legal purposes', 'sales']
message_terms = ['newsletters','announcements']
service_terms = ['advertising', 'social media plugins', 'widgets', 'behavioral advertising']

transfer_terms = ['asset', 'acquisition', 'merger', 'transferred', 'acquired', 'becoming', 'bankruptcy', 'periodically', 'transfers']
rights_terms = ['right', 'responsability', 'may', 'can', 'you can', 'you may', 'must']

if __name__ == "__main__":

    import datetime
    print datetime.datetime.today().isoformat()

    exp = raw_input("Experiment Name? ")

    print exp

    # fixing class weight tests
    text_clf6 = Pipeline([  #('decode', UnicodeTransform()),
                ('union', FeatureUnion([
                    #('mf', scikit_pipeline_steps.MostFrequentEntity()),
                    ('bow2', scikit_pipeline_steps.BOWTransform()),
                    ('dict', scikit_pipeline_steps.DictionaryTransformer()),
                    ('sentecne_transform',get_sentence_transformer(syns=False, processneg=False)),
                    #('person', scikit_pipeline_steps.EntityTransform()),
                    ('transformunion', FeatureUnion([
                        ('jurtransform', TermTransformer(['giving','exclusively','courts','america','jurisdiction'])),
                        ('copytransform', TermTransformer(['derivative','worldwide','sublicense','derive','reproduce','works','non-exclusive', 'transferable','license', 'copyright', 'intellectual'])),
                        ('hamless', TermTransformer(['implied', 'warranties','risk','guarantee','failure','neither','reliance','accuracy','liable'])),
                        ('lawtransform', TermTransformer(['enforcement','suspected','investigate','preserve','faith','taxes','judicial','pending','governmental'])),
                        ('collection', TermTransformer(['combine','operating','pixel','metrics','gifs'])),
                        ('pii', TermTransformer(pii_terms)),
                        ('tech_transform', TermTransformer(tech_terms)),
                        ('business_transform', TermTransformer(business_terms)),
                        ('message_terms', TermTransformer(message_terms)),
                        ('service_terms', TermTransformer(service_terms)),
                        ('transfer_terms', TermTransformer(transfer_terms)),
                        ('rrterms', TermTransformer(rights_terms)),
                    ], n_jobs=4)),
                ])),
            ])
    def print_featureunion(pipeline, depth=0):
        for item in pipeline.transformer_list:
            print "\t" * depth + "U " + item[0]
            if isinstance(item[1], FeatureUnion):
                print_featureunion(item[1], depth + 1)

            if isinstance(item[1], Pipeline):
                print_steps(item[1], depth + 1)

    def print_steps(pipeline, depth=0):
        for item in pipeline.steps:
            print "\t" * depth + "| " + item[0]
            if isinstance(item[1], FeatureUnion):
                print_featureunion(item[1], depth + 1)

            if isinstance(item[1], Pipeline):
                print_steps(item[1], depth + 1)

    print_steps(text_clf6)

    text_clf6_classifier = OneVsRestClassifier(SVC(kernel="linear", class_weight='auto'), n_jobs=-1)

    print text_clf6_classifier



    sklearn_multiclass_document(list(tos.classes.keys()), preprocessor=text_clf6, text_clf=text_clf6_classifier, reportwrong=True)
    exit()
    for k in tos.classes.keys():
        sklearn_multiclass_document(k, preprocessor=text_clf6, text_clf=text_clf6_classifier, reportwrong=True, ignorenone=True)
