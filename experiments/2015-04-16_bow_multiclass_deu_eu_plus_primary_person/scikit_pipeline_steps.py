#!/usr/bin/env python2

import nltk.tokenize
import discriminator
import sklearn
import numpy as np
import sklearn.pipeline
import lru








@lru.lru_cache(50000)
def word_tokenize_lru(x):
    return nltk.tokenize.word_tokenize(x)

class MostFrequentEntity(sklearn.base.TransformerMixin):
    def fit(self, X, y=None, **fit_params):
        return self
    
    def transform(self, X, **transform_params):
        output = []
        for text in X:
            b = []
            words = set(word_tokenize_lru(text))
            words = [w.lower() for w in words]

            we = 0
            you = 0
            gov = 0
            third = 0
            for w in words:
                if w in set(['we','us','our']):
                    we += 1
                elif w in set(['you','your']):
                    you += 1
                elif w in set(['government', 'enforcement']):
                    gov += 1
                elif w in set(['party','third-party']):
                    third += 1
                    
            m = max([we,you,gov,third])
            b = [we == m, you == m, gov == m, third == m]
            b = map(int, b)
            
            output.append(b)

        return np.array(output)

class EntityTransform(sklearn.base.TransformerMixin):
    def fit(self, X, y=None, **fit_params):
        return self
    
    def transform(self, X, **transform_params):
        output = []
        for text in X:
            b = []
            words = set(word_tokenize_lru(text))

            for w in words:
                if w.lower() in ['we','us','our']:
                    b.append(1)
                    break
            else:
                b.append(0)
            
            for w in words:
                if w.lower() in ['you','your']:
                    b.append(1)
                    break
            else:
                b.append(0)
            
            for w in words:
                if w.lower() in ['government']:
                    b.append(1)
                    break
            else:
                b.append(0)
            for w in words:
                if w.lower() in ['party','third-party']:
                    b.append(1)
                    break
            else:
                b.append(0)
            output.append(b)

        return np.array(output)


class ParagraphToSentenceTransform(sklearn.base.TransformerMixin):
    def transform(self, X, **transform_params):
        return np.array([nltk.tokenize.sent_tokenize(x) for x in X])

    def fit(self, X, y=None, **fit_params):
        return self

class TextToTokenTransform(sklearn.base.TransformerMixin):
    '''Converts a list of sentences into a list of tokens'''
    def transform(self, X, **transform_params):
        return np.array([word_tokenize_lru(x) for x in X])

    def fit(self, X, y=None, **fit_params):
        return self
        

class BOWTransform(sklearn.base.TransformerMixin):
    '''Converts a list of strings to a list of tokens'''
    knownclasses = set([])
    words = []
    
    def process(self, X):
        return np.array([word_tokenize_lru(x) for x in X])
    
    def transform(self, X, **transform_params):
        a = self.process(X)
        
        output = []
        for s in a:
            s = set(s)
            output.append([1 if x in s else 0 for x in self.words])
            
        
        return np.array(output)

    def fit(self, X, y=None, **fit_params):
        #print a

        words = []
        for arr in self.process(X):
            words += arr
        
        for word in words:
            if word in self.knownclasses:
                continue
            else:
                self.knownclasses.add(word)
                self.words.append(word)            
        return self
        

class EntityBOWTransform(sklearn.base.TransformerMixin):
    '''Converts a list of strings to a list of tokens with the entities mentioned in their sentences'''
    
    def __init__(self, entities=['we','us','our', 'you','your', 'government', 'party','third-party']):
        self.entities = entities
        
    def transform(self, X, **transform_params):
        output = []
        
        for text in X:
            textoutput = []
            sentences, tokens, tags = sent_token_tag(text)
            
            for tokenlist in tokens:
                tokenset = set(tokenlist)
                for e in self.entities:
                    if e in tokenset:
                        textoutput += ["{}_{}".format(e,t) for t in tokenlist]
            output.append(textoutput)
        
        return np.array(output)

    def fit(self, X, y=None, **fit_params):
        return self


