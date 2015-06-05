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
    test_examples = rawpos[posdiv:]        + rawneg[negdiv:]
    train_set     = processed_pos[:posdiv] + processed_neg[:negdiv]
    
    # train multilabel
    mlb = MultiLabelBinarizer()
    _, y = zip(*train_set)
    mlb.fit(y)
    _, y = zip(*test_set)
    mlb.fit(y)
    
    
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
    '''
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
    '''

    print "Correct mean: {}".format(numpy.mean(predicted == y))
    print(metrics.classification_report(y, predicted, target_names=labels))
    
    #print(sklearn.metrics.confusion_matrix(y, predicted, labels=labels))



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

    # fixing class weight tests
    text_clf6 = Pipeline([  #('decode', UnicodeTransform()),
                            ('union', FeatureUnion([
                                #('binarizer', scikit_pipeline_steps.get_chunks_bin()),
                                #('bow', scikit_pipeline_steps.get_entitybow_bin()),
                                ('bow', scikit_pipeline_steps.get_directional_entitybow_bin()),
                                ('bow1', scikit_pipeline_steps.get_entitybow_bin()),
                                #('bow2', scikit_pipeline_steps.BOWTransform()),
                                ('dict', scikit_pipeline_steps.DictionaryTransformer()),
                                ('person', scikit_pipeline_steps.EntityTransform()),
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
                                ('rrterms', TermTransformer('rights_terms')),
                         ])),
                     ])
                     
                     
    text_clf6_classifier = OneVsRestClassifier(LinearSVC())
    
    sklearn_multiclass_document(list(tos.classes.keys()), preprocessor=text_clf6, text_clf=text_clf6_classifier, reportwrong=True)
    exit()
    for k in tos.classes.keys():
        sklearn_multiclass_document(k, preprocessor=text_clf6, text_clf=text_clf6_classifier, reportwrong=True, ignorenone=True)
