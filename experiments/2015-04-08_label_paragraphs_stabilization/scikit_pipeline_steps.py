#!/usr/bin/env python2

import nltk.tokenize
import discriminator
import sklearn
import numpy as np

class EntityTransform(sklearn.base.TransformerMixin):
    def fit(self, X, y=None, **fit_params):
        return self
    
    def transform(self, X, **transform_params):
        output = []
        for text in X:
            b = []
            words = nltk.tokenize.word_tokenize(text)

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
        return np.array([nltk.tokenize.word_tokenize(x) for x in X])

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
