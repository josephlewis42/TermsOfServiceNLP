#!/usr/bin/env python

# privacy, cookies, terms, legal
#goose-extractor, beautifulsoup


# website, keyword, html, extracted_text, URL, title
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from goose import Goose
import robotparser
import os
import sqlite3
import urllib2
from BeautifulSoup import BeautifulSoup
import urlparse


if __name__ == "__main__":

    conn = sqlite3.connect('websites.db')
    c = conn.cursor()
    c.execute('''SELECT cleaned FROM tos''')
    
    s = 0
    num = 0
    while True:
        r = c.fetchone()
        if r == None:
            break
            
        if len(r[0]) < 200:
            continue
        num += 1
        s += len(r[0].split())
    conn.close()
    
    print float(s) / num
