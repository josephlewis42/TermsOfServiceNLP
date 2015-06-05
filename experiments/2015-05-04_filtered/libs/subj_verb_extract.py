#!/usr/bin/env python2
'''
Extracts a subject and verb from a maltparser/conll format input
'''

import sys
import argparse

'''
Examples:
1	I	_	PRP	PRP	_	2	nsubj	_	_
2	said	_	VBD	VBD	_	0	null	_	_
3	hello	_	UH	UH	_	2	dep	_	_
4	!	_	.	.	_	2	punct	_	_

1	Argos.co.uk	_	NN	NN	_	5	nsubj	_	_
2	is	_	VBZ	VBZ	_	5	cop	_	_
3	an	_	DT	DT	_	5	det	_	_
4	awesome	_	JJ	JJ	_	5	amod	_	_
5	company	_	NN	NN	_	0	null	_	_
6	.	_	.	.	_	5	punct	_	_

1	We	_	PRP	PRP	_	3	nsubj	_	_
2	may	_	MD	MD	_	3	aux	_	_
3	steal	_	VB	VB	_	0	null	_	_
4	your	_	PRP$	PRP$	_	5	poss	_	_
5	data	_	NNS	NNS	_	3	dobj	_	_
6	.	_	.	.	_	3	punct	_	_

1	Our	_	PRP$	PRP$	_	2	poss	_	_
2	partners	_	NNS	NNS	_	4	nsubj	_	_
3	may	_	MD	MD	_	4	aux	_	_
4	use	_	VB	VB	_	0	null	_	_
5	tracking	_	NN	NN	_	6	nn	_	_
6	cookies	_	NNS	NNS	_	4	dobj	_	_
7	.	_	.	.	_	4	punct	_	_
'''


instance = []
output_seperator = ":"
outputsubj = True
outputverb = True
outputobj  = True
negprefix = "neg_"
outputneg = True

def process():
	global instance
	global seperator
	global outputsubj
	global outputverb
	global outputobj
	global negprefix
	global outputneg
	
	subj = ""
	verb = ""
	verbline = ""
	obj = ""
	isneg = False
	
	for line in instance:
		split = line.split("\t")
		refnum = split[6]
		if refnum.strip() == "0":
			verbline = split[0]
	
	for line in instance:
		split = line.split("\t")
		
		linetype = split[7]
		value = split[1]
		refnum = split[6]
		
		if linetype == "nsubj":
			subj = value
		elif linetype == "dobj" and refnum == verbline:
			obj = value
		elif linetype == "neg" and refnum == verbline:
			isneg = True
		elif refnum == "0":
			verb = value
	
	
	output = []
	if outputsubj:
		output.append(subj)
	if outputverb:
		if outputneg and isneg:
			verb = "neg_" + verb
		output.append(verb)
	if outputobj:
		output.append(obj)
	
	sys.stdout.write(output_seperator.join(output))
	sys.stdout.write("\n")
	instance = []


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("--seperator", help="the seperator to use, default: ':'")
	parser.add_argument("-v", "--verb", help="Don't show the verb", action="store_true")
	parser.add_argument("-s", "--subj", help="Don't show the subject", action="store_true")
	parser.add_argument("-o", "--obj", help="Don't show the object", action="store_true")
	parser.add_argument("-n", "--neg", help="Don't append the negative prefix to the verb", action="store_true")
	parser.add_argument("--negprefix", help="The prefix to append to verbs determined to be negative", default="neg_")
	
	args = parser.parse_args()
	
	if args.verb:
		outputverb = False
	if args.subj:
		outputsubj = False
	if args.obj:
		outputobj = False
	
	if args.seperator:
		output_seperator = args.seperator
	
	if args.negprefix:
		negprefix = args.negprefix
	if args.neg:
		outputneg = False
	
		
	for line in sys.stdin:
		if line == "\n":
			process()
		else:
			instance.append(line)
			
