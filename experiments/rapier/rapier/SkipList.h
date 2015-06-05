//-*-C++-*-
/* 8/30/96 Mary Elaine Califf
 * a C++ class implementing skip lists
 */


typedef struct SkipListNode {
  char*         key;
  void*         value;
  SkipListNode* forward[16]; /* array of forward pointers */
} SkipListNode;



void InitSkipLists(); // initialize the random bit source

class SkipList {

  int m_nLevel; // max level of the list

  SkipListNode* m_ListHead;  // pointer to the head of the list

  SkipListNode* m_IterPtr;

public:
  SkipList();

  ~SkipList();

  void Insert(char* key, void* value); // insert a key, value pair into the list

  int  Delete(char* key); // deletes all key, value pairs from list

  int  Delete(char* key, void* value); // delete key, value pair from list

  DynamicPtrArray* Search(char* key); // return all values associated with key
                                   // in a dynamic array

  void InitializeIterator(); // set iterator to beginning of list

  void* GetNextItem(); // get the next item and move iterator forward
 

protected:

  int RandomLevel();  // Returns a random level

};

