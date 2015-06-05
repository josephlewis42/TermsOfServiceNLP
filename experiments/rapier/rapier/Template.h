//-*-C++-*-
/* 5/21/96 Mary Elaine Califf 
 * a template
 */



class Slot {
protected:
  char*    m_SlotName;
  char     m_SlotType;
  int      m_nNumFillers;
  Filler*  m_FillerList;
  int      m_bLearnSlot;
  int      m_bMismatch;
  int      m_nNumFilled;
  int      m_nNumStarred;
  int      m_nMinCertainty;

public:

  char* GetSlotName() { return m_SlotName; }

  char GetSlotType() { return m_SlotType; }

  Filler* GetFillerList() {return m_FillerList; }

  void SetMismatchFlag() { m_bMismatch = 1; }

  void SetLearn() { m_bLearnSlot = 1; }

  int UseSlotToLearn() {return m_bLearnSlot;}
  
  int GetNumFilled() { return m_nNumFilled; }

  void IncNumFilled() { m_nNumFilled++; }

  void ZeroNumFilled() { m_nNumFilled = 0; }

  int GetNumStarred() { return m_nNumStarred; }

  void IncNumStarred() { m_nNumStarred++; }

  void ZeroNumStarred() { m_nNumStarred = 0; }

  int GetMinCertainty() { return m_nMinCertainty; }

  void SetNewCertainty(int newCertainty);

  Slot(char* name,char type, int numFilled, int numStarred);
  
  Slot(char* name,char type, int learner);
  
  ~Slot(); // release Filler storage

  int CountFillers();

  int CountMatchingFillers(Filler* matchingFillerList);

  int FillersMatch(Filler* matchingFillerList);

  void AddFiller(WordList* newFiller);

  void AddFillerList(Filler* newFillers);

  void ReadFillers(istream& stream, WordStore& ws);

  void WriteMarkedUnion(ostream& stream);

  void Write(ostream& stream);

  void WriteUnfilled(ostream& stream);

};

class Template
{
protected:
  char*    m_TemplateName;
  int      m_nNumSlots;
  Slot*    m_SlotArray[30];
  RuleBase* m_RuleBase;

public:
  //accessors
  char* GetTemplateName() { return m_TemplateName; }
  int GetNumSlots() { return m_nNumSlots; }
  char* GetSlotName(int slotNum) 
    { return m_SlotArray[slotNum]->GetSlotName(); }
  Filler* GetSlotFillers(int slotNum) 
    { return m_SlotArray[slotNum]->GetFillerList(); }
  Slot* GetSlot(int slotNum) { return m_SlotArray[slotNum]; }


  Template(RuleBase* ruleBase);
  
  ~Template();

  int CountFillers();

  int CountMatchingFillers(Template* matchingTemplate);
  
  int GetTemplateCertainty();

  int FillersMatch(Template* matchingTemplate);

  int UnionTemplates(Template* temp1, Template* temp2);

  void IntersectTemplates(Template* temp1, Template* temp2);

  void ReadUnFilled(istream& stream,WordStore& ws);
  
  void ReadFilled(istream& stream, WordStore& ws);
  
  void Write(ostream& stream);

  void WriteMarkedUnion(ostream& stream);

  void WriteUnfilled(ostream& stream);

  Template* CopyStructure();
  
  void ApplyTo(Document* doc);

  void MostSpecificRules(Document* doc, RuleBase* rb1, RuleBase* rb2);

  int MSpRulesForSlot(int slotNum, Document* doc, RuleBase* rb, Template* protoTemplate);

  int LocateFillerInDoc(Document* doc, Filler* filler,int startWord = -1);

  Rule* CreateNewRule(Document* doc, Filler* filler, int startWord, char* slotName);

};

int symbol(char c);

int abbreviation(char* c);

