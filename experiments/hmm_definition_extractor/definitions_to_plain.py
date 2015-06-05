#!/usr/bin/env python3
'''

Copyright 2014 Joseph Lewis <joehms22@gmail.com> | <joseph@josephlewis.net>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the
  distribution.
* Neither the name of the  nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


'''

from nltk.tag import HiddenMarkovModelTrainer
from nltk.tokenize import word_tokenize
import nltk.tag.hmm


i = 0


data = []


def process(joined_again, defline):

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

with open("definitions_extracted.txt") as definitions:
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
        
        '''
        for line in lines:
            if "/NN" in line:
                pos_tagged = line
            if "/out" in line:
                hmm_tagged = line
            if "term: " in line:
                termline = line
            elif line.startswith("definition: "):
                defline = line
        '''
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

hmt.test(test)


for t in test:
    print "---"
    print t
    tag = hmt.tag(t)
    print hmt.tag(t)
    
    print t == tag
    

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
