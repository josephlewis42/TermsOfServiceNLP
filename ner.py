import tos

txtfiles = tos.getFiles("/home/joseph/Desktop/tos3/tosback2/crawl")

txtfiles = txtfiles[:5]

for f in txtfiles:
    print "\n"*4
    print "="*80

    with open(f) as fd:
        text = fd.read()
        paragraphs = text.split("<p>")
        
        for paragraph in paragraphs:
            raw = tos.strip_html(paragraph) # remove html  
            raw = raw.replace("third par", "Third Par")
            tokens = tos.tokenize(raw) 
            sentences = tos.split_sentences(tokens)
            
            ners = set()
            
            for sentence in sentences:
                try:
                    ner = tos.er_senna(sentence)
                    ners.update(ner)
                    #print "-"* 80
                    #print " ".join(sentence)
                    #print "\n"
                    #print "senna:"
                    #print ner
                except Exception as e:
                    #print "!"* 80
                    print e
                    #print sentence
            
            print "-"*80
            print ners
            
