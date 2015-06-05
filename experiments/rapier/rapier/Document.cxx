//-*-C++-*-

#include <iostream>
#include <string.h>
#include <malloc.h>
#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"


#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

Document::Document()
{
  m_WordList = (DocWord*) malloc(200 * sizeof(DocWord));
  m_nAllocatedSize = 200;
  m_DocLength = 0;
  m_IterPtr = -1;
  m_nSavedIterTop = -1;
  m_SavedIters = (DocIterPtr*) malloc(k_iterStackSize * sizeof(DocWord));
  m_nIterStackSize = k_iterStackSize;
}

Document::~Document()
{
  free(m_WordList);
}

void Document::Read(istream& stream, istream& origStream, WordStore& ws) 
{
  // OK, we've got to read the document in from stream, ending at eof
  // we're making the assumption that this is tagged text where each
  // word is word/tag and both may have capital letters (so we're going
  // to lower case them
  // we're also assuming that each sentence is on a separate line
  // since that's a requirement of the tagger
  // At the same time we need to read the text from the original document

  char c, nextc;
  char origC;
  char buffer[300];
  char origBuffer[500];
  DocWord* wtpair = NULL;
  char* word = NULL;
  char* tag = NULL;
  char* prevText = NULL;
  char* origText = NULL;
  int   i;  
  int   origI;
  char  useChar;
  int   cont;

  stream.get(c);
  origStream.get(origC);
  while (stream.good()) {
    // read word/tag pairs
    switch (c) {
    case 10:
    case 13:
      word = ws.StoreWord((char*)"endsent");
      tag = word;
      wtpair = new DocWord;
      wtpair->theWord = word;
      wtpair->theTag = tag;
      wtpair->prevText = prevText;
      wtpair->origText = origText;
      this->AddWord(wtpair);
      stream.get(c);
      break;
    case 32:
    case 9:
      stream.get(c);
      break;
    default:
      // read in a word and tag
      // first, read in the prevText
      //      origStream.get(origC);
      origI = 0;
      while (c != origC) {
	if ((origC == 10) || (origC == 13) || (origC == 9)) {
	  origC = ' ';
	}
	if ((origI > 0) && (origBuffer[origI - 1] == ' ') && (origC == ' ')) {
	  origStream.get(origC);
	} else {
	  origBuffer[origI] = origC;
	  origI++;
	  if (origI >=500) {
	    cout << "origI greater than 500\n";
	    cout << "buffer: " << buffer << "\n";
	    cout << "origBuffer: " << origBuffer << "\n";
	  }
	  origStream.get(origC);
	}
      }
      origBuffer[origI] = 0;
      origI++;
      if (origI >=500) {
	cout << "origI greater than 500\n";
      }
      prevText = (char*) malloc(origI * sizeof(char));
      prevText = strcpy(prevText,origBuffer);
      origI = 0;
      i = 0;
      cont = 1;
      while (cont) {
	if (c == '/') {
	  stream.get(nextc);
	  if (nextc != '/') {
	    cont = 0;
	  } 
	  stream.putback(nextc);
	};
	if (cont) {
	  origBuffer[origI] = origC;
	  origI++;
	  if (origI >=500) {
	    cout << "origI greater than 500\n";
	  }
	  origStream.get(origC);
	  useChar = lowercase(c);
	  buffer[i] = useChar;
	  i++;
	  if (i >= 500) {
	    cout << "i greater than 500\n";
	  }
	  stream.get(c);
	}
      }
      // finished with word
      origBuffer[origI] = 0;
      origI++;
      origText = (char*) malloc(origI * sizeof(char));
      origText = strcpy(origText,origBuffer);
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      i = 0;
      stream.get(c); // throw away '/' separating word and tag
      cont = 1;
      // now read tag
      while (cont) {
	if ((c == 32) || (c == 10) || (c == 13)) {
	  cont = 0;
	} else {
	  useChar = lowercase(c);
	  buffer[i] = useChar;
	  i++;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      tag = ws.StoreWord(buffer);
      wtpair = new DocWord;
      wtpair->theWord = word;
      wtpair->theTag = tag;
      wtpair->prevText = prevText;
      wtpair->origText = origText;
      this->AddWord(wtpair);
    } // switch
  } // while
  
  
} // Read


void Document::Write(ostream& stream)
{
  DocWord curWord;

  for (int i = 0; i < m_DocLength; i++) {
    curWord = m_WordList[i];
    stream << curWord.theWord;
    stream << '/';
    stream << curWord.theTag;
    stream << ' ';
  }
}

void Document::WriteFromIter(ostream& stream, int wordsToWrite)
{
  DocWord curWord;
  int i = m_IterPtr;
  int lim = m_IterPtr + wordsToWrite;
  if (lim > m_DocLength) lim = m_DocLength;
  
  while (i < lim) {
    curWord = m_WordList[i];
    stream << curWord.theWord;
    stream << '/';
    stream << curWord.theTag;
    stream << ' ';
    i++;
  }
  stream << "\n";
}

// Document iterator routines

void Document::InitializeIterator()
{
  m_IterPtr = 0;
  m_nSavedIterTop = -1;
}

DocWord* Document::GetNextWord()
{
  if ((m_IterPtr >= 0) && (m_IterPtr < m_DocLength)) {
    DocWord* temp = &(m_WordList[m_IterPtr]);
    m_IterPtr++;
    return temp;
  } else 
    return NULL;  // NULL
}

DocWord* Document::GetPrevWord()
{
  if ((m_IterPtr > 0) && (m_IterPtr <= m_DocLength)) {
    m_IterPtr--;
    DocWord* temp = &(m_WordList[m_IterPtr]);
    return temp;
  } else 
    return NULL;  // NULL
}

void Document::AdvanceIterator()
{
  if ((m_IterPtr >= 0) && (m_IterPtr < m_DocLength))
    m_IterPtr++;
}

void Document::BackUpIterator()
{
  if ((m_IterPtr > 0) && (m_IterPtr <= m_DocLength))
    m_IterPtr--;
}

void Document::SaveIterState()
{
  m_nSavedIterTop++;
  if (m_nSavedIterTop < m_nIterStackSize) {
    m_SavedIters[m_nSavedIterTop] = m_IterPtr;
  } else {
    DocIterPtr* temp = (DocIterPtr *) realloc(m_SavedIters, (m_nIterStackSize + k_iterStackSize) * sizeof(DocIterPtr));
    if (temp) {
      m_SavedIters = temp;
      m_nIterStackSize = m_nIterStackSize + k_iterStackSize;
    } else {
      cerr << "!!!ERR!!! -- iterator stack resize failed\n";
    }
  }
}

void Document::SaveSingleIterState()
{
  m_nSavedIterTop = 0;
  m_SavedIters[0] = m_IterPtr;
}

void Document::SaveIterState(Pattern* aPattern)
{
  m_nSavedIterTop++;
  if (m_nSavedIterTop < m_nIterStackSize) {
    m_SavedIters[m_nSavedIterTop] = m_IterPtr;
  } else {
    DocIterPtr* temp = (DocIterPtr *) realloc(m_SavedIters, (m_nIterStackSize + k_iterStackSize) * sizeof(DocIterPtr));
    if (temp) {
      m_SavedIters = temp;
      m_nIterStackSize = m_nIterStackSize + k_iterStackSize;
    } else {
      cerr << "!!!ERR!!! -- iterator stack resize failed\n";
    }
  }
}

void Document::PopIterState()
{
  if (m_nSavedIterTop > -1) {
    m_nSavedIterTop--;
  }
}

void Document::RestoreIterState()
{
  if (m_nSavedIterTop > -1) {
    m_IterPtr = m_SavedIters[m_nSavedIterTop];
    m_nSavedIterTop--;
  }
}

void Document::RestoreAdvanceAndSaveIterState() 
{
  if (m_nSavedIterTop > -1) {
    m_nSavedIterTop = 0;
    m_IterPtr = m_SavedIters[m_nSavedIterTop];
  }
  if ((m_IterPtr >= 0) && (m_IterPtr < m_DocLength)) {
    m_IterPtr++;
  }
  m_SavedIters[m_nSavedIterTop] = m_IterPtr;
}

void Document::UseSavedIterState()
{
  if (m_nSavedIterTop > -1) {
    m_IterPtr = m_SavedIters[m_nSavedIterTop];
  }
}


// Document internal routines

void Document::AddWord(DocWord* newWord)
{
  if (m_DocLength >= m_nAllocatedSize) {
    // we need to grow the array
    DocWord* temp = (DocWord*) realloc(m_WordList, (m_nAllocatedSize + 100) * sizeof(DocWord));
    if (temp) {
      m_WordList = temp;
      m_nAllocatedSize = m_nAllocatedSize +100;
    } else {
      cout << "!! ERROR !! resize of document array failed\n";
    }
  }
  m_WordList[m_DocLength++] = *(newWord);
} //AddWord

