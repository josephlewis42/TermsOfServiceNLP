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



db = None  # database

KEYWORDS = ["cookie", "terms", "legal", "agreement", "use", "conditions", "privacy"]
UA_STRINGS = ["*"] #, "Googlebot", "ia_archiver", "msnbot"] # look up useragent strings

'''
# article extraction
>>> from goose import Goose
>>> url = 'http://sociedad.elpais.com/sociedad/2012/10/27/actualidad/1351332873_157836.html'
>>> g = Goose()
>>> article = g.extract(url=url)
>>> article.title
u'Las listas de espera se agravan'
>>> article.cleaned_text[:150]
u'Los recortes pasan factura a los pacientes. De diciembre de 2010 a junio de 2012 las listas de espera para operarse aumentaron un 125%. Hay m\xe1s ciudad'

# robots.txt parsing
>>> import robotparser
>>>
>>> rp.set_url("http://www.musi-cal.com/robots.txt")
>>> rp.read()
>>> rp.can_fetch("*", "http://www.musi-cal.com/cgi-bin/search?city=San+Francisco")
False
>>> rp.can_fetch("*", "http://www.musi-cal.com/")
'''

def read_page(url):

    if not can_fetch(url):
        return ""

    try:
        req = urllib2.Request(url, headers={ 'User-Agent': 'Mozilla/5.0' })
        return urllib2.urlopen(req).read()
    except Exception as e:
        print "error {}".format(e)

    return ""

def fetch_potential_links(url):


    page = read_page(url)
    soup = BeautifulSoup(page)
    soup.prettify()

    hrefs = set()
    anchors = [a for a in soup.findAll('a', href=True)]
    for anchor in soup.findAll('a', href=True):
        if anchor.string == None:
            continue

        anchorstr = anchor.string.lower()
        if any([x in anchorstr for x in KEYWORDS]):
            href = urlparse.urljoin(url, anchor['href'])  # fix up relative
            hrefs.add(href)

    return hrefs




ROBOTS = {}
def get_robots(url):
    split = urlparse.urlsplit(url)
    robotspath = "{}://{}/robots.txt".format(split[0], split[1])
    if robotspath in ROBOTS:
        return ROBOTS[robotspath]

    rp = robotparser.RobotFileParser()
    rp.set_url(robotspath)
    rp.read()

    ROBOTS[robotspath] = rp
    return rp


def can_fetch(path):
    ''' Can we fetch the given page?
    '''
    rp = get_robots(path)
    for ua in UA_STRINGS:
        if rp.can_fetch(ua, path):
            return True

    return False

def fetch_site(hostname, db):

    c = db.cursor()
    path = "http://{}/".format(hostname)

    print "=" * 80
    print "fetching {}".format(path)

    if not can_fetch(path):
        print "not allowed to fetch {}".format(path)
        return

    links = fetch_potential_links(path)

    print "Got links: {}".format(", ".join(links))

    for link in links:
        if can_fetch(link):
            print "\n" * 2
            print "Article: {}".format( link )
            print "\n"
            g = Goose()
            article = g.extract(url=link)

            tup = (hostname.decode('utf-8', 'ignore'),
                    article.title.decode('utf-8', 'ignore'),
                    article.cleaned_text.decode('utf-8', 'ignore'),
                    article.raw_html.decode('utf-8', 'ignore'))

            c.execute("INSERT INTO tos (hostname, title, cleaned, html) VALUES (?, ?, ?, ?)", tup)

            print hostname
            print article.title
            print article.cleaned_text[:50]
            print "\n\n"

    conn.commit()

if __name__ == "__main__":

    #with sqlite3.open("test.sql") as db:# open database
    conn = sqlite3.connect('websites.db')
    c = conn.cursor()

    c.execute('''CREATE TABLE IF NOT EXISTS tos (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                hostname text,
                                title text,
                                cleaned text,
                                html text,
                                fetchtime timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)''')
    conn.commit()

    with open(sys.argv[1]) as websitefile:
        for line in websitefile.read().split("\n"):
            try:
                website = line.split(",")[1]
                fetch_site(website, conn)
            except Exception as e:
                print "Error: {}".format(e)
    conn.close()

    # read robots.txt
    #canget /
    # get /
    # parse / for legal, terms, cookies, privacy
    # foreach
    #   canget *
    #   get *
    #   parse *
    #   save to database
