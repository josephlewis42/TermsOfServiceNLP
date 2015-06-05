
sents = []
verbs = []


with open("other_sentences.txt") as sin:
	sents.extend(sin.read().split("\n"))

with open("other_sentences_extracted.txt") as subjs:
	verbs.extend(subjs.read().split("\n"))
if len(sents) != len(verbs):
	print "mismatch at " + letter
	print len(sents)
	print len(verbs)
	exit(1)

print "other_pairings = {}".format( dict(zip(sents, verbs)))