def get_entitybow_bin():
    ''' Gets a chunking and binarizer pipeline'''
    return sklearn.pipeline.Pipeline([   
                ('entities', EntityBOWTransform()),
                ('binarizr', BinaryFeatureTransform())])


class DirectionalEntityBOWTransform(sklearn.base.TransformerMixin):
    '''Converts a list of strings to a list of tokens with the entities mentioned in their sentences'''
    
    def __init__(self, entities=['we','us','our', 'you','your', 'government', 'party','third-party']):
        self.entities = entities
        
    def transform(self, X, **transform_params):
        output = []
        
        for text in X:
            textoutput = []
            sentences, tokens, tags = sent_token_tag(text)
            
            for tokenlist in tokens:
                tokenlist = [x.lower() for x in tokenlist]
                tokenset = set(tokenlist)
                for e in self.entities:
                    if e in tokenset:
                        before = "before"
                        for t in tokenlist:
                            if t == e:
                                before = "after"
                                continue
                            textoutput.append("{}_{}_{}".format(e,t, before))
            output.append(textoutput)
        
        return np.array(output)

    def fit(self, X, y=None, **fit_params):
        return self


def get_directional_entitybow_bin():
    ''' Gets a chunking and binarizer pipeline'''
    return sklearn.pipeline.Pipeline([   
                ('entities', DirectionalEntityBOWTransform()),
                ('binarizr', BinaryFeatureTransform())])

class SubjVerb(sklearn.base.TransformerMixin):
    '''Converts to subject/verb paris'''
    
    potentials = None
        
    def transform(self, X, **transform_params):
        out = []
        
        for x in X:
            xvbprp = []
            for sentence in nltk.tokenize.sent_tokenize(x):
                words = nltk.tokenize.word_tokenize(sentence)
                tagged = nltk.pos_tag(words)
                
                preps = [y[0] for y in tagged if y[1] == "PRP"]
                verbs = [y[0] for y in tagged if y[1] == "VB"]
                
                for p in preps:
                    for v in verbs:
                        xvbprp.append("{}_{}".format(p, v))
            if len(xvbprp) == 0:
                xvbprp.append("nothing_here")
            out.append(" ".join(xvbprp))
            
        return np.array(out)                
    
    def fit(self, X, y=None, **fit_params):
        return self
        




class DictionaryTransformer(sklearn.base.TransformerMixin):
    '''Converts a list of sentences into a list of tokens'''
    def __init__(self):
        self.descrim = discriminator.get_discriminator()
    def transform(self, X, **transform_params):
        output = []
        for text in X:
            sentences = nltk.tokenize.sent_tokenize(text)
            
            out = any(self.descrim.predict(sentences))
            output.append([out])
        
        return np.array(output)

    def fit(self, X, y=None, **fit_params):
        return self


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


class NERTransform(sklearn.base.TransformerMixin):
    
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


class ARITransform(sklearn.base.TransformerMixin):

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

class LengthTransform(sklearn.base.TransformerMixin):
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
        

class PeekTransform(sklearn.base.TransformerMixin):
    '''Print every nth item in a pipeline'''
    
    def __init__(self, every=1):
        self.every = every
    
    def transform(self, X, **transform_params):
        for i in xrange(0, len(X), self.every):
            print ""
            print "Peeking element {} of {}".format(i, len(X))
            print "Type: {}".format(type(X[i]))
            print "Value: {}".format(X[i])
            
        return numpy.array([[0]] * len(X))

    def fit(self, X, y=None, **fit_params):
        return self


class MatrixPeekTransform(sklearn.base.TransformerMixin):
    '''Print every nth item in a pipeline'''
    
    def __init__(self, every=1):
        self.every = every
    
    def transform(self, X, **transform_params):
        xlen = X.shape[0]
        for i in xrange(0, xlen, self.every):
            print ""
            print "Peeking element {} of {}".format(i, xlen)
            print "Type: {}".format(type(X[i]))
            print "Value: {}".format(X[i])
            
        return numpy.array([[0]] * xlen)

    def fit(self, X, y=None, **fit_params):
        return self

