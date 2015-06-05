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
from sklearn.svm import SVR
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
    #chunked = nltk.ne_chunk(tagged, binary=True)
    
    tagged = [("(","(") if t[0] == "(" else t for t in tagged]
    tagged = [(")",")") if t[0] == ")" else t for t in tagged]
    tagged = [("\"","\"") if t[0] in ["\"", "\'", "`", "``", "''", "\"\"", "\'`", "`\'"] else t for t in tagged]
    
    ne = []
    nepos = []
    '''for i in chunked:
        if isinstance(i, nltk.tree.Tree):
            words = map(lambda x: x[0], i)
            ne.append(words)
            nepos.append("ne")
        else:
            if i[1] == "PRP":
                ne.append([i[0]])
                nepos.append("prp")
    '''
    
    grammar = r"""
  PNP:  {(<,|CC>?<DT|PP\$>?<JJ>*<NNP|NNS>+)+}                # chunk sequences of proper nouns
       {<DT|PP\$>?<JJ>*<NNS|NNP>}   # chunk determiner/possessive, adjectives and noun

"""
    cp = nltk.RegexpParser(grammar)
    for i in cp.parse(tagged):
        if isinstance(i, nltk.tree.Tree):
            words = map(lambda x: x[0], i)
            ne.append(words)
            nepos.append("pnp")
    grammar = r"""
  NP:  {(<,|CC>?<DT|PP\$>?<JJ>*<NN|NNS>+)+}                # chunk sequences of proper nouns
       {<DT|PP\$>?<JJ>*<NN|NNS>}   # chunk determiner/possessive, adjectives and noun

"""
    cp = nltk.RegexpParser(grammar)
    for i in cp.parse(tagged):
        if isinstance(i, nltk.tree.Tree):
            words = map(lambda x: x[0], i)
            ne.append(words)
            nepos.append("np")
    
    grammar = r"""PP: {(<,|CC>?<PRP>)+}                # chunk sequences of prepositions
"""
    cp = nltk.RegexpParser(grammar)
    for i in cp.parse(tagged):
        if isinstance(i, nltk.tree.Tree):
            words = map(lambda x: x[0], i)
            ne.append(words)
            nepos.append("pp")
    
    return tokens, tagged, ne, nepos


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

def get_features(termne, defnne, sentencetokens, termnepos, defnnepos):
    features = []
    
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
    
    if defsub < termsub:
        features.append("definition_before_term")
    else:
        features.append("term_before_definition")
    
    

    for word in between:
        features.append("between_word_{}".format(word))
    
    for i, b1 in enumerate(between):
        for b2 in between[i:]:
            features.append("between_bigram_{}_{}".format(b1, b2))
    
    for word in defbefore[-2:]:
        features.append("def_pre_{}".format(word))
    
    #for word in defafter[:4]:
    #    features.append("def_post_{}".format(word))
        
    for word in termbefore[-2:]:
        features.append("term_pre_{}".format(word))
    #for word in termafter[:4]:
    #    features.append("term_post_{}".format(word))
    
    features.append("term_pos_{}".format(termnepos))
    features.append("defn_pos_{}".format(defnnepos))
    
    features.append("termdefn_pos_{}_{}".format(termnepos, defnnepos))
    
    for word in termne:
        if word[-4:] in [".com",".net",".org",".edu", "o.uk"]:
            features.append("term_is_website")
    
    for word in defnne:
        if word[-4:] in [".com",".net",".org",".edu", "o.uk"]:
            features.append("defn_is_website")
    
    if len(between) <= 5:
        features.append("dist_lte_five")
    elif len(between) < 10:
        features.append("dist_fiveten")
    elif len(between) < 20:
        features.append("dist_tentwenty")
    else:
        features.append("dist_gt_twenty")
    
    for word in termne:
        if word in defnne:
            features.append("common_word")
    
    fuzzyterm = fuzzy_clean("".join(termne))
    fuzzydef = fuzzy_clean("".join(defnne))
    
    defcomb = "".join(defnne) 
    if defcomb == defcomb.upper():
        features.append("defn_all_upper_case")
    
    dictterms = ["(including", "(\"", ".com)", ".org)", ".net)"]
    for d in dictterms:
        if d in defcomb:
            features.append("dictterm_in")
            break
    
    acronym_def = "".join([d[0] for d in defnne])
    acronym_term = "".join([t[0] for t in termne])
    
    if acronym_def in termne:
        features.append("acronym")
    if acronym_term in defnne:
        features.append("acronym")
    
    if find_sublist(termne, defnne):
        features.append("term_in_defn")
    
    if find_sublist(defnne,termne):
        features.append("defn_in_term")
    
    return " ".join(features)


