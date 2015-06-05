
//-*-C++-*-

#include <string.h>
#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"
#include "SemClass.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

int gNumRules;
int gNumPatterns;
int gNumItems;

void InitializeRuleCounters()
{
  gNumRules = 0;
  gNumPatterns = 0;
  gNumItems = 0;
}

void WriteRuleCounts()
{
  cout << "Number of Rules: " << gNumRules;
  cout << "\nNumber of Patterns: " << gNumPatterns;
  cout << "\nNumber of Items: " << gNumItems << "\n";
}


// PatternItem routines

PatternItem::PatternItem(char * word, char* tag)
{
  gNumItems++;
  m_Word = new WordList;
  m_Word->word = word;
  m_Word->prevText = NULL;
  m_Word->origText = NULL;
  m_Word->next = NULL;
  m_Tag = new WordList;
  m_Tag->word = tag;
  m_Tag->prevText = NULL;
  m_Tag->origText = NULL;
  m_Tag->next = NULL;
  m_Sem = NULL;
  m_WordNot = NULL;
  m_TagNot = NULL;
  m_SemNot = NULL;
}

PatternItem::PatternItem()
{
  gNumItems++;
  m_Word = NULL;
  m_Tag = NULL;
  m_Sem = NULL;
  m_WordNot = NULL;
  m_TagNot = NULL;
  m_SemNot = NULL;
}

PatternItem::PatternItem(WordList* word, WordList* tag, WordList* sem,
			 WordList* wordNot, WordList* tagNot, WordList* semNot)
{
  gNumItems++;
  m_Word = CopyWordList(word);
  m_Tag = CopyWordList(tag);
  m_Sem = CopyWordList(sem);
  m_WordNot = CopyWordList(wordNot);
  m_TagNot = CopyWordList(tagNot);
  m_SemNot = CopyWordList(semNot);
}

PatternItem::PatternItem(PatternItem* oldItem)
{
  gNumItems++;
  m_Word = CopyWordList(oldItem->GetWord());
  m_Tag = CopyWordList(oldItem->GetTag());
  m_Sem = CopyWordList(oldItem->GetSem());
  m_WordNot = CopyWordList(oldItem->GetWordNot());
  m_TagNot = CopyWordList(oldItem->GetTagNot());
  m_SemNot = CopyWordList(oldItem->GetSemNot());
}

PatternItem::~PatternItem()
{
  gNumItems++;
  // need to free WordLists
  WordList* i = NULL;
  WordList* j = NULL;

  i = m_Word;
  while (i) {
    j = i->next;
    delete i;
    i = NULL;
    i = j;
  }
  i = m_Tag;
  while (i) {
    j = i->next;
    delete i;
    i = NULL;
    i = j;
  }
  i = m_Sem;
  while (i) {
    j = i->next;
    delete i;
    i = NULL;
    i = j;
  }
  i = m_WordNot;
  while (i) {
    j = i->next;
    delete i;
    i = NULL;
    i = j;
  }
  i = m_TagNot;
  while (i) {
    j = i->next;
    delete i;
    i = NULL;
    i = j;
  }
  i = m_SemNot;
  while (i) {
    j = i->next;
    delete i;
    i = NULL;
    i = j;
  }
} // ~PatternItem

int PatternItem::CalcSize(int isFiller) 
{
  int temp = 2; // base of item
  WordList *i = NULL;
  i = m_Word;
  while(i) {
    // weight words more heavily because they're so specific
    temp = temp + 2;
    if (isFiller) {
      temp = temp + 0;
    }
    i = i->next;
  }
  i = m_Tag;
  while(i) {
    temp++;
    i = i->next;
  }
  i = m_Sem;
  while(i) {
    temp++;
    i = i->next;
  }
  i = m_WordNot;
  while(i) {
    temp++;
    i = i->next;
  }
  i = m_TagNot;
  while(i) {
    temp++;
    i = i->next;
  }
  i = m_SemNot;
   while(i) {
    temp++;
    i = i->next;
  }
  return temp;
}

