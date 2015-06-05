//-*-C++-*-
/* 10/1/96 Mary Elaine Califf 
 *  the learner
 */

class Example {
protected:
  int         m_bUseToLearnSlot;
  Document*   m_Doc;
  Template*   m_filledTemplate;

public:

  Document* GetDocument() { return m_Doc; }
  Template* GetFilledTemplate() { return m_filledTemplate; }
  int UseToLearnSlot() { return m_bUseToLearnSlot; }

  Example(Template* baseTemplate);

  ~Example();

  void ReadExample(char* fileRoot, WordStore& ws);

  void MostSpecificRules(RuleBase* rb1, RuleBase* rb2);

  void MSpRulesForSlot(int slotNum, RuleBase* rb, Template* protoTemplate);

};

class RuleInProgress : public Rule
{
protected:
  double m_fValue;
  int    m_nSlotNum;
  int    m_nNumNegs;
  int    m_nPrefixLength1;
  int    m_nPrefixLength2;
  int    m_nSuffixLength1;
  int    m_nSuffixLength2;
  Rule* m_baseRule1;
  Rule* m_baseRule2;
  DynamicPtrArray* m_CoveredExamples;

public:
  int GetNumNegs() { return m_nNumNegs; }
  double GetValue() { return m_fValue; }
  int GetPrefixLength1() { return m_nPrefixLength1; }
  int GetPrefixLength2() { return m_nPrefixLength2; }
  int GetSuffixLength1() { return m_nSuffixLength1; }
  int GetSuffixLength2() { return m_nSuffixLength2; }
  int GetSlotNum() {return m_nSlotNum; }
  DynamicPtrArray* GetCoveredExamples() { return m_CoveredExamples; }
  Rule* GetBaseRule1() { return m_baseRule1; }
  Rule* GetBaseRule2() { return m_baseRule2; }

  RuleInProgress(char* templateName, char* slotName, Pattern* filler,
		 int slotNum, Rule* baseRule1, Rule* baseRule2); 

  RuleInProgress(RuleInProgress* baseRule);

  virtual ~RuleInProgress();

  void AddToSuffix(Pattern* newSuffixPart, int sufLength1, int sufLength2);

  void AddToPrefix(Pattern* newPrefixPart, int prefLength1, int prefLength2);

  double Evaluate(DynamicPtrArray* examplesToUse = NULL);

};
  
class Learner {
protected:  
  RuleBase*    m_RB1;
  RuleBase*    m_RB2;
  int          m_nSeed1;
  int          m_nSeed2;
 
  Template*    m_ProtoTemplate;
  DynamicPtrArray* m_ExampleList;

public:

  Learner(Template* emptyTemplate);

  ~Learner();

  void ReadExamples(char* exampleFile, WordStore& ws);

  void BuildInitialRules();

  void BuildInitRulesForSlot(int slotNum,RuleBase* rb);

  void WriteRuleBases();
  
  void WriteRuleBase1(char* fileName);
  
  void WriteRuleBase2(char* fileName);

  void WriteProtoTemplate(char* fileName);

  void DoGeneralization();

  void GeneralizeRuleBase(RuleBase* rb, int* randSeed);

  int CompressRuleArray(DynamicPtrArray* ruleArray, char* slotName, 
			int* randSeed, int slotNum);

  int RemoveRedundantRules(DynamicPtrArray* ruleArray, char* slotName,
			   int slotNum);

  DynamicPtrArray* GeneralizeFillers(Rule* rule1, Rule* rule2);

  DynamicPtrArray* GeneralizeSuffixes(int sufLength1, int sufLength2,
				      PriorityDynPtrArray* seedRuleList,
				      double maxVal = 99999.9);

  DynamicPtrArray* GeneralizePrefixes(int prefLength1, int prefLength2,
				      PriorityDynPtrArray* seedRuleList,
				      double maxVal = 99999.9);

  DynamicPtrArray* GeneralizePattern(Pattern* pattern1, int start1, int end1,
				     Pattern* pattern2, int start2, int end2);

  DynamicPtrArray* EvenMatch(Pattern* pattern1, int start1, int end1,
			     Pattern* pattern2, int start2, int end2);

  DynamicPtrArray* UnevenMatch(Pattern* pattern1, int start1, int end1,
			       Pattern* pattern2, int start2, int end2);
};


  

