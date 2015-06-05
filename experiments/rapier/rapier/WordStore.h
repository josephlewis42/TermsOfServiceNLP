//-*-C++-*-

/* WordStore.h -- single storage for all "words" in use in the program */


typedef struct StorageBlock {
  StorageBlock *  nextBlock;
  char*          nextStart;
  char           blockData[4080];
} StorageBlock;

class WordStore 
{
 protected:
  StorageBlock  *m_StorageBlocks[27];

 public:
  WordStore();
    // initialize the word store

  char* StoreWord(char* wordToStore);
    // store wordToStore unless its already in the word store, and return
    // a pointer to the word in the word store
  
  void PrintStats();
    // Print statistics on use of the word store

};