void PatternItem::Read(istream& stream, WordStore& ws)
{
  // Assume that the first character we read will be the first character
  // of m_Word

  char      c;
  char     buffer[300];
  int       i;
  char*     word = NULL;
  WordList* curWord = NULL;
  WordList* newWord = NULL;
  char      endChar;

  // Read m_Word
  stream.get(c);
  if (c == '_') {
    // no constraint on m_Word
    stream.get(c); //read the comma
  } else {
    if (c == '[') {
      endChar = ']';
      stream.get(c);
    } else
      endChar = ',';
    curWord = NULL;
    // word loop
    while (c != endChar) {
      i = 0;
      if (c == 39) { // we're quoted
	int  cont = 1;
	char nextc;
	
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else  {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while ((c != ',') && (c != endChar)) {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      if (curWord) {
	newWord = new WordList;
	newWord->word = word;
	newWord->prevText = NULL;
	newWord->origText = NULL;
	newWord->next = NULL;
	curWord->next = newWord;
	curWord = newWord;
      } else {
	curWord = new WordList;
	curWord->word = word;
	curWord->prevText = NULL;
	curWord->origText = NULL;
	curWord->next = NULL;
	m_Word = curWord;
      }
    if ((endChar == ']') && (c == ','))
      stream.get(c);
    }
    if (endChar == ']') 
      stream.get(c);
  } // mWord

  // Read m_Tag
  stream.get(c);
  if (c == '_') { // no constraints on m_Tag
    m_Tag = NULL;
    stream.get(c); // read the comma
  } else {
    if (c == '[') {
      endChar = ']';
      stream.get(c);
    } else
      endChar = ',';
    curWord = NULL;
    // word loop
    while (c != endChar) {
      i = 0;
      if (c == 39) { // we're quoted
	int  cont = 1;
	char nextc;
	
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else  {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while ((c != ',') && (c != endChar)) {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      if (curWord) {
	newWord = new WordList;
	newWord->word = word;
        newWord->prevText = NULL;
        newWord->origText = NULL;
	newWord->next = NULL;
	curWord->next = newWord;
	curWord = newWord;
      } else {
	curWord = new WordList;
	curWord->word = word;
        curWord->prevText = NULL;
        curWord->origText = NULL;
	curWord->next = NULL;
	m_Tag = curWord;
      }
      if ((endChar == ']') && (c == ',')) {
	stream.get(c);
      }
    }
  } // m_Tag

  // Read m_Sem
  stream.get(c);
  if (c == '_') { // no constraints on m_Sem
    m_Sem = NULL;
    stream.get(c); // read the comma
  } else {
    if (c == '[') {
      endChar = ']';
      stream.get(c);
    } else
      endChar = ',';
    curWord = NULL;
    // word loop
    while (c != endChar) {
      i = 0;
      if (c == 39) { // we're quoted
	int  cont = 1;
	char nextc;
	
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else  {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while ((c != ',') && (c != endChar)) {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      if (curWord) {
	newWord = new WordList;
	newWord->word = word;
        newWord->prevText = NULL;
        newWord->origText = NULL;
	newWord->next = NULL;
	curWord->next = newWord;
	curWord = newWord;
      } else {
	curWord = new WordList;
	curWord->word = word;
        curWord->prevText = NULL;
        curWord->origText = NULL;
	curWord->next = NULL;
	m_Sem = curWord;
      }
    if ((endChar == ']') && (c == ','))
      stream.get(c);
    }
  } // m_Sem

  // Read m_WordNot
  stream.get(c);
  if (c == '_') { // no constraints on m_WordNot
    m_WordNot = NULL;
    stream.get(c); // read the comma
  } else {
    if (c == '[') {
      endChar = ']';
      stream.get(c);
    } else
      endChar = ',';
    curWord = NULL;
    // word loop
    while (c != endChar) {
      i = 0;
      if (c == 39) { // we're quoted
	int  cont = 1;
	char nextc;
	
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else  {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while ((c != ',') && (c != endChar)) {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      if (curWord) {
	newWord = new WordList;
	newWord->word = word;
        newWord->prevText = NULL;
        newWord->origText = NULL;
	newWord->next = NULL;
	curWord->next = newWord;
	curWord = newWord;
      } else {
	curWord = new WordList;
	curWord->word = word;
        curWord->prevText = NULL;
        curWord->origText = NULL;
	curWord->next = NULL;
	m_WordNot = curWord;
      }
    if ((endChar == ']') && (c == ','))
      stream.get(c);
    }
  } // m_WordNot

  // Read m_TagNot
  stream.get(c);
  if (c == '_') { // no constraints on m_TagNot
    m_TagNot = NULL;
    stream.get(c); // read the comma
  } else {
    if (c == '[') {
      endChar = ']';
      stream.get(c);
    } else
      endChar = ',';
    curWord = NULL;
    // word loop
    while (c != endChar) {
      i = 0;
      if (c == 39) { // we're quoted
	int  cont = 1;
	char nextc;
	
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else  {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while ((c != ',') && (c != endChar)) {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      if (curWord) {
	newWord = new WordList;
	newWord->word = word;
        newWord->prevText = NULL;
        newWord->origText = NULL;
	newWord->next = NULL;
	curWord->next = newWord;
	curWord = newWord;
      } else {
	curWord = new WordList;
	curWord->word = word;
        curWord->prevText = NULL;
        curWord->origText = NULL;
	curWord->next = NULL;
	m_TagNot = curWord;
      }
    if ((endChar == ']') && (c == ','))
      stream.get(c);
    }
    if (endChar == ']') stream.get(c);
  } // m_TagNot

  // Read m_SemNot
  stream.get(c);
  if (c == '_') { // no constraints on m_SemNot
    m_SemNot = NULL;
    stream.get(c); // read the parenthesis
  } else {
    if (c == '[') {
      endChar = ']';
      stream.get(c);
    } else
      endChar = ')';
    curWord = NULL;
    // word loop
    while (c != endChar) {
      i = 0;
      if (c == 39) { // we're quoted
	int  cont = 1;
	char nextc;
	
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else  {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while ((c != ',') && (c != endChar)) {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      if (curWord) {
	newWord = new WordList;
	newWord->word = word;
        newWord->prevText = NULL;
        newWord->origText = NULL;
	newWord->next = NULL;
	curWord->next = newWord;
	curWord = newWord;
      } else {
	curWord = new WordList;
	curWord->word = word;
        curWord->prevText = NULL;
        curWord->origText = NULL;
	curWord->next = NULL;
	m_SemNot = curWord;
      }
    if ((endChar == ']') && (c == ','))
      stream.get(c);
    }
  } // m_SemNot

} // PatternItem::Read


void PatternItem::Write(ostream& stream)
{
  stream << "item(";
  this->WriteConstraints(stream);
  stream << ")";
}

/*int PatternItem::Match(Document* doc, int retry)
{
  if (retry)
    return FAILED; // can't retry an item match, only a list match
  WordList* junk = NULL;
  int result;
  WordList* i = NULL;
  result = this->MatchFiller(doc,junk, retry);
  while (junk) {
    i = junk;
    junk = junk->next;
    delete i;
    i = NULL;
  }
  return result;
}*/

int PatternItem::Match(Document* doc, int retry)
{
  if (retry)
    return FAILED; // can't retry an item match, only a list match
  DocWord*  testElement = NULL;
  int       goodMatch;
  WordList* i = NULL;
  int       semMatch;  

  testElement = doc->GetNextWord();
  if (!testElement) return ENDOFDOC;
  if (m_Word) {
    goodMatch = 0;
    i = m_Word;
    while (i && !goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      return FAILED;
    } else {
      // word Matches; do nothing but drop to next test
    }
  } else {
    // word Matches; do nothing but drop to next test
  }
  if (m_Tag) {
    goodMatch = 0;
    i = m_Tag;
    while (i && !goodMatch) {
      if (i->word == testElement->theTag) {
        goodMatch = 1;
      } else {
        i = i->next;
      }
    }
    if (!goodMatch) {
      return FAILED;
    } else {
      // tag matches; do nothing but drop to next test
    }
  } else {
    // tag matches; do nothing but drop to next test
  }
  if (m_WordNot) {
    goodMatch= 1;
    i = m_WordNot;
    while (i && goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      return FAILED;
    } else {
      // WordNot matches; do nothing but drop to next test
    }
  } else {
    // WordNot matches; do nothing but drop to next test
  }
  if (m_TagNot) {
    goodMatch= 1;
    i = m_TagNot;
    while (i && goodMatch) {
      if (i->word == testElement->theTag) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      return FAILED;
    } else {
      // TagNot matches; do nothing but drop to next test
    }    
  } else {
    // TagNot matches; do nothing but drop to next test
  }
  if (m_Sem) {
    goodMatch = 0;
    i = m_Sem;
    while (i && !goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      return FAILED;
    } else {
      // Sem matches; do nothing but drop to next test
    }
  } else {
    // Sem matches; do nothing but drop to next test
  }
  if (m_SemNot) {
    goodMatch = 1;
    i = m_SemNot;
    while (i && goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      return FAILED;
    } else {
      // SemNot matches; do nothing but drop to next test
    }
  } else {
    // SemNot matches, too
  }
  return GOOD;
}

int PatternItem::MatchFiller(Document* doc,WordList*& filler, int retry)
{
  if (retry)
    return FAILED; // can't retry an item match, only a list match
  WordList* result = NULL;
  DocWord*  testElement = NULL;
  int       goodMatch;
  WordList* i = NULL;
  int       semMatch;  

  testElement = doc->GetNextWord();
  if (!testElement) return ENDOFDOC;
  if (m_Word) {
    goodMatch = 0;
    i = m_Word;
    while (i && !goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      filler = NULL;
      return FAILED;
    } else {
      // word Matches; do nothing but drop to next test
    }
  } else {
    // word Matches; do nothing but drop to next test
  }
  if (m_Tag) {
    goodMatch = 0;
    i = m_Tag;
    while (i && !goodMatch) {
      if (i->word == testElement->theTag) {
        goodMatch = 1;
      } else {
        i = i->next;
      }
    }
    if (!goodMatch) {
      filler = NULL;
      return FAILED;
    } else {
      // tag matches; do nothing but drop to next test
    }
  } else {
    // tag matches; do nothing but drop to next test
  }
  if (m_WordNot) {
    goodMatch= 1;
    i = m_WordNot;
    while (i && goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      filler = NULL;
      return FAILED;
    } else {
      // WordNot matches; do nothing but drop to next test
    }
  } else {
    // WordNot matches; do nothing but drop to next test
  }
  if (m_TagNot) {
    goodMatch= 1;
    i = m_TagNot;
    while (i && goodMatch) {
      if (i->word == testElement->theTag) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      filler = NULL;
      return FAILED;
    } else {
      // TagNot matches; do nothing but drop to next test
    }    
  } else {
    // TagNot matches; do nothing but drop to next test
  }
  if (m_Sem) {
    goodMatch = 0;
    i = m_Sem;
    while (i && !goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      filler = NULL;
      return FAILED;
    } else {
      // Sem matches; do nothing but drop to next test
    }
  } else {
    // Sem matches; do nothing but drop to next test
  }
  if (m_SemNot) {
    goodMatch = 1;
    i = m_SemNot;
    while (i && goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      filler = NULL;
      return FAILED;
    } else {
      // SemNot matches; do nothing but drop to next test
    }
  } else {
    // SemNot matches, too
  }
  result = new WordList;
  result->word = testElement->theWord;
  result->prevText = testElement->prevText;
  result->origText = testElement->origText;
  result->next = NULL;
  filler = result;
  return GOOD;
}

int PatternItem::MinLength()
{
  return 1;
}


// PatternItem internal routines

void PatternItem::WriteConstraints(ostream& stream)
{
  WordList* i = NULL;
  int doingList;
  // write m_Word
  if (!m_Word) {
    stream << "_,";
  } else {
    if (m_Word->next) {
      doingList = 1;
      stream << '[';
    } else {
      doingList = 0;
    }
    i = m_Word;
    while (i) {
      stream << "'";
      int j;
      int wordLen = strlen(i->word);
      for ( j = 0; j < wordLen; j++) {
	stream << i->word[j];
	if (i->word[j] == 39) {
	  stream << i->word[j];
	}
      }
      stream << "'";
      i = i->next;
      if (i)
	stream << ',';
    }
    if (doingList)
      stream << ']';
    stream << ',';
  }
  
  // write m_Tag
  if (!m_Tag) {
    stream << "_,";
  } else {
    if (m_Tag->next) {
      doingList = 1;
      stream << '[';
    } else {
      doingList = 0;
    }
    i = m_Tag;
    while (i) {
      stream << "'" << i->word << "'";
      i = i->next;
      if (i)
	stream << ',';
    }
    if (doingList)
      stream << ']';
    stream << ',';
  }

  // write m_Sem
  if (!m_Sem) {
    stream << "_,";
  } else {
    if (m_Sem->next) {
      doingList = 1;
      stream << '[';
    } else {
      doingList = 0;
    }
    i = m_Sem;
    while (i) {
      stream << "'" << i->word << "'";
      i = i->next;
      if (i)
	stream << ',';
    }
    if (doingList)
      stream << ']';
    stream << ',';
  }

  // write m_WordNot
  if (!m_WordNot) {
    stream << "_,";
  } else {
    if (m_WordNot->next) {
      doingList = 1;
      stream << '[';
    } else {
      doingList = 0;
    }
    i = m_WordNot;
    while (i) {
      stream << "'" << i->word << "'";
      i = i->next;
      if (i)
	stream << ',';
    }
    if (doingList)
      stream << ']';
    stream << ',';
  }

  // write m_TagNot
  if (!m_TagNot) {
    stream << "_,";
  } else {
    if (m_TagNot->next) {
      doingList = 1;
      stream << '[';
    } else {
      doingList = 0;
    }
    i = m_TagNot;
    while (i) {
      stream << "'" << i->word << "'";
      i = i->next;
      if (i)
	stream << ',';
    }
    if (doingList)
      stream << ']';
    stream << ',';
  }

  // write m_SemNot
  if (!m_SemNot) {
    stream << "_";
  } else {
    if (m_SemNot->next) {
      doingList = 1;
      stream << '[';
    } else {
      doingList = 0;
    }
    i = m_SemNot;
    while (i) {
      stream << "'" << i->word << "'";
      i = i->next;
      if (i)
	stream << ',';
    }
    if (doingList)
      stream << ']';
  }
}


// PatternList routines

PatternList::PatternList()
{ 
  // empty constructor
}

PatternList::PatternList(int len, WordList* word, WordList* tag, WordList* sem,
			 WordList* wordNot, WordList* tagNot,
			 WordList* semNot) : PatternItem(word, tag, sem,
							 wordNot, tagNot,
							 semNot)
{
  m_nLimit = len;
}

PatternList::PatternList(PatternList* oldList) : PatternItem(oldList)
{
  m_nLimit = oldList->GetListLimit();
}


PatternList::~PatternList()
{
  // empty destructor
}

int PatternList::CalcSize(int isFiller)
{
  return 1 + PatternItem::CalcSize(isFiller);
}

int PatternList::MinLength()
{
  return 0;
}


void PatternList::Read(istream& stream, WordStore& ws) 
{
  //  We need to read the number and the comma, then call PatternItem::Read()
  
  int num;
  char c;

  stream >> num;

  m_nLimit = num;
  stream.get(c);
  while (c != ',') {
    stream.get(c);
  }

  PatternItem::Read(stream,ws);
}

void PatternList::Write(ostream& stream)
{
  stream << "list(" << m_nLimit << ",";
  this->WriteConstraints(stream);
  stream << ")";
}


/*int PatternList::Match(Document* doc, int retry)
{
  WordList* junk=NULL;
  int result;
  WordList* i = NULL;
  result = this->MatchFiller(doc,junk, retry);
  while (junk) {
    i = junk;
    junk = junk->next;
    delete i;
    i = NULL;
  }
  return result;
}*/

int PatternList::Match(Document* doc, int retry)
{
  DocWord*  testElement = NULL;
  int       goodMatch;
  int       semMatch;
  WordList* i;

  if (!retry) {
    // this is the first try to match this list
    m_nLastTryLength = 0;
    // we have to notify the doc that it's at a restart point
    doc->SaveIterState();
    return RESTARTPOINT;
  } else { // this is a retry
    // we have to restore the document state
    if (m_nLastTryLength+1 <= m_nLimit) {
      doc->UseSavedIterState();
    } else {
      doc->RestoreIterState();
    }
    // for 0 to m_nLastTryLength, no need to check
    // it, because you already know it's good
    if (m_nLastTryLength > 0) {
      testElement = doc->GetNextWord();
      if (!testElement) {
	//	cout << "testElement NULL collecting filler\n";
	//	cout << "m_nLastTryLength: " << m_nLastTryLength << "\n";
	return ENDOFDOC;
      }
    }
    for (int j = 1; j < m_nLastTryLength; j++) {
      testElement = doc->GetNextWord();
      if (!testElement) {
	if (m_nLastTryLength+1 >= m_nLimit) {
	  doc->PopIterState();
	}
	return ENDOFDOC;
      }
    }

    // now we need to test the next word
    testElement = doc->GetNextWord();
      if (!testElement) {
	if (m_nLastTryLength+1 >= m_nLimit) {
	  doc->PopIterState();
	}
	return ENDOFDOC;
      }
    if (m_Word) {
    goodMatch = 0;
    i = m_Word;
    while (i && !goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      return FAILED;
    } else {
      // word Matches; do nothing but drop to next test
    }
  } else {
    // word Matches; do nothing but drop to next test
  }
  if (m_Tag) {
    goodMatch = 0;
    i = m_Tag;
    while (i && !goodMatch) {
      if (i->word == testElement->theTag) {
        goodMatch = 1;
      } else {
        i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
    } else {
      // tag matches; do nothing but drop to next test
    }
  } else {
    // tag matches; do nothing but drop to next test
  }
  if (m_WordNot) {
    goodMatch= 1;
    i = m_WordNot;
    while (i && goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      return FAILED;
    } else {
      // WordNot matches; do nothing but drop to next test
    }
  } else {
    // WordNot matches; do nothing but drop to next test
  }
  if (m_TagNot) {
    goodMatch= 1;
    i = m_TagNot;
    while (i && goodMatch) {
      if (i->word == testElement->theTag) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      return FAILED;
    } else {
      // TagNot matches; do nothing but drop to next test
    }    
  } else {
    // TagNot matches; do nothing but drop to next test
  }
  if (m_Sem) {
    goodMatch = 0;
    i = m_Sem;
    while (i && !goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit)
	doc->PopIterState();
      return FAILED;
    } else {
      // Sem matches; do nothing but drop to next test
    }
  } else {
    // Sem matches; do nothing but drop to next test
  }
  if (m_SemNot) {
    goodMatch = 1;
    i = m_SemNot;
    while (i && goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      return FAILED;
    } else {
      // SemNot matches; do nothing but drop to next test
    }
  } else {
    // SemNot matches, too
  }
  m_nLastTryLength++;
  if (m_nLastTryLength < m_nLimit)
    return RESTARTPOINT;
  else
    return GOOD;
  }
}


int PatternList::MatchFiller(Document* doc,WordList*& filler, int retry)
{
  WordList* result = NULL;
  WordList* lastWord = NULL;
  DocWord*  testElement = NULL;
  int       goodMatch;
  WordList* i = NULL;
  int       semMatch;

  if (!retry) {
    // this is the first try to match this list
    m_nLastTryLength = 0;
    filler = NULL;
    // we have to notify the doc that it's at a restart point
    doc->SaveIterState();
    return RESTARTPOINT;
  } else { // this is a retry
    // we have to restore the document state
    doc->RestoreIterState();
    // then if this is a restart point notify the document
    if (m_nLastTryLength+1 <= m_nLimit) {
      doc->SaveIterState();
    }
    // for 0 to m_nLastTryLength, you just collect the filler; no need to check
    // it, because you already know it's good
    if (m_nLastTryLength > 0) {
      testElement = doc->GetNextWord();
      if (!testElement) {
	//	cout << "testElement NULL collecting filler\n";
	//	cout << "m_nLastTryLength: " << m_nLastTryLength << "\n";
	return ENDOFDOC;
      }
      result = new WordList;
      result->word = testElement->theWord;
      result->prevText = testElement->prevText;
      result->origText = testElement->origText;
      result->next = NULL;
      filler = result;
      lastWord = result;
    }
    for (int j = 1; j < m_nLastTryLength; j++) {
      testElement = doc->GetNextWord();
      if (!testElement) {
	if (m_nLastTryLength+1 >= m_nLimit) {
	  doc->PopIterState();
	}
	while (filler) {
	  i = filler;
	  filler = filler->next;
	  delete i;
	  i = NULL;
	}
	return ENDOFDOC;
      }
      result = new WordList;
      result->word = testElement->theWord;
      result->prevText = testElement->prevText;
      result->origText = testElement->origText;
      result->next = NULL;
      lastWord->next = result;
      lastWord = result;
    }

    // now we need to test the next word
    testElement = doc->GetNextWord();
      if (!testElement) {
	if (m_nLastTryLength+1 >= m_nLimit) {
	  doc->PopIterState();
	}
	while (filler) {
	  i = filler;
	  filler = filler->next;
	  delete i;
	  i = NULL;
	}
	return ENDOFDOC;
      }
    if (m_Word) {
    goodMatch = 0;
    i = m_Word;
    while (i && !goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      while (filler) {
	i = filler;
	filler = filler->next;
	delete i;
	i = NULL;
      }
      return FAILED;
    } else {
      // word Matches; do nothing but drop to next test
    }
  } else {
    // word Matches; do nothing but drop to next test
  }
  if (m_Tag) {
    goodMatch = 0;
    i = m_Tag;
    while (i && !goodMatch) {
      if (i->word == testElement->theTag) {
        goodMatch = 1;
      } else {
        i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      while (filler) {
	i = filler;
	filler = filler->next;
	delete i;
	i = NULL;
      }
      return FAILED;
    } else {
      // tag matches; do nothing but drop to next test
    }
  } else {
    // tag matches; do nothing but drop to next test
  }
  if (m_WordNot) {
    goodMatch= 1;
    i = m_WordNot;
    while (i && goodMatch) {
      if (i->word == testElement->theWord) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      while (filler) {
	i = filler;
	filler = filler->next;
	delete i;
	i = NULL;
      }
      return FAILED;
    } else {
      // WordNot matches; do nothing but drop to next test
    }
  } else {
    // WordNot matches; do nothing but drop to next test
  }
  if (m_TagNot) {
    goodMatch= 1;
    i = m_TagNot;
    while (i && goodMatch) {
      if (i->word == testElement->theTag) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      while (filler) {
	i = filler;
	filler = filler->next;
	delete i;
	i = NULL;
      }
      return FAILED;
    } else {
      // TagNot matches; do nothing but drop to next test
    }    
  } else {
    // TagNot matches; do nothing but drop to next test
  }
  if (m_Sem) {
    goodMatch = 0;
    i = m_Sem;
    while (i && !goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 1;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit)
	doc->PopIterState();
      while (filler) {
	i = filler;
	filler = filler->next;
	delete i;
	i = NULL;
      }
      return FAILED;
    } else {
      // Sem matches; do nothing but drop to next test
    }
  } else {
    // Sem matches; do nothing but drop to next test
  }
  if (m_SemNot) {
    goodMatch = 1;
    i = m_SemNot;
    while (i && goodMatch) {
      if (sem_class(testElement->theWord,i->word)) {
	goodMatch = 0;
      } else {
	i = i->next;
      }
    }
    if (!goodMatch) {
      if (m_nLastTryLength+1 >= m_nLimit) {
	doc->PopIterState();
      }
      while (filler) {
	i = filler;
	filler = filler->next;
	delete i;
	i = NULL;
      }
      return FAILED;
    } else {
      // SemNot matches; do nothing but drop to next test
    }
  } else {
    // SemNot matches, too
  }
  // Now we need to tack the testElement onto the filler, increment the
  // m_nLastTryLength and determine which code to return
  result = new WordList;
  result->word = testElement->theWord;
  result->prevText = testElement->prevText;
  result->origText = testElement->origText;
  result->next = NULL;
  if (lastWord) {
    lastWord->next = result;
  } else {
    filler = result;
  }
  m_nLastTryLength++;
  if (m_nLastTryLength < m_nLimit)
    return RESTARTPOINT;
  else
    return GOOD;
  }
}


// Pattern routines

int Pattern::GetMinLength()
{
  int len = this->GetLength();
  PatternItem* item = NULL;
  int sum = 0;

  for (int i = 0; i < len; i++) {
    item = (PatternItem *) m_PatternArray->GetItem(i);
    if (item) {
      sum = sum + item->MinLength();
    }
  }  
  return sum;
}

Pattern::Pattern() 
{
  gNumPatterns++;
  m_PatternArray = new DynamicPtrArray;
  if (!m_PatternArray) {
    cout << "!!! ERROR !!! failed to create new PatternArray\n";
  }
  m_RetryStack = NULL;
}


Pattern::Pattern(int maxSize) 
{
  gNumPatterns++;
  m_PatternArray = new DynamicPtrArray(maxSize);
  if (!m_PatternArray) {
    cout << "!!! ERROR !!! failed to create new PatternArray\n";
  }
  m_RetryStack = NULL;
}

Pattern::Pattern(Pattern* oldPattern)
{
  gNumPatterns++;
  m_RetryStack = NULL;
  m_PatternArray = new DynamicPtrArray();
  if (!m_PatternArray) {
    cout << "!!! ERROR !!! failed to create new PatternArray\n";
  }
  int len = oldPattern->GetLength();
  for (int i = 0; i < len; i++) {
    PatternItem* oldItem = oldPattern->GetPatternItem(i);
    if (1 == oldItem->GetType()) {
      // we have an actual Item
      PatternItem* newItem = new PatternItem(oldItem);
      m_PatternArray->Add(newItem);
    } else {
      // we have a PatternList
      PatternList* newList = new PatternList((PatternList*) oldItem);
      m_PatternArray->Add(newList);
    }
  }
}

Pattern::~Pattern()
{
  gNumPatterns--;
  // first, delete all of the elements in the pattern array, then
  // delete the array itself

  if (m_RetryStack) {
    PtnRetryPt* a;
    PtnRetryPt* b;
    a = m_RetryStack;
    while (a) {
      b = a->next;
      delete a;
      a = b;
    }
  }

  int len = this->GetLength();
  PatternItem* item = NULL;

  for (int i = 0; i < len; i++) {
    item = (PatternItem *) m_PatternArray->GetItem(i);
    delete item;
    item = NULL;
  }  

  delete m_PatternArray;
  m_PatternArray = NULL;
}

void Pattern::AddToSuffix(Pattern* suffixAddition)
{
  int len = suffixAddition->GetLength();
  for (int i = 0; i < len; i++) {
    PatternItem* oldItem = suffixAddition->GetPatternItem(i);
    if (1 == oldItem->GetType()) {
      // we have an actual Item
      PatternItem* newItem = new PatternItem(oldItem);
      m_PatternArray->Add(newItem);
    } else {
      // we have a PatternList
      PatternList* newList = new PatternList((PatternList*) oldItem);
      m_PatternArray->Add(newList);
    }
  }
}


void Pattern::AddToPrefix(Pattern* prefixAddition)
{
  DynamicPtrArray* oldArray = m_PatternArray;
  m_PatternArray = new DynamicPtrArray;
  int len = prefixAddition->GetLength();
  int i;
  for (i = 0; i < len; i++) {
    PatternItem* oldItem = prefixAddition->GetPatternItem(i);
    if (1 == oldItem->GetType()) {
      // we have an actual Item
      PatternItem* newItem = new PatternItem(oldItem);
      m_PatternArray->Add(newItem);
    } else {
      // we have a PatternList
      PatternList* newList = new PatternList((PatternList*) oldItem);
      m_PatternArray->Add(newList);
    }
  }
  len = oldArray->GetSize();
  for (i = 0; i < len; i++) {
    m_PatternArray->Add(oldArray->GetItem(i));
  }
  delete oldArray;
}

void Pattern::PrefixFromDoc(Document* doc, int fillerStart)
{
  // let's try doing only the 50 words in the immediate context
  int docStart = fillerStart -50;
  int j;
  if (docStart <= 0) {
    j = 0;
    doc->InitializeIterator();
  } else {
    j = docStart;
    doc->ResetIterator(docStart);
  }
  DocWord* i;
  i = doc->GetNextWord();
  while (j < fillerStart) {
    PatternItem* newItem = new PatternItem(i->theWord,i->theTag);
    m_PatternArray->Add(newItem);
    j++;
    i = doc->GetNextWord();
  }
}

void Pattern::FillerPattern(Filler* filler,Document* doc, int startPoint)
{
  doc->ResetIterator(startPoint);
  DocWord* j;
  j = doc->GetNextWord();
  WordList* i = filler->filler;
  while (i) {
    PatternItem* newItem = new PatternItem(j->theWord, j->theTag);
    m_PatternArray->Add(newItem);
    i = i->next;
    j = doc->GetNextWord();
  }
}

void Pattern::SuffixFromDoc(Document* doc, int startPoint)
{
  doc->ResetIterator(startPoint);
  DocWord* i;
  i = doc->GetNextWord();
  // let's try limiting the suffix to no more than 50 words
  int j = 0;
  while (i && (j < 50)) {
    j++;
    PatternItem* newItem = new PatternItem(i->theWord,i->theTag);
    m_PatternArray->Add(newItem);
    i = doc->GetNextWord();
  }
}


void Pattern::Read(istream& stream, WordStore& ws)
{
  //  We are, by definition, reading a list
  //  assume that the comma preceding the pattern has been read, but not
  //  the bracket beginning the list

  char     c;
  PatternItem* newItem = NULL;
  
  stream.get(c);
  while (c != '[') 
    stream.get(c);
  while (c != ']') { // this is the main loop of the read
    while ((c != 'i') && (c != 'l')  && (c != ']')) 
      stream.get(c);
    if (c != ']') {
    if (c == 'i') {
      newItem = new PatternItem;
    } else { 
      newItem = new PatternList;
    }
    while (c != '(')
      stream.get(c);
    newItem->Read(stream,ws);
    m_PatternArray->Add((void *)newItem);
    while ((c != ',') && (c != ']'))
      stream.get(c);
    }
  }
}


void Pattern::Write(ostream& stream)
{
  stream << "\n[";

  int size = this->GetLength();


  PatternItem* item = NULL;
  if (!m_PatternArray) cout << "m_PatternArray is NULL\n";
  
  for (int i = 0; i < (size - 1); i++) {
    item = (PatternItem *) m_PatternArray->GetItem(i);
    if (!item) cout << "item pointer is NULL\n";
    item->Write(stream);
    stream << ',';
    if (0 == (i % 2))
      stream << '\n';
  }
  if (size > 0) {
    if (!m_PatternArray) cout << "m_PatternArray is NULL\n";
  item = (PatternItem *) m_PatternArray->GetItem(size - 1);
  if (!item) cout << "item pointer is NULL\n";
  item->Write(stream);
  }
  stream << "]";
}

int Pattern::CalcSize(int isFiller)
{
  int size = 0;
  int len = this->GetLength();
  PatternItem* item = NULL;

  for (int i = 0; i < len; i++) {
    item = (PatternItem *) m_PatternArray->GetItem(i);
    size = size + item->CalcSize(isFiller);    
  }
  return size;
}

int Pattern::Match(Document* doc, int retry)
{
  int len = this->GetLength();
  int i; // pattern array index;
  PtnRetryPt* tempPt = NULL;
  int retryFlag;
  int returnCode;
  int loopCont = 1;
  int result;
  // this is just matching, so we don't need to worry about fillers
  // first, let's see whether we're retrying
  if (retry) {
    if (!m_RetryStack)
      return FAILED;
    else {
      // pop the top of the RetryStack
      tempPt = m_RetryStack;
      m_RetryStack = m_RetryStack->next;
      i = tempPt->arrayIndex;
      delete tempPt;
      tempPt = NULL;
      retryFlag = 1;
    }
  } else {
    // clear out the retry stack
    PtnRetryPt* k = m_RetryStack;
    while (k) {
      m_RetryStack = m_RetryStack->next;
      delete k;
      k = m_RetryStack;
    }
    i = 0;
    retryFlag = 0;
  }
  PatternItem* item = NULL;
  
  while (loopCont && (i < len)) {
    item = (PatternItem *) m_PatternArray->GetItem(i);
    result = item->Match(doc,retryFlag);
    switch (result) {
    case RESTARTPOINT :
      tempPt = new PtnRetryPt;
      tempPt->arrayIndex = i;
      tempPt->fillerSoFar = NULL;
      tempPt->next = m_RetryStack;
      m_RetryStack = tempPt;
    case GOOD :
      i++;
      retryFlag = 0;
      break;
    case FAILED :
    case ENDOFDOC :
      if (m_RetryStack) {
	// pop the top of the RetryStack
	tempPt = m_RetryStack;
	m_RetryStack = m_RetryStack->next;
	i = tempPt->arrayIndex;
	delete tempPt;
	tempPt = NULL;
	retryFlag = 1;
      } else {
	return result;
      }
    }
  } 
  if (m_RetryStack) 
    return RESTARTPOINT;
  else
    return GOOD;
}

int Pattern::MatchFiller(Document* doc, WordList*& filler, int retry)
{
  int len = this->GetLength();
  int i; // pattern array index;
  PtnRetryPt* tempPt = NULL;
  int retryFlag;
  int returnCode;
  int loopCont = 1;
  int result;
  WordList* tempWord = NULL;
  WordList* tempWord2 = NULL;
  // this time we have to worry about fillers
  // first, let's see whether we're retrying
  if (retry) {
    if (!m_RetryStack) {
      filler = NULL;
      return FAILED;
    } else {
      // pop the top of the RetryStack
      tempPt = m_RetryStack;
      m_RetryStack = m_RetryStack->next;
      i = tempPt->arrayIndex;
      filler = tempPt->fillerSoFar;
      delete tempPt;
      tempPt = NULL;
      retryFlag = 1;
    }
  } else {
    i = 0;
    retryFlag = 0;
  }
  PatternItem* item = NULL;
  while (loopCont && (i < len)) {
    item = (PatternItem *) m_PatternArray->GetItem(i);
    tempWord = NULL;
    result = item->MatchFiller(doc,tempWord,retryFlag);
    switch (result) {
    case RESTARTPOINT :
      tempPt = new PtnRetryPt;
      tempPt->arrayIndex = i;
      // copy former filler into tempPt->fillerSoFar
      if (!filler) {
	tempPt->fillerSoFar = NULL;
      } else {
	tempPt->fillerSoFar = CopyWordList(filler);
      }
      tempPt->next = m_RetryStack;
      m_RetryStack = tempPt;
    case GOOD :
      i++;
      retryFlag = 0;
      if (!filler) {
	filler = tempWord;
      } else {
	tempWord2 = filler;
	while (tempWord2->next) {
	  tempWord2 = tempWord2->next;
	}
	tempWord2->next = tempWord;
      }
      break;
    case FAILED :
    case ENDOFDOC :
      // clear out the current filler
      while (filler) {
	tempWord2 = filler;
	filler = filler->next;
	delete tempWord2;
	tempWord2 = NULL;
      }
      if (m_RetryStack) {	
	// pop the top of the RetryStack
	tempPt = m_RetryStack;
	m_RetryStack = m_RetryStack->next;
	i = tempPt->arrayIndex;
	filler = tempPt->fillerSoFar;
	delete tempPt;
	tempPt = NULL;
	retryFlag = 1;
      } else {
	return result;
      }
    }
  } 
  if (m_RetryStack) 
    return RESTARTPOINT;
  else
    return GOOD;
}




// Rule routines

Rule::Rule()
{
  gNumRules++;
  m_TemplateName = NULL;
  m_SlotName = NULL;
  m_nPosCovered = 0;
  m_nNegCovered = 0;
  m_PrefixPattern = NULL;
  m_FillerPattern = NULL;
  m_SuffixPattern = NULL;
  m_nSize = 0;
  m_CovList = NULL;
}

Rule::Rule(char* templateName, char* slotName)
{
  gNumRules++;
  m_TemplateName = templateName;
  m_SlotName = slotName;
  m_nPosCovered = 0;
  m_nNegCovered = 0;
  m_PrefixPattern = NULL;
  m_FillerPattern = NULL;
  m_SuffixPattern = NULL;
  m_nSize = 0;
  m_CovList = NULL;
}

Rule::Rule(Rule* orig) 
{
  gNumRules++;
  m_TemplateName = orig->GetTemplateName();
  m_SlotName = orig->GetSlotName();
  m_nSize = orig->GetRuleSize();
  m_nPosCovered = orig->GetPosCovered();
  m_nNegCovered = orig->GetNegCovered();
  m_PrefixPattern = new Pattern(orig->GetPrefixPattern());
  m_FillerPattern = new Pattern(orig->GetFillerPattern());
  m_SuffixPattern = new Pattern(orig->GetSuffixPattern());
  m_CovList = orig->GetCovList();
}

Rule::~Rule()
{
  gNumRules--;
  delete m_PrefixPattern;
  m_PrefixPattern = NULL;
  delete m_FillerPattern;
  m_FillerPattern = NULL;
  delete m_SuffixPattern;
  m_SuffixPattern = NULL;
  //  delete m_CovList;
  CoversNode* a = m_CovList;
  CoversNode* b;
  while (a) {
    b = a->next;
    delete a;
    a = b;
  }
  m_CovList = NULL;
}

void Rule::CreateFromDoc(Document* doc,Filler *filler,int startWord)
{
  m_nPosCovered = 1;
  m_nNegCovered = 0;
  m_PrefixPattern = new Pattern();
  m_PrefixPattern->PrefixFromDoc(doc,startWord);
  m_FillerPattern = new Pattern();
  m_FillerPattern->FillerPattern(filler,doc,startWord);
  m_SuffixPattern = new Pattern();
  // find start of suffix
  WordList* i = filler->filler;
  while (i) {
    startWord++;
    i = i->next;
  }
  m_SuffixPattern->SuffixFromDoc(doc,startWord);
  CoversNode* newNode = new CoversNode;
  newNode->docCovered = doc;
  newNode->fillerCovered = filler->filler;
  newNode->next = m_CovList;
  m_CovList = newNode;
}
  
void Rule::Read(istream& stream, WordStore& ws) 
{
  // We need to find the beginning of the rule and read to the parenthesis
  char c;
  char buffer[80];
  int  i;

  stream.get(c);
  while (c != 'r')
    stream.get(c);
  while (c != '(')
    stream.get(c);
  stream.get(c); // move one beyond the parenthesis
  while (c == ' ')
    stream.get(c);
  // Now read the template name for the rule
  i = 0;
  // assume single word unquoted
  while (c != ',') {
    buffer[i++] = c;
    stream.get(c);
  }
  buffer[i] = 0;
  m_TemplateName = ws.StoreWord(buffer);
  
  // Now read the slot name for the rule
  stream.get(c);
  while ((c == ' ') || (c == 10) || ( c == 13)) 
    stream.get(c);
  i = 0;
  while (c != ',') {
    buffer[i++] = c;
    stream.get(c);
  }
  buffer[i] = 0;
  m_SlotName = ws.StoreWord(buffer);
  
  //Now read the number of positives covered
  stream >> m_nPosCovered;
  stream.get(c);
  while (c != ',') {
    stream.get(c);
  }
  // Now read the number of negatives covered
  stream >> m_nNegCovered;
  stream.get(c);
  while (c != ',') {
    stream.get(c);
  }
  
  // create and read the prefix pattern
  Pattern* aPattern = new Pattern;
  aPattern->Read(stream,ws);
  m_PrefixPattern = aPattern;

  // OK, we've read the right bracket, but not beyond
  stream.get(c);
  while (c != ',')
    stream.get(c);
  // create and read the filler pattern
  aPattern = new Pattern;
  aPattern->Read(stream,ws);
  m_FillerPattern = aPattern;

  stream.get(c);
  while (c != ',')
    stream.get(c);
  // create and read the suffix pattern
  aPattern = new Pattern;
  aPattern->Read(stream,ws);
  m_SuffixPattern = aPattern;
  
  stream.get(c);
  while (c != ')')
    stream.get(c);
  while (c != '.')
    stream.get(c);
  
}


void Rule::Write(ostream& stream)
{
  stream << "rule(";
  stream << m_TemplateName;
  stream << ", ";
  stream << m_SlotName;
  stream << ", ";
  stream << m_nPosCovered;
  stream << ", ";
  stream << m_nNegCovered;
  stream << ", ";
  m_PrefixPattern->Write(stream);
  stream << ",";
  m_FillerPattern->Write(stream);
  stream << ",";
  m_SuffixPattern->Write(stream);
  stream << ").\n";
}


int Rule::CalcSize()
{
  m_nSize = m_PrefixPattern->CalcSize(0) + m_FillerPattern->CalcSize(1) +
    m_SuffixPattern->CalcSize(0);
  return m_nSize;
}

int Rule::Match(Document* doc, Filler*& fillers, int findall)
{
  // What should this control structure look like
  // Should I go ahead and implement the min length check?
  fillers = NULL;
  int remDocLength = doc->Length();
  int ruleLength = m_PrefixPattern->GetMinLength() +
                   m_FillerPattern->GetMinLength() +
                   m_SuffixPattern->GetMinLength();
  int contLoop = 1;
  int result;
  int prefixRestartable;
  int fillerRestartable;
  int prefixBad;
  Filler* lastFiller = NULL;
  Filler* fillerPtr = NULL;
  WordList* i = NULL;
  WordList* j = NULL;
  WordList* curFiller = NULL;;
  fillers = NULL;
  doc->InitializeIterator();
  doc->SaveIterState();
  result = m_PrefixPattern->Match(doc,0);
  switch (result) {
  case GOOD :
    prefixRestartable = 0;
    prefixBad = 0;
    break;
  case RESTARTPOINT :
    prefixRestartable = 1;
    prefixBad = 0;
    break;
  default :
    prefixRestartable = 0;
    prefixBad = 1;
    remDocLength--;
  }
  int needFiller;

  while (contLoop && (remDocLength >= ruleLength)) {
    // get a good prefix match
    while (prefixBad && (remDocLength >= ruleLength)) {
      if (prefixRestartable) {
	result= m_PrefixPattern->Match(doc,1);
      } else {
	doc->RestoreAdvanceAndSaveIterState();
	result = m_PrefixPattern->Match(doc,0);
      }
      switch (result) {
      case GOOD :
	prefixRestartable = 0;
	prefixBad = 0;
	break;
      case RESTARTPOINT :
	prefixRestartable = 1;
	prefixBad = 0;
	break;
      default :
	prefixBad = 1;
	prefixRestartable = 0;
	remDocLength--;
      }
    } // while prefixBad ...
    // Now we need to try to pick up a filler
    fillerRestartable = 0;
    needFiller = 1;
    while (needFiller && !prefixBad) {
      result = m_FillerPattern->MatchFiller(doc,curFiller,fillerRestartable);
      switch (result) {
      case GOOD :
	fillerRestartable = 0;
	needFiller = 0;
	break;
      case RESTARTPOINT :
	fillerRestartable = 1;
	needFiller = 0;
	break;
      default :
	// the filler failed, so we need to go back to the prefix
	fillerRestartable = 0;
	prefixBad = 1;
	needFiller = 1;
	remDocLength--;
	break;
      }
      if (!needFiller) {
	Filler* f;
	int isNew;

	// if we actually got a good filler, we need to check the suffix
	result = m_SuffixPattern->Match(doc,0);
	switch (result) {
	case GOOD :
	case RESTARTPOINT :
	  // OK, we have a good filler in curFiller
	  // let's put it into fillers
	  f = fillers;
          isNew = 1;
          while (f && isNew) {
            if (EqualFillers(f->filler,curFiller))
              isNew = 0;
            f = f->next;
          }
          if (isNew) {
	    lastFiller = new Filler;
	    lastFiller->filler = curFiller;
	    lastFiller->next = fillers;
	    fillers = lastFiller;
          } else {
            // need to delete curFiller
            while (curFiller) {
              i = curFiller;
              curFiller = curFiller->next;
              delete i;
              i = NULL;
            }
	  }
	  // then let's see whether or not we need to collect more fillers
          curFiller = NULL;
	  lastFiller = NULL;
	  if (findall) { 
	    // we need to find all the fillers we can
	    needFiller = 1;
	    // trying something
	    //	    if (fillerRestartable) {
	    //fillerRestartable = 0;
	    //doc->PopIterState();
	    //}
	    if (!fillerRestartable && !prefixRestartable) {
	      doc->SaveSingleIterState();
	      remDocLength--;
	    }
	    if (! fillerRestartable)
	      prefixBad = 1;
	    // end new code
	  } else {
	    // just needed one filler, so we're all done
	    contLoop = 0;
	  }
	  break;
	default :
	  // the suffix was bad
	  // first, free curFiller
	  while (curFiller) {
	    i = curFiller;
	    curFiller = curFiller->next;
	    delete i;
	    i = NULL;
	  }
	  // then set flags to redo 
	  needFiller = 1;
	  if (!fillerRestartable) {
	    prefixBad = 1;
	    if (!prefixRestartable)
	      remDocLength--;
	  }
	}
      }
    }
  }

  if (fillers) {
    return GOOD;
  } else {
    return FAILED;
  }
}


int Rule::CheckCoverage(CoversNode* covCompare) 
{
  // for each node in m_CovList; check to see if it is in covCompare
  // if all are in covCompare, return TRUE
  int result = 1;
  int tempResult = 0;
  CoversNode* a = m_CovList;
  while (result && a) {
    tempResult = 0;
    CoversNode* b = covCompare;
    while (!tempResult && b) {
      if ((a->docCovered == b->docCovered) && (a->fillerCovered == b->fillerCovered)) {
	tempResult = 1;
      }
      b = b->next;
    }
    if (tempResult == 0) {
      result = 0;
    }
    a = a->next;
  }
  return result;
}
