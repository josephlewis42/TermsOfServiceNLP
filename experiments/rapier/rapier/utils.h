//-*-C++-*-
#include <istream>
#include <ostream>
#include <iostream>

char lowercase(char c);

typedef struct WordList {
  char*      word;
  char*      prevText;
  char*      origText;
  WordList*  next;
} WordList;

typedef struct Filler {
  WordList*  filler;
  Filler*    next;
} Filler;


class DynamicPtrArray {
protected:
  int     m_nAllocatedSize;
  int     m_nFilledSize;
  void**  m_PointerArray;
  int     m_CurPointer;
  
public:
  DynamicPtrArray();

  DynamicPtrArray(int initialSize);

  ~DynamicPtrArray();

  DynamicPtrArray* Copy();
  
  virtual void Add(void* itemPtr);

  virtual void AddFirst(void* itemPtr);

  int GetSize() { return m_nFilledSize; }

  void* GetItem(int index) { return m_PointerArray[index]; }

  void RemoveItem(int index);  // remove the item at index
};

typedef struct PriorityItem {
  double priority;
  void*  item;
} PriorityItem;

class PriorityDynPtrArray 
{
protected:
  int            m_nAllocatedSize;
  int            m_nFilledSize;
  PriorityItem*  m_PointerArray;
  int            m_CurPointer;

public:
  
  PriorityDynPtrArray(int queueLength);
  
  ~PriorityDynPtrArray();

  void* Add(double priority, void* item);
  
  void InitializeIterator() { m_CurPointer = 0; }

  void* GetNextItem();

  void* GetFirst();
  
  double GetFirstVal();

  double GetLastVal();

};

int EqualFillers(WordList* a, WordList* b);

int EqualFillerList(Filler* a, Filler* b);

int WordListSubset(WordList* subset, WordList* set);

WordList* CopyWordList(WordList* orig);

Filler* CopyFillerList(Filler* orig);

WordList* UnionWordLists(WordList* list1, WordList* list2);

void DestUnionWordLists(WordList*& list1, WordList* list2);

Filler* UnionFillerLists(Filler* list1, Filler* list2);

Filler* IntersectionFillerLists(Filler* list1, Filler* list2);

int WordListMember(char* word, WordList* list);

WordList* FillerMember(WordList* filler, Filler* fillerList);

