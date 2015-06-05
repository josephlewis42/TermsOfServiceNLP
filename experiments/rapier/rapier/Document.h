//-*-C++-*-
/* 5/9/96 Mary Elaine Califf 
 * a document's data is a simple list of word/tag pairs
 */
 
#include <istream>
#include <ostream>
using namespace std;

const int k_iterStackSize = 30;

typedef struct DocWord{
  char* theWord;
  char* theTag;
  char* prevText;
  char* origText;
} DocWord;


typedef int DocIterPtr;

class Pattern;

class Document 
{
protected:
  DocWord* m_WordList;
  int      m_nAllocatedSize;
  int      m_DocLength;
  DocIterPtr      m_IterPtr;
  int m_nSavedIterTop;
  DocIterPtr* m_SavedIters;
  int      m_nIterStackSize;
  
public:
  Document();
  
  ~Document();

  DocIterPtr GetCurIterPtr() { return m_IterPtr; }

  int Length() { return m_DocLength; }

  void Read(istream& stream,istream& origStream, WordStore& ws);
  
  void Write(ostream& stream);

  void WriteFromIter(ostream& stream, int wordsToWrite  = 10);

 // Iterators
  void InitializeIterator(); // Set iterator to beginning of document
                             // and clear out the saved iterators

  void ResetIterator(int newIterPoint) {m_IterPtr = newIterPoint; }

  DocWord* GetNextWord(); // Get next word and move iterator forward

  DocWord* GetPrevWord(); // Back up iterator and get the previous word

  void AdvanceIterator(); // move iterator forward one word

  void BackUpIterator(); // more iterator back one word

  void SaveIterState(); // Save the current iterator state

  void SaveSingleIterState(); // Save iterator state after cleaning stack

  void SaveIterState(Pattern* aPattern);
  
  void PopIterState(); // Pop the top saved iterator state

  void RestoreIterState(); // Restore the iterator to the top saved state

  void RestoreAdvanceAndSaveIterState(); 

  void UseSavedIterState();

protected:
  void AddWord(DocWord* newWord);

};


