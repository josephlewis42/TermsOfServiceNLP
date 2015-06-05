//-*-C++-*-

#include <string.h>
#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "SkipList.h"
#include "Rule.h"
#include "RuleBase.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

// RuleBase routines

RuleBase::RuleBase()
{
  m_RuleList = new SkipList();
}

RuleBase::~RuleBase()
{
  delete m_RuleList;
}

void RuleBase::Read(istream& stream, WordStore& ws)
{
  int loopFlag = 1;
  char c;

  while (loopFlag) {
    // Find the beginning of the next item
    stream.get(c);
    while (stream.good() && ((c < 'a') || (c > 'z')) && ((c < 'A') || (c > 'Z'))) 
      stream.get(c);
    if (c == 'r') {
      stream.unget();
      Rule* newRule = new Rule;
      newRule->Read(stream,ws);
      m_RuleList->Insert(newRule->GetSlotName(),newRule);
    } else {
      stream.unget();
      loopFlag = 0;
    }
  }
}


void RuleBase::Write(ostream& stream)
{
  Rule* curRule = NULL;
  m_RuleList->InitializeIterator();
  curRule = (Rule *) m_RuleList->GetNextItem();
  while (curRule) {
    curRule->Write(stream);
    curRule = (Rule *) m_RuleList->GetNextItem();
  }
}

void RuleBase::AddRule(Rule* newRule)
{
  m_RuleList->Insert(newRule->GetSlotName(),newRule);

}


DynamicPtrArray* RuleBase::FindRules(char* slotName, char* templateName)
{
  DynamicPtrArray* result = NULL;
  
  result = m_RuleList->Search(slotName);
  // now, this could theoretically contain rules for other templates with
  // the same slotName
  // so filter the result for templateName
  if (result) {
    int len = result->GetSize();
    Rule* aRule = NULL;
    for (int i = len -1; i >= 0; i--) {
      aRule = (Rule*) result->GetItem(i);
      //      if (!i) { cout << "null rule in " << slotName << "\n";}
      if (strcmp(templateName, aRule->GetTemplateName())) {
	// this rule doesn't apply to this template
	result->RemoveItem(i);
      }
    }
  }
  return result;
}

void RuleBase::DeleteRules(char* slotName) 
{
  m_RuleList->Delete(slotName);
}
