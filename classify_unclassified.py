import pickle
import tos

#!/usr/bin/env python


import glob
import os
import re
import string
import json
import textwrap
import classification2 as classification
import tos
import pickle

classifiers = classification.get_classifiers()
nsites = 0

with open('my_classifier.pickle') as f:
	pickle.dump(classifiers, f)

idf = tos.get_idf()

os.chdir("tosback2/crawl")
for fname in glob.glob("*"):
	
	nsites += 1
	
	os.chdir(fname)
	for policy in glob.glob("*.txt"):
		
		print(policy)
		
		document = ""		
		document += policy
		document += "\n==================================\n\n"

		with open(policy) as fp:
			rawtext = fp.read()
			paragraphs = rawtext.split("<p>")
			
			for paragraph in paragraphs:
			
				# clean it up in a standard way
				paragraph = tos.clean_text(paragraph)
				
				# remove empty text
				paragraph = paragraph.strip()
				if not paragraph:
					continue
				
				
				document += "\n\n**Paragraph:**\n\n\t" + textwrap.fill(paragraph).replace("\n", "\n\t")
				
				tf = tos.calculate_word_frequencies(paragraph)
				features = tos.tf_idf(tf, idf)
				
				document += "\n\n"
				for k, cls in classifiers.items():
					result = cls.classify(features)
					
					document += "* {} - {}\n".format(result, tos.classes[result[-1:]])

					   
		with open(policy + ".md", "w") as fp:
			fp.write(document)
		
		if nsites > 50:
			break
	
	os.chdir("..")

print(nparagraphs)
