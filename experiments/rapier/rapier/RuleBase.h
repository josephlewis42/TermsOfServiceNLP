//-*-C++-*-
/* 8/28/96 Mary Elaine Califf 
 * management of list of rules
 */

class RuleBase {

protected:
  // Here we need the list of rules that is the basis of the rulebase
  SkipList* m_RuleList;

public:

  RuleBase(); 

  ~RuleBase();

  void Read(istream& stream, WordStore& ws);
  
  void Write(ostream& stream);

  void AddRule(Rule* newRule);  

  DynamicPtrArray* FindRules(char* slotName, char* templateName);

  void DeleteRules(char* slotName);

};

