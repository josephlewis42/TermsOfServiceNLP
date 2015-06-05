#!/usr/bin/env python3
'''
Copyright 2015 Joseph Lewis  <joseph@josephlewis.net>

All rights reserved
'''

from nltk.tag import HiddenMarkovModelTrainer
from nltk.tokenize import word_tokenize
import nltk.tag.hmm


i = 0


data = []


def process(joined_again, defline):

    definition = defline.split(":")[1].strip()
    
    # if no defn, just return all dashes.
    if definition == "":
        data.append([(u,"-") for u in joined_again.split()])
        return
    
    definition = definition.replace("``", "\"")
    definition = definition.replace("''", "\"")
    definition = definition.replace("/", " / ")
    
    definition = word_tokenize(definition)
    
    text = joined_again.split()
    
    j = 0
    for i in range(len(text)):
        if text[i:i + len(definition)] == definition:
        
        
            arr = []
            unbefore = text[:i-2]
            pre = text[i - 2:i]
            within = text[i:i + len(definition)]
            post = text[i + len(definition):i + len(definition) + 1]
            after = text[i + len(definition) + 1:]
            
            
            arr += [(u,"-") for u in unbefore]
            arr += [(p,"PRE") for p in pre]
            arr += [(w, "TARGET") for w in within]
            arr += [(p, 'POST') for p in post]
            arr += [(a,"-") for a in after]
            
            data.append(arr)            
            break

with open("../../data/definitions_extracted.txt") as definitions:
    text = definitions.read()
    defs = text.split("---\n")
    
    for definition in defs:
        if not definition:
            continue
        i += 1
        
        lines = definition.split("\n")
        if len(lines) != 10:
            print("fail")
            print definition
            exit()

        raw = lines[0]
        cleaned = lines[1]
        joined_again = lines[2]
        pos_tagged = lines[3]
        hmm_tagged = lines[4]
        
        termline = lines[7]
        defline = lines[8]
        
        if termline.split(":")[1].strip() == "":
            continue
        
        print "---"
        print termline
        print defline
        
        process(joined_again, termline)
        '''
        definition = defline.split(":")[1].strip()
        
        definition = definition.replace("``", "\"")
        definition = definition.replace("''", "\"")
        definition = definition.replace("/", " / ")
        
        definition = word_tokenize(definition)
        
        text = joined_again.split()
        
        
        j = 0
        for i in range(len(text)):
            if text[i:i + len(definition)] == definition:
            
            
                arr = []
                unbefore = text[:i-2]
                pre = text[i - 2:i]
                within = text[i:i + len(definition)]
                post = text[i + len(definition):i + len(definition) + 1]
                after = text[i + len(definition) + 1:]
                
                
                arr += [(u,"-") for u in unbefore]
                arr += [(p,"PRE") for p in pre]
                arr += [(w, "TARGET") for w in within]
                arr += [(p, 'POST') for p in post]
                arr += [(a,"-") for a in after]
                
                data.append(arr)
                
                
                
                break
        
        
        #datasplit.append() 
        '''
        


train = data[:int(len(data) * .8)]
test = data[int(len(data) * .8):]

print len(train)
print len(test)


trainer = HiddenMarkovModelTrainer()

hmt = trainer.train_supervised(train)

testresults = hmt.test(test)

allwords = 0
wordscorrect = 0
entirecorrect = 0
for t in test:
    print "---"
    tag = hmt.tag([e[0] for e in t])
    print "\t".join([e[0] for e in t])
    print "\t".join([e[1] for e in t])
    print "\t".join([e[1] for e in tag])
    
    ec = True
    for i in range(len(t)):
        if tag[i][1] == "-":
            continue
        
        allwords += 1
        if tag[i][1] == t[i][1]:
            wordscorrect += 1
        else:
            ec = False
    if ec:
        entirecorrect += 1

print "Correct words: {} / {}".format(wordscorrect, allwords)
print "Correct docs: {} / {}".format(entirecorrect, len(test))
    

'''
Train a new HiddenMarkovModelTagger using the given labeled and
unlabeled training instances. Testing will be performed if test
instances are provided.

:return: a hidden markov model tagger
:rtype: HiddenMarkovModelTagger
:param labeled_sequence: a sequence of labeled training instances,
    i.e. a list of sentences represented as tuples
:type labeled_sequence: list(list)
:param test_sequence: a sequence of labeled test instances
:type test_sequence: list(list)
:param unlabeled_sequence: a sequence of unlabeled training instances,
    i.e. a list of sentences represented as words
:type unlabeled_sequence: list(list)
:param transform: an optional function for transforming training
    instances, defaults to the identity function, see ``transform()``
:type transform: function
:param estimator: an optional function or class that maps a
    condition's frequency distribution to its probability
    distribution, defaults to a Lidstone distribution with gamma = 0.1
:type estimator: class or function
:param verbose: boolean flag indicating whether training should be
    verbose or include printed output
:type verbose: bool
:param max_iterations: number of Baum-Welch interations to perform
:type max_iterations: int

'''
