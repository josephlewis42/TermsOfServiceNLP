//-*-C++-*-
/* 5/23/96 Mary Elaine Califf 
 * a rule
 */

enum MatchVal { GOOD, RESTARTPOINT /* implied GOOD */, FAILED, ENDOFDOC }; 


void InitializeRuleCounters();

void WriteRuleCounts();

class PatternItem {
protected:
  WordList* m_Word;
  WordList* m_Tag;
  WordList* m_Sem;
  WordList* m_WordNot;
  WordList* m_TagNot;
  WordList* m_SemNot;

public:
  virtual int GetType() { return 1; }
  WordList* GetWord() { return m_Word; }
  WordList* GetTag() { return m_Tag; }
  WordList* GetSem() { return m_Sem; }
  WordList* GetWordNot() { return m_WordNot; }
  WordList* GetTagNot() { return m_TagNot; }
  WordList* GetSemNot() { return m_SemNot; }

  PatternItem();
  PatternItem(char* word, char* tag);
  PatternItem(WordList* word, WordList* tag, WordList* sem, 
	      WordList* wordNot, WordList* tagNot, WordList* semNot);
  PatternItem(PatternItem* oldItem);
  virtual ~PatternItem();

  virtual int CalcSize(int isFiller = 0);
  
  virtual void Read(istream& stream, WordStore& ws);

  virtual void Write(ostream& stream);

  virtual int Match(Document* doc,int retry);

  virtual int MatchFiller(Document* doc, WordList*& filler, int retry);

  virtual int MinLength();

protected:
  void WriteConstraints(ostream& stream);
  
};

class PatternList : public PatternItem {
protected:
  int   m_nLimit;
  int   m_nLastTryLength;

public:
  virtual int GetType() { return 2; }
  int GetListLimit() {return m_nLimit; }
  PatternList();
  PatternList(int len, WordList* word, WordList* tag, WordList* sem,
	      WordList* wordNot, WordList* tagNot, WordList* semNot);
  PatternList(PatternList* oldList);
  virtual ~PatternList();

  virtual int CalcSize(int isFiller = 0);

  virtual void Read(istream& stream, WordStore& ws);

  virtual void Write(ostream& stream);

  virtual int Match(Document* doc, int retry);

  virtual int MatchFiller(Document* doc, WordList*& filler, int retry);

  virtual int MinLength();


};

typedef struct PtnRetryPt {
  int    arrayIndex;
  WordList* fillerSoFar;
  PtnRetryPt* next;
} PtnRetryPt;

// Pattern

class Pattern {
protected:
  DynamicPtrArray* m_PatternArray;
  PtnRetryPt*      m_RetryStack; // stack of possible retry points

public:

  int GetLength() { return m_PatternArray->GetSize(); }

  int GetMinLength();

  PatternItem* GetPatternItem(int i) 
    { return (PatternItem*) m_PatternArray->GetItem(i); }
  
  Pattern();
  
  Pattern(int maxSize);
  
  Pattern(Pattern* oldPattern);
  
  virtual ~Pattern();
  
  void AddItem(PatternItem* newItem) { m_PatternArray->Add(newItem); }

  void AddToSuffix(Pattern* suffixAddition);

  void AddToPrefix(Pattern* prefixAddition);

  void PrefixFromDoc(Document* doc, int fillerStart);
 
  void FillerPattern(Filler* filler,Document* doc, int startPoint);
  
  void SuffixFromDoc(Document* doc, int startPoint);

  void Read(istream& stream, WordStore& ws);

  void Write(ostream& stream);

  int CalcSize(int isFiller = 0);

  int Match(Document* doc, int retry);
  
  int MatchFiller(Document* doc, WordList*& filler, int retry);


};

typedef struct CoversNode {
  Document * docCovered;
  WordList * fillerCovered;
  struct CoversNode* next;
} CoversNode;

class Rule {
protected:
  char*    m_TemplateName;
  char*    m_SlotName;
  int      m_nPosCovered;
  int      m_nNegCovered;
  Pattern* m_PrefixPattern;
  Pattern* m_FillerPattern;
  Pattern* m_SuffixPattern;
  int      m_nSize;
  CoversNode* m_CovList;

public:
  char* GetTemplateName() { return m_TemplateName; }
  char* GetSlotName() { return m_SlotName; }
  int GetRuleSize() { return m_nSize; }
  int GetPosCovered() { return m_nPosCovered; }
  int GetNegCovered() { return m_nNegCovered; }
  Pattern* GetPrefixPattern() { return m_PrefixPattern; }
  Pattern* GetFillerPattern() { return m_FillerPattern; }
  Pattern* GetSuffixPattern() {return m_SuffixPattern; }
  CoversNode* GetCovList() { return m_CovList; }

  Rule();

  Rule(char* templateName, char* slotName);

  Rule(Rule* orig);

  virtual ~Rule();

  void CreateFromDoc(Document* doc,Filler *filler,int startWord);

  void Read(istream& stream, WordStore& ws);
  
  void Write(ostream& stream);

  int CalcSize();

  int Match(Document* doc, Filler*& fillers, int findall);

  int CheckCoverage(CoversNode* covCompare);

};

