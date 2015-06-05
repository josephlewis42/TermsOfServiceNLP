import re
from nltk.tokenize import word_tokenize
import string
import nltk

def strip_html(text):
    return re.sub('(<[^<]+?>|\\t\\r)'," ", text)

def definition_extractor(raw_text):
    '''Extracts sentences potentially containing definitions and returns them
    as a list.
    
    '''
    unclosed_line = strip_html(raw_text)
    
    # remove unicode junk
    unclosed_line = filter(lambda x: x in string.printable, unclosed_line)

    # ignore single letters with nothing, e.g. "( i )"
    unclosed_line = re.sub(r'\(\s?\w\s?\)', "", unclosed_line) 
    
    # ignore lines with only (small) roman numerals
    unclosed_line = re.sub(r'\(\s?[\s?\.iIvVxX]+\s?\)', "", unclosed_line) 
    
    # ignore lines with only phone numbers
    unclosed_line = re.sub(r'\(\s?[\d\.-]+\s?\)', "", unclosed_line)
    
    # replace common words that get our sentence modifier mixed up
    unclosed_line = unclosed_line.replace("a.k.a.", "aka")
    unclosed_line = unclosed_line.replace("i.e.", "ie")
    unclosed_line = unclosed_line.replace("e.g.", "eg")
    
    # split into lines
    lines = re.split(r"(\.\s+|\n)", unclosed_line)
    
    output = []
    for line in lines:        
        if "(" in line or "collectiv" in line or "referred" in line or "mean" in line:
            output.append(line)
    
    return output