class UnicodeTransform(sklearn.base.TransformerMixin):
    ''' Changes a string to ascii from unicode
    '''
    def __init__(self, every=1):
        self.every = every
    
    def transform(self, X, **transform_params):
        return numpy.array([unidecode(x) for x in X])

    def fit(self, X, y=None, **fit_params):
        return self




def bucketize_lt(buckets, value):
    tmp = [0] * len(buckets)
    for j, bucketend in enumerate(buckets):
        if value < bucketend:
            tmp[j] = 1
            #break
    return tmp

  
  

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
        total_length = len(X)
        for i in range(total_length):
            p = float(i) / total_length
            tmp = bucketize_lt([.33, .66, 1.1], p)
            output.append(tmp)
        return numpy.array(output)

    def fit(self, X, y=None, **fit_params):
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

@lru.lru_cache(50000)
def sent_token_tag(text):
    '''Converts a text into sentences, then tokenizes them, and finally tags 
    them.
    
    Returns:
        sentences - a split list of sentences
        tokens    - a 2d array corresponding to sentence[tokens]
        tags      - a 2d array corresponding to sentence[tags]
    '''
    
    # converts text to an array of sentences
    sentences = nltk.tokenize.sent_tokenize(text)
    
    # converts each sentence to an array of tokens
    tokens    = [nltk.tokenize.word_tokenize(x) for x in sentences]
    
    # converts each array of tokens to a pos tag
    tags      = [nltk.pos_tag(x) for x in tokens]
    
    return sentences, tokens, tags

def extract_chunked_phrases(tree):
    '''Change a chunked sentence into the actual chunked words:
    
    e.g. 
    From "Hello world! Said Joe."
         [['Said', 'Joe']], [[('Said', 'NNP'), ('Joe', 'NNP')]]
    '''
     
    phrases = []
    tagged_phrases = []
    
    for a in tree:
        if not isinstance(a, tuple) and not isinstance(a, str) and a.label() != "":
                phrases.append([l[0] for l in a.leaves()])
                tagged_phrases.append(a.leaves())

    return phrases, tagged_phrases

class ChunksTransform(sklearn.base.TransformerMixin):
    '''Converts a text into NP chunks'''
    
    def __init__(self, grammar=None):
        	
    	self.grammer = grammar
    	if grammar == None:
            self.grammar = r"""
              NP: {<DT|PP\$>?<JJ>*<NN>}   # chunk determiner/possessive, adjectives and noun
                  {<NNP>+}                # chunk sequences of proper nouns
            """
        self.parser = nltk.RegexpParser(self.grammar)

    def process(self, X):
        output = []
        for text in X:
            textoutput = []
            sentences, tokens, tags = sent_token_tag(text)
            for taglist in tags:
                parsed = self.parser.parse(taglist)
                phrases, tagged_phrases = extract_chunked_phrases(parsed)
                textoutput += ["_".join(p) for p in phrases]
            output.append(textoutput)
            
        return output
    
    def transform(self, X, **transform_params):
        return np.array(self.process(X))

    def fit(self, X, y=None, **fit_params):    
        return self

class BinaryFeatureTransform(sklearn.base.TransformerMixin):
    knownclasses = set()
    words = []
    
    def transform(self, X, **transform_params):
        output = []
        for s in X:
            s = set(s)
            output.append([1 if x in s else 0 for x in self.words])
        
        return np.array(output)
    
    def fit(self, X, y=None, **fit_params):
        words = []
        
        for arr in X:
            words += arr
        
        for word in words:
            if word in self.knownclasses:
                continue
            else:
                self.knownclasses.add(word)
                self.words.append(word)            
        return self

def get_chunks_bin():
    ''' Gets a chunking and binarizer pipeline'''
    return sklearn.pipeline.Pipeline([   
                ('chunker', ChunksTransform()),
                ('binariz', BinaryFeatureTransform())])


