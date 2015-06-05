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

def sklearn_multiclass_output(classname, preprocessor):
	print "# Classification over {}".format(classname)
	
	examples = []
	examplesdocs = []
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
		
				doc.append((text, int(classname in classes)))
				
		if len(doc) != 0:
			examples.append(doc)
			examplesdocs.append(jdoc)

	# this should be constant 
	random.shuffle(examples)
	
	'''print len(examples)
	'''
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
	
	output_train = ""
	output_test = ""
	for docno, example in enumerate(examples):
		x, y = zip(*example)
		
		print ""
		features = preprocessor.transform(x)
		for i, x in enumerate(features):
		
			val = y[i]

			features = []
			
			for pos, feature in enumerate(x):
				if feature != 0:
					features.append("{}:{}".format(pos + 1,feature))
			
			#PRON qid:1.1 1:1 2:1 3:1 #see
			val = "NOT" + classname if val == 0 else classname
			val = val.upper()
			output = "{} qid:{}.{} {} # nothing\n".format(val, docno + 1, i + 1, " ".join(features))
			
			if docno > len(examples) * .8:
				output_test += output
			else:
				output_train += output
			
			
			
	return output_train, output_test

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

class SentenceTransformer(sklearn.base.TransformerMixin):
	def __init__(self, subj=True, verb=True, argument=True, neg=True):
		self.subj = subj
		self.verb = verb
		self.argument = argument
		self.neg = neg
	
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
					subj, verb, arg = term.split(":",2)
					out.append(subj + "_" + verb + "_" + arg)
					out.append(subj + "_" + verb)
					out.append(subj + "_" + arg)
					out.append(verb + "_" + arg)
		
					if verb.startswith("not_"):
						verb = verb[4:]
						out.append(subj + "_" + verb + "_" + arg)
						out.append(subj + "_" + verb)
						out.append(subj + "_" + arg)
						out.append(verb + "_" + arg)
					#if len(out) != 8:
					#	out += [''] * (8 - len(out))
			output.append(out)
		
		return output

	def fit(self, X, y=None, **fit_params):
		return self

	
def get_sentence_transformer():
	return Pipeline([('st', SentenceTransformer()),
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

	# fixing class weight tests
	text_clf6 = Pipeline([  #('decode', UnicodeTransform()),
				('union', FeatureUnion([
					#('mf', scikit_pipeline_steps.MostFrequentEntity()),
					('bow2', scikit_pipeline_steps.BOWTransform()),
					('dict', scikit_pipeline_steps.DictionaryTransformer()),
					('sentecne_transform',get_sentence_transformer()),
					#('person', scikit_pipeline_steps.EntityTransform()),
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
				])),
			])
			
	for k in tos.classes.keys():
		print k
		train, test = sklearn_multiclass_output(k, text_clf6)
		
		with open("{}_train.dat".format(k), 'w') as fd:
			fd.write(train)
		
		with open("{}_test.dat".format(k), 'w') as fd:
			fd.write(test)