def create_examples(sentence, term, definition):
    ''' Creates a set of training examples from a given sentence.
    
    '''
    st, sta, sne, snepos = sent_parse(sentence)
    tt, tta, tne, tnepos = sent_parse(term)
    dt, dta, dne, dnepos = sent_parse(definition)
    
    tne = []
    dne = []
    tne_pos = []
    dne_pos = []
    
    for i, ex in enumerate(sne):
        if find_sublist(ex, tt) >= 0:
            tne.append(ex)
            tne_pos.append(snepos[i])
        if find_sublist(ex, dt) >= 0:
            dne.append(ex)
            dne_pos.append(snepos[i])
    
    examples = []
    for i, t_entity in enumerate(tne):
        for j, d_entity in enumerate(dne):
            examples.append(([t_entity, d_entity, st, tne_pos[i], dne_pos[j]],1))
    
    for i, ne1 in enumerate(sne):
        for j, ne2 in enumerate(sne[i + 1:]):

            if ne1 in dne or ne2 in tne:
                continue
            if ne2 in dne or ne1 in tne:
                continue

            examples.append(([ne1, ne2, st, snepos[i], snepos[j]], 0))
    
    ex2 = []
    for ex in examples:
        if ex not in ex2:
            ex2.append(ex)
    
    random.shuffle(ex2)
    
    return ex2

def get_examples():
    examples = []
    with open(defns_extracted_file) as fd:
        text = fd.read()
        
        # clean it up
        text = ''.join([i if ord(i) < 128 else '' for i in text])
        defns = text.split("---\n")
        

        rawsseen = set()
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
            
            if term == "" or definition == "":
                continue
            
            raw = lines[0]
            if raw in rawsseen:
                print "problem, raw already seen!: {}".format(raw)
                continue
            rawsseen.add(raw)
            
            #print "Extracted: {}".format(raw)
            
            if len(raw.split()) > 150:
                #print "Couldn't parse {}, too long ({})".format(raw, raw.split())
                continue # ignore sentences we can't parse due to memory
            
            examples.append((raw, term, definition))
    return examples

def get_discriminator():
    examples = get_examples()
    
    #examples = filter(lambda x: isinstance(x[0], str), examples)
    train, test = split_data(.8, .2, examples)
    
    # create all training examples together
    tmp = []
    for ex in train:
        tmp += create_examples(*ex)
    
    train = tmp
    
    print "Train size: {}".format(len(train))
    print "Test size:  {}".format(len(test))
    
    text_clf = Pipeline([
          ('features', DefinitionFeatureExtractor()),
          ('counts', CountVectorizer(tokenizer=lambda x: x.split())),
    ('perceptron', Perceptron(class_weight="auto"))])#(class_weight='auto'))])

    traindata, traintarget = zip(*train)
    traintarget = [i for i in traintarget]

    algorithm = text_clf.fit(list(traindata), traintarget)
    
    
    total_correct = 0
    total_incorrect = 0
    
    # testing
    for example in examples: #test:
        testgrouping = create_examples(*example)
        
        if len(testgrouping) == 0:
            continue

        
        data, target = zip(*testgrouping)
        data = list(data)
        target = list(target)
        
        predicted = algorithm.decision_function(data)
        maxval = max(predicted)
        
        top = sorted(predicted)[-1:]
        
        print "-" * 80
        print "Correct: term: {} def: {}".format(example[1], example[2])
        print "Max val {}".format(maxval)
        correct = False
        for i, dat in enumerate(data):
            if predicted[i] in top:
                print ""
                print "Term: {}".format(data[i][0])
                print "Defn: {}".format(data[i][1])
                
                if target[i] == 1:
                    print "> Correct"
                    correct = True

                            
        print "Sentence"
        print data[i][2]
        
        print ""
        
        if correct:
            total_correct += 1
        else:
            total_incorrect += 1
        
        #for i, d in enumerate(data):
        #    print "{} -> term: {}, def: {}".format(predicted[i], d[0], d[1])
    
    print "Total Correct: {} Incorrect: {} Total: {}".format(total_correct, total_incorrect, len(examples))

    return algorithm


if __name__ == "__main__":
    get_discriminator()

