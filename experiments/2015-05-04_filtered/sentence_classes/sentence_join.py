
sents = []
verbs = []

letter = 'c'
for letter in 'cdilmoprt':
	with open("{}_sentences.txt".format(letter)) as sin:
		sents.extend(sin.read().split("\n"))

	with open("{}_sents_parsed_subjverb.txt".format(letter)) as subjs:
		verbs.extend(subjs.read().split("\n"))
	if len(sents) != len(verbs):
		print "mismatch at " + letter
		print len(sents)
		print len(verbs)
		exit(1)

print "pairings = {}".format( dict(zip(sents, verbs)))