sitetypes = {'centerforinquiry.net': 'news', 'verizonwireless.com': 'sales', 'motorola.com': 'sales', 'nps.gov': 'education', 'autopartswarehouse.com': 'shopping', 'onlineshoes.com': 'sales', 'dishnetwork.com': 'sales', 'baidu.com': 'other', 'cbsalary.com': 'service', 'shopjustice.com': 'sales', 'ticketmaster.com': 'sales', 'savemore.com': 'sales', 'comcast.com': 'sales', 'wineverygame.com': 'service', 'argos.co.uk': 'shopping', 'mattel.com': 'sales', 'rachaelray.com': 'news', 'aetna.com': 'shopping', 'acdelco.com': 'shopping', 'freescottwalsh.info': 'socialmedia', 'skype.com': 'service', 'cbc.ca': 'education', 'megavideo.com': 'entertainment', 'sprint.com': 'sales', 'byliner.com': 'news', 'cdiscount.com': 'discount', 'nordstrom.com': 'sales', 'readwrite.com': 'news', 'code.google.com': 'service', 'partycity.com': 'sales', 'freeonlinegames.com': 'entertainment', 'spiceworks.com': 'service', 'kodakgallery.com': 'service', 'linkline.com': 'sales', 'privacyscore.com': 'sales', 'primarygames.com': 'entertainment', 'carbonunity.com': 'education', 'gci.com': 'sales', 'scienceblogs.com': 'news', 'webshots.com': 'news', 'continental.com': 'shopping', 'boardgamegeek.com': 'news', 'stumbleupon.com': 'social', 'hautelook.com': 'sales', 'kongrenate.com': 'other', 'avidaviator.com': 'shopping', 'wix.com': 'service', 'xhamster.com': 'entertainment', 'zazzle.com': 'sales', 'lowes.com': 'sales', 'priceline.com': 'sales', 'bikeexif.com': 'news', 'tribalfusion.com': 'marketing', 'mittromney.com': 'other', 'peoplesmart.com': 'sales', 'mylife.com': 'other', 'instapaper.com': 'service', 'adotas.com': 'marketing', 'bl.uk': 'education', 'pbs.org': 'news', 'townandcountrymag.com': 'sales', 'dashlane.com': 'service', 'nickjr.com': 'entertainment', 'en.softonic.com': 'news', 'barbie.com': 'shopping', 'apple.com': 'shopping', 'mapstone.net': 'news', 'quisma.com': 'other', 'lifehacker.com': 'news', 'pchlotto.com': 'marketing', 'cruises.co.uk': 'shopping', 'carsdirect.com': 'shopping', 'waterhavens.com': 'sales', 'tinyurl.com': 'service', 'usestealth.com': 'other', 'change.org': 'other', 'groupon.com': 'sales', 'youtube.com': 'entertainment', 'myspace.com': 'socialmedia', 'gittip.com': 'banking', 'infospace.com': 'service', 'thepiratebay.org': 'other', 'mega.co.nz': 'other', 'ivillage.com': 'news', 'rr.com': 'sales', 'comedycentral.com': 'entertainment', 'color.com': 'other', 'cooks.com': 'news', 'globalresearch.ca': 'news', 'speeddate.com': 'sales', 'bestbuy.com': 'shopping', 'blockbuster.com': 'entertainment', 'www.sprintpcs.com': 'sales', 'pjmedia.com': 'news', 'toysrus.com': 'sales'}



class SitetypeTransform(sklearn.base.TransformerMixin):
    '''Sets up a site type for a given text'''
    
    def transform(self, X, **transform_params):
        output = []
        for s in sitetypes.keys():
            if s in X:
                output.append([sitetypes[s]])
                break
        else:
            output.append(["undef"])
        
        return np.array(output)
        
    def fit(self, X, y=None, **fit_params):    
        return self




def get_sitetype_bin():
    ''' Gets a sitetype and binarizer pipeline'''
    return sklearn.pipeline.Pipeline([   
                ('chunker', SitetypeTransform()),
                ('binariz', BinaryFeatureTransform())])
#print get_chunks_bin().fit_transform(['Hello world said Joseph Lewis', "Argos Limited is an awesome company to work for in the Joseph Lewis."])
