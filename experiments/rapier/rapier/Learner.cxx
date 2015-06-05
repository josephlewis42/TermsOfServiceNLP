//-*-C++-*-

#include <string.h>
#include "WordStore.h"
#include "utils.h"
#include "Document.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"
#include "Learner.h"
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include "SemClass.h"
#include <math.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

const int k_BeamWidth = 7;
const int k_CompressFails = 4;
const int k_ExtendFails = 3;
const int k_NumPairs = 6;
const int k_MaxNoGainSearch = 5;
const double k_SizeWeight = 0.01;
const int k_MinCov = 3;

// Example methods
Example::Example(Template* baseTemplate)
{
  m_Doc = NULL;
  m_filledTemplate = NULL;
  
  m_Doc = new Document();
  m_filledTemplate = baseTemplate->CopyStructure();

}

Example::~Example()
{
  delete m_Doc;
  delete m_filledTemplate;
}

void Example::ReadExample(char* fileRoot, WordStore& ws)
{
  char    docFileName[80];
  char    origFileName[80];
  char    tempFileName[80];

    cout << "reading example " << fileRoot << "\n";
  strcpy(docFileName, fileRoot);
  strcpy(tempFileName, fileRoot);
  strcpy(origFileName, fileRoot);
  
  strcat(docFileName, "_out");
  strcat(origFileName, ".orig");
  strcat(tempFileName, ".template");
  
  // open the document file and create an istream for it
  filebuf docin;
  filebuf* fb = docin.open(docFileName,std::ios::in);
  filebuf origin;
  if (!fb) {
    cout << "could not open file " << docFileName << "\n";
  } else {
    fb = origin.open(origFileName, std::ios::in);
    if(!fb) {
      cout << "could not open file " << origFileName << "\n";
    } else {
      //    cout << "reading document\n";
      istream docfile(&docin);
      istream origfile(&origin);
      m_Doc->Read(docfile,origfile,ws);
      docin.close();
    }
  }
  //  cout << "template file name: " << tempFileName << "\n";
  fb = docin.open(tempFileName,std::ios::in);
  if (!fb) {
    cout << "could not open file " << tempFileName << "\n";
  } else {
    //    cout << "reading template\n";
    istream tempfile(&docin);
    m_filledTemplate->ReadFilled(tempfile,ws);
    docin.close();
  }
  //  cout << "done reading example\n";

}

void Example::MostSpecificRules(RuleBase* rb1, RuleBase* rb2)
{
  m_filledTemplate->MostSpecificRules(m_Doc, rb1, rb2);
}

void Example::MSpRulesForSlot(int slotNum, RuleBase* rb, Template* protoTemplate)
{
  m_bUseToLearnSlot = m_filledTemplate->MSpRulesForSlot(slotNum, m_Doc, rb, protoTemplate);

}

// RuleInProgress methods
RuleInProgress::RuleInProgress(char* templateName, char* slotName, 
			       Pattern* filler,int slotNum,
			       Rule* baseRule1, Rule* baseRule2) :
  Rule(templateName, slotName)
{
  m_FillerPattern = filler;
  m_PrefixPattern = new Pattern(0);
  m_SuffixPattern = new Pattern(0);
  m_nPrefixLength1 = 0;
  m_nPrefixLength2 = 0;
  m_nSuffixLength1 = 0;
  m_nSuffixLength2 = 0;
  m_CoveredExamples = NULL;
  m_nSlotNum = slotNum;
  m_baseRule1 = baseRule1;
  m_baseRule2 = baseRule2;
}

RuleInProgress::RuleInProgress(RuleInProgress* baseRule) :
  Rule(baseRule->GetTemplateName(), baseRule->GetSlotName())
{
  m_FillerPattern = new Pattern(baseRule->GetFillerPattern());
  m_PrefixPattern = new Pattern(baseRule->GetPrefixPattern());
  m_SuffixPattern = new Pattern(baseRule->GetSuffixPattern());
  m_CoveredExamples = baseRule->GetCoveredExamples()->Copy();
  m_nPrefixLength1 = baseRule->GetPrefixLength1();
  m_nPrefixLength2 = baseRule->GetPrefixLength2();
  m_nSuffixLength1 = baseRule->GetSuffixLength1();
  m_nSuffixLength2 = baseRule->GetSuffixLength2();
  m_nSlotNum = baseRule->GetSlotNum();
  m_baseRule1 = baseRule->GetBaseRule1();
  m_baseRule2 = baseRule->GetBaseRule2();
}

RuleInProgress::~RuleInProgress()
{
  //  cout << "deleting rule\n";
  if (m_CoveredExamples) { 
    //    cout << "deleting covered examples\n";
    delete m_CoveredExamples;
  }
}

void RuleInProgress::AddToSuffix(Pattern* newSuffixPart, int sufLength1, 
				 int sufLength2)
{
  m_SuffixPattern->AddToSuffix(newSuffixPart);
  m_nSuffixLength1 = sufLength1;
  m_nSuffixLength2 = sufLength2;
}

void RuleInProgress::AddToPrefix(Pattern* newPrefixPart, int prefLength1,
				int prefLength2)
{
  m_PrefixPattern->AddToPrefix(newPrefixPart);
  m_nPrefixLength1 = prefLength1;
  m_nPrefixLength2 = prefLength2;
}

double RuleInProgress::Evaluate(DynamicPtrArray* examplesToUse)
{
  int negatives = 0;
  int positives = 0;
  double temp_ratio;
  double log_val;
  double inverted_log_val;

  DynamicPtrArray* usedExamples = NULL;
  if (!m_CoveredExamples && !examplesToUse) {
    cout << "RuleInProgress:Evaluate called with no examples\n";
    return 0.0;
  }
  if (examplesToUse) {
    if (m_CoveredExamples) {
      delete m_CoveredExamples;
    }
    usedExamples = examplesToUse;
  } else {
    usedExamples = m_CoveredExamples;
  }
  Example* anExample = NULL;
  Filler* fillersFound = NULL;
  int matchFound = 0;
  m_CoveredExamples = new DynamicPtrArray;
  int len = usedExamples->GetSize();
  for (int i = 0; i < len; i++) {
    anExample = (Example*) usedExamples->GetItem(i);
    if (anExample->UseToLearnSlot()) {
      fillersFound = NULL;
      matchFound = this->Match(anExample->GetDocument(),fillersFound,1);
      if ((GOOD == matchFound) || (RESTARTPOINT == matchFound)) {
	m_CoveredExamples->Add(anExample);
	// have to count up positives and negatives
	Filler* found = fillersFound;
	WordList* matchedFiller;
	while (found) {
	  if (matchedFiller = FillerMember(found->filler,anExample->GetFilledTemplate()->GetSlotFillers(m_nSlotNum))) {
	    positives++;
	    CoversNode* newNode = new CoversNode;
	    newNode->docCovered = anExample->GetDocument();
	    newNode->fillerCovered = matchedFiller;
	    newNode->next = m_CovList;
	    m_CovList = newNode;
	    
	  } else {
	    negatives++;
	    //cout << "spurious filler: ";
	    //  WordList* a;
	    //a = found->filler;
	    //while (a) {
	    // cout << a->word << " ";
	    //a = a->next;
	    //}
	    //cout << "\n";
	  }
	  found = found->next;
	}
	found = fillersFound;
	Filler* prev;
	while (found) {
	  WordList* a =found->filler;
	  WordList* b = NULL;
	  while (a) {
	    b = a;
	    a = a->next;
	    delete b;
	  }
	  prev = found;
	  found= found->next;
	  delete prev;
	}
      }
    }
  }
  if (!examplesToUse) {
    delete usedExamples;
  }
  // now we have to do the calculations 
  m_nNumNegs = negatives;
  //  cout << "negs: " << negatives << " pos: " << positives << "\n";
  m_nPosCovered = positives;
  m_nNegCovered = negatives;
  if (positives < k_MinCov) {
    m_fValue = 99999.9;
    return m_fValue;
  } else {
    this->CalcSize();
            // info gain type value
    //below modified to use Laplace est
    temp_ratio = ((double) positives + 1) / ((double) (positives + negatives + 2));
    log_val = log(temp_ratio);
    inverted_log_val = -1.442695 * log_val;
        m_fValue = inverted_log_val + (k_SizeWeight * this->GetRuleSize())/(double) positives;
    //    m_fValue = inverted_log_val + (0.01 * this->GetRuleSize());
    /*
m_fValue = (double) this->GetRuleSize() / (double) positives + negatives;
*/
    //    m_fValue = -1.442695 * log(((double) positives) / (double) (positives + negatives)) / ((0.1 * this->GetRuleSize()));
    return m_fValue;
  }
}



//Learner methods
Learner::Learner(Template* emptyTemplate)
{
  m_ProtoTemplate = emptyTemplate;
  // first let's give the 2 seeds values
  long temp;
  time(&temp);
  m_nSeed1 = temp;
  //m_nSeed1 = 4567;
  m_nSeed2 = temp*2 + 1;

  // create the 2 rule bases
  m_RB1 = NULL;
  m_RB2 = NULL; // in case we fail
  m_RB1 = new RuleBase();
  m_RB2 = new RuleBase();
  
  m_ExampleList = new DynamicPtrArray;
}

Learner::~Learner()
{
  if (m_RB1) 
    delete m_RB1;
  if (m_RB2)
    delete m_RB2;
  
  if (m_ExampleList) {
    // first delete all of the examples
    int len = m_ExampleList->GetSize();
    for (int i = 0; i < len; i++) {
      delete m_ExampleList->GetItem(i);
    }
    delete m_ExampleList;
  }
}
  

void Learner::ReadExamples(char* exampleFile, WordStore& ws)
{
  char     baseName[80];
  Example* anExample = NULL;

  filebuf fin;
  fin.open(exampleFile,std::ios::in);
  istream infile(&fin);

  while (infile.good()) {
    // read the doc name
    infile.getline(baseName,80);
    if (strlen(baseName)) {
      // create an example
      anExample = new Example(m_ProtoTemplate);
      // read the example
      anExample->ReadExample(baseName,ws);
      // add the example to the example list
      m_ExampleList->Add(anExample);
    }
  }
}


void Learner::BuildInitialRules()
{
  // spin through all of the examples, and create a rule for each template
  // filler consisting of the entire document context, and add each rule
  // to both RB1 and RB2
  int len = m_ExampleList->GetSize();
  for (int i = 0; i < len; i++) {
        cout << "building rules for example " << i << " \n";
    Example* anExample = (Example*) m_ExampleList->GetItem(i);
    anExample->MostSpecificRules(m_RB1,m_RB2);
  }
}

void Learner::BuildInitRulesForSlot(int slotNum,RuleBase* rb)
{
  int len = m_ExampleList->GetSize();
  for (int i = 0; i < len; i++) {
        cout << "building rules for example " << i << " \n";
    Example* anExample = (Example*) m_ExampleList->GetItem(i);
    anExample->MSpRulesForSlot(slotNum,rb,m_ProtoTemplate);
  }
}
  

void Learner::WriteRuleBases()
{
  cout << "rule base 1\n\n";
  m_RB1->Write(cout);
  cout << "\n\nrule base 2\n\n";
  m_RB2->Write(cout);
  cout << "\n";
} 

void Learner::WriteRuleBase1(char* fileName) 
{
  filebuf rbout;
  filebuf* fb = rbout.open(fileName,std::ios::out);
  ostream rboutstream(&rbout);
  m_RB1->Write(rboutstream);
}

void Learner::WriteRuleBase2(char* fileName) 
{
  filebuf rbout;
  filebuf* fb = rbout.open(fileName,std::ios::out);
  ostream rboutstream(&rbout);
  m_RB2->Write(rboutstream);
}

void Learner::WriteProtoTemplate(char* fileName)
{
  filebuf rbout;
  filebuf* fb = rbout.open(fileName,std::ios::out);
  ostream rboutstream(&rbout);
  m_ProtoTemplate->WriteUnfilled(rboutstream);
}


void Learner::DoGeneralization()
{
  this->GeneralizeRuleBase(m_RB1, &m_nSeed1);
  WriteRuleCounts();
  //  this->GeneralizeRuleBase(m_RB2, &m_nSeed2);
}

void Learner::GeneralizeRuleBase(RuleBase* rb, int* randSeed)
{
  char* slotName;
  // we need to generalize the rules for each slot
  int numSlots = m_ProtoTemplate->GetNumSlots();
  int len;
  for (int i = 0; i < numSlots; i++) {
    slotName = m_ProtoTemplate->GetSlotName(i);
    m_ProtoTemplate->GetSlot(i)->ZeroNumFilled();
    m_ProtoTemplate->GetSlot(i)->ZeroNumStarred();
    //first, build the initial rules for the slot
    cout << "creating intial rules for " << slotName << "\n";
    this->BuildInitRulesForSlot(i,rb);
    // now generalize those rules
    cout << "generalizing rules for " << slotName << "\n";
    // we need to collect the starting rules 
    DynamicPtrArray* ruleArray = 
      rb->FindRules(slotName, m_ProtoTemplate->GetTemplateName());
    int failures = 0;
    if (ruleArray) {
      while ((failures < k_CompressFails) && (failures < ruleArray->GetSize()/k_NumPairs + 1) &&
	     (ruleArray->GetSize() > 1)) {
	// write rules
	int oldSize = ruleArray->GetSize();
	cout << "Number of rules:" << ruleArray->GetSize() << "\n";
	// cout << "Current rules for " << slotName << "\n";
	// len = ruleArray->GetSize();
	// for (int k = 0; k < len; k++) {
	//((Rule*) ruleArray->GetItem(k))->Write(cout);
	// }
	cout << "\nAttempting to compress\n";      
	if (this->CompressRuleArray(ruleArray,slotName,randSeed,i)) {
	  if (ruleArray->GetSize() < oldSize) {
	    oldSize = ruleArray->GetSize();
	    failures = 0;
	  } else if (ruleArray->GetSize() > oldSize) {
	    failures = ruleArray->GetSize();
	  } else {
	    failures++;
	  }
	} else 
	  failures++;      
      }
      cout << "Final rules for " << slotName << "\n";
      len = ruleArray->GetSize();
      cout << "Number of rules: " << len << "\n";
      for (int k = 0; k < len; k++) {
	((Rule*) ruleArray->GetItem(k))->Write(cout);
      }
      cout << "\n\n";  
      // plug the new rule set in in place of the original set
      rb->DeleteRules(slotName);
      len = ruleArray->GetSize();
      for (int j = 0; j < len; j++) {
	rb->AddRule((Rule*) ruleArray->GetItem(j));
      }
      delete ruleArray;
    }
    WriteRuleCounts();
  }
}

typedef struct RulePair {
  Rule* rule1;
  Rule* rule2;
} RulePair;

RulePair* GetRulePairs(int maxPairs, DynamicPtrArray* ruleArray, int* randSeed,
		       int& numPairs)
{
  RulePair* pairArray;
  int arrLen = ruleArray->GetSize();
  int allPairs = (arrLen * (arrLen-1)) / 2;
  if (allPairs > maxPairs) {
    // find maxPairs random pairs
    numPairs = maxPairs;
    pairArray = (RulePair*) malloc(numPairs * sizeof(RulePair));
    for (int i = 0; i < numPairs; i++) {
      // generate a random pair
      int ruleNum1, ruleNum2;
      ruleNum1 = rand_r((unsigned int*)randSeed) % arrLen;
      ruleNum2 = rand_r((unsigned int*)randSeed) % arrLen;
      while (ruleNum1 == ruleNum2)
	ruleNum2 = rand_r((unsigned int*) randSeed) % arrLen;
      pairArray[i].rule1 = (Rule*) ruleArray->GetItem(ruleNum1);
      pairArray[i].rule2 = (Rule*) ruleArray->GetItem(ruleNum2);
    }
  } else {
    // compute all poss pairs
    numPairs = allPairs;
    pairArray = (RulePair*) malloc(numPairs * sizeof(RulePair));
    int i = 0;
    for (int j = 1; j < arrLen; j++) {
      for (int k = 0; k < j; k++) {
	pairArray[i].rule1 = (Rule*) ruleArray->GetItem(j);
	pairArray[i].rule2 = (Rule*) ruleArray->GetItem(k);
	i++;
      }
    }    
  }
  return pairArray;
}

int Learner::CompressRuleArray(DynamicPtrArray* ruleArray, char* slotName,
			       int* randSeed, int slotNum)
{
  int i;
  // select two rules from the array to generalize from
  int numRules = ruleArray->GetSize();
  int numPairs = 0;
  RulePair* pairArray = GetRulePairs(k_NumPairs,ruleArray,randSeed,numPairs);
  PriorityDynPtrArray* newRules = new PriorityDynPtrArray(k_BeamWidth);
  // generalize the fillers
  int prefixLimit = 0;
  int suffixLimit = 0;
  int len;
  for (int j = 0; j < numPairs; j++) {
    DynamicPtrArray* genFillers = this->GeneralizeFillers(pairArray[j].rule1,
							  pairArray[j].rule2);
    if (!genFillers) return 0; // no fillers produced
    // make rules from them, evaluate, and add to prioritized array
    len = genFillers->GetSize();
     cout << len << " generalized fillers found\n";
    if (len == 0) {
      delete genFillers;
      return 0; // no fillers produced
    }
    for (i = 0; i < len; i++) {
      RuleInProgress* newRule = 
	new RuleInProgress(m_ProtoTemplate->GetTemplateName(), slotName, 
			   (Pattern*)genFillers->GetItem(i),slotNum,
			   pairArray[j].rule1,pairArray[j].rule2);
      //           newRule->Write(cout);
      // now evaluate the rule: no covered examples yet
      double ruleVal = newRule->Evaluate(m_ExampleList);
      //    cout << "ruleVal: " << ruleVal << "\n";
      if (ruleVal < 99999.9) {
	RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	if (deadRule) {
	  //  	  cout << "deleting deadRule\n";
	  delete deadRule;
	}
      } else {
	//cout << "deleting newRule\n";
	delete newRule;
      }
    }
    delete genFillers;
  }
  // check for best rule being a perfect rule
  RuleInProgress* learnedRule = NULL;
  double prevValue = 99999.9;
  int numNoImprovements = 0;
  if (!newRules->GetFirst()) return 0;
  //mec commenting out check after filler generalization
  //  if (0 == ((RuleInProgress*) newRules->GetFirst())->GetNumNegs()) {
  //    learnedRule = (RuleInProgress*) newRules->GetFirst();
  //  }

  prevValue = newRules->GetFirstVal();
  // while no perfect rule and still trying
  while (!learnedRule && (numNoImprovements < k_ExtendFails)) {
    cout << "numNoImprovements: " << numNoImprovements << "\n";
    cout << "generalizing prefixes to " << prefixLimit << "+\n";
    double maxVal = newRules->GetLastVal();
    DynamicPtrArray* newPrefRules1 = 
      GeneralizePrefixes(prefixLimit,
			 prefixLimit+1,newRules,maxVal);
    DynamicPtrArray* newPrefRules2 =
      GeneralizePrefixes(prefixLimit+1,
			 prefixLimit,newRules,maxVal);
    DynamicPtrArray* newPrefRules3 =
      GeneralizePrefixes(prefixLimit+1,
			 prefixLimit+1,newRules,maxVal);
    prefixLimit++;
    // evaluate the rules
    len = newPrefRules1->GetSize();
    for (i = 0; i < len; i++) {
      RuleInProgress* newRule = (RuleInProgress*) newPrefRules1->GetItem(i);
      double ruleVal = newRule->GetValue();
      //            newRule->Write(cout);
      //    cout << "rule value is " << ruleVal << "\n";
      if (ruleVal < 99999.9) {
	RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	if (deadRule)
	  delete deadRule;
      }
    }
    delete newPrefRules1;
    len = newPrefRules2->GetSize();
    //        cout << "got " << len << " new rules\n";
    for (i = 0; i < len; i++) {
      RuleInProgress* newRule = (RuleInProgress*) newPrefRules2->GetItem(i);
      //      cout << "new rule with addition to prefix\n";
      //    newRule->Write(cout);
      double ruleVal = newRule->GetValue();
      //    cout << "rule value is " << ruleVal << "\n";
      if (ruleVal < 99999.9) {
	RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	if (deadRule) delete deadRule;
      } else {
	delete newRule;
      }
    }
    delete newPrefRules2;
    len = newPrefRules3->GetSize();
    //        cout << "got " << len << " new rules\n";
    for (i = 0; i < len; i++) {
      RuleInProgress* newRule = (RuleInProgress*) newPrefRules3->GetItem(i);
      //            cout << "new rule with addition to prefix\n";
      //    newRule->Write(cout);
      double ruleVal = newRule->GetValue();
      //    cout << "rule value is " << ruleVal << "\n";
      if (ruleVal < 99999.9) {
	RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	if (deadRule) delete deadRule;
      } else {
	delete newRule;
      }
    }
    delete newPrefRules3;
    // mec commenting out check after prefix generalization
    //    if (0 == ((RuleInProgress*) newRules->GetFirst())->GetNumNegs()) {
    //      learnedRule = (RuleInProgress*) newRules->GetFirst();
    //    } else {
      maxVal = newRules->GetLastVal();
      DynamicPtrArray* newSufRules1 = 
	GeneralizeSuffixes(suffixLimit,
			   suffixLimit+1,newRules,maxVal);
      DynamicPtrArray* newSufRules2 =
	GeneralizeSuffixes(suffixLimit+1,
			   suffixLimit,newRules,maxVal);
      DynamicPtrArray* newSufRules3 =
	GeneralizeSuffixes(suffixLimit+1,
			   suffixLimit+1,newRules,maxVal);
      suffixLimit++;
      // evaluate the rules
      len = newSufRules1->GetSize();
      for (i = 0; i < len; i++) {
	RuleInProgress* newRule = (RuleInProgress*) newSufRules1->GetItem(i);
	double ruleVal = newRule->GetValue();
	//		newRule->Write(cout);
	//cout << "rule value is " << ruleVal << "\n";
	if (ruleVal < 99999.9) {
	  RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	  if (deadRule) delete deadRule;
	} else {
	  delete newRule;
	}
      }
      delete newSufRules1;
      len = newSufRules2->GetSize();
      for (i = 0; i < len; i++) {
	RuleInProgress* newRule = (RuleInProgress*) newSufRules2->GetItem(i);
	double ruleVal = newRule->GetValue();
	//		newRule->Write(cout);
	//cout << "rule value is " << ruleVal << "\n";
        if (ruleVal < 99999.9) {
	  RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	  if (deadRule) delete deadRule;
	} else {
	  delete newRule;
	}
      }
      delete newSufRules2;
      len = newSufRules3->GetSize();
      for (i = 0; i < len; i++) {
        RuleInProgress* newRule = (RuleInProgress*) newSufRules3->GetItem(i);
        double ruleVal = newRule->GetValue();
	//		newRule->Write(cout);
	//cout << "rule value is " << ruleVal << "\n";
        if (ruleVal < 99999.9) {
	  RuleInProgress* deadRule = (RuleInProgress*) newRules->Add(ruleVal,newRule);
	  if (deadRule) delete deadRule;
	} else {
	  delete newRule;
	}
      }
      delete newSufRules3;

      // specialize rules with negative constraints

      //    }
    if (0 == ((RuleInProgress*) newRules->GetFirst())->GetNumNegs()) {
      learnedRule = (RuleInProgress*) newRules->GetFirst();
    } else {
      if (prevValue <= newRules->GetFirstVal()) {
	numNoImprovements++;
      } else {
	prevValue = newRules->GetFirstVal();
	numNoImprovements = 0;
      }
    }
  } // end while
  if (!learnedRule) {
    int pos1;
    int neg1;
    if (((pos1 = ((Rule*) newRules->GetFirst())->GetPosCovered()) >
         (neg1 = ((Rule*) newRules->GetFirst())->GetNegCovered())) 
        && (((double) (pos1 - neg1) / (double) (pos1 + neg1)) > 0.9)) {
      learnedRule = (RuleInProgress*) newRules->GetFirst();
    }
  }
  // delete all unused rules
  if (learnedRule) {
    cout << "learned a rule\n";
    learnedRule->Write(cout);
  }
  newRules->InitializeIterator();
  RuleInProgress* tempRule = (RuleInProgress*) newRules->GetNextItem();
  if (!learnedRule) {
    delete tempRule;
  }
  tempRule = (RuleInProgress*) newRules->GetNextItem();
  while (tempRule) {
    delete tempRule;
    tempRule = (RuleInProgress*) newRules->GetNextItem();
  }
  free(pairArray);
  delete newRules;
  //  cout << "extra rules deleted\n";
  if (learnedRule) {
    //    cout << "adding the new rule\n";
    // add new rule to ruleArray
    ruleArray->AddFirst(learnedRule);
    //      cout << "removing redundant rules\n";
    // remove redundant rules from ruleArray
    return this->RemoveRedundantRules(ruleArray, slotName, slotNum);
  } else {
    return 0;
  }
}

int Learner::RemoveRedundantRules(DynamicPtrArray* ruleArray, 
				  char* slotName, int slotNum)
{
  Rule* newRule = (Rule*) ruleArray->GetItem(0);
  int len = ruleArray->GetSize();
  int i;
  for (i = len-1; i>0; i--) {
    Rule* aRule = (Rule *) ruleArray->GetItem(i);
    if (aRule->CheckCoverage(newRule->GetCovList())) {
      // delete this rule; it's covered by the new one
      cout << "removing rule #" << i << "\n";
      delete (Rule*) ruleArray->GetItem(i);
      ruleArray->RemoveItem(i);
    } else {
      cout << "rule " << i << " needed\n";
    }
  }
return 1;
}

DynamicPtrArray* Learner::GeneralizeFillers(Rule* rule1, Rule* rule2)
{
  Pattern* filler1 = rule1->GetFillerPattern();
  Pattern* filler2 = rule2->GetFillerPattern();

  int length1 = filler1->GetLength();
  int length2 = filler2->GetLength();

  DynamicPtrArray* results = NULL;

      cout << "in GeneralizeFillers\n";
   cout << "Filler 1:\n";
   filler1->Write(cout);
  cout << "\nFiller 2: \n";
  filler2->Write(cout);
  cout << "\n";

  results = this->GeneralizePattern(filler1, 0, length1-1, 
				    filler2, 0, length2-1);
  if (!results) {
    //cout << "no generalizations found";
  }
  return results;
  
}

DynamicPtrArray* Learner::GeneralizePattern(Pattern* pattern1, int start1, 
					    int end1, Pattern* pattern2,
					    int start2, int end2)
{
  DynamicPtrArray* results = NULL;
  if ((end1-start1 < 0) || (end2-start2 < 0 )) {
    return NULL;
  }
  if (end1-start1 == end2-start2) {
    //    cout << "going to EvenMatch\n";
    results = this->EvenMatch(pattern1, start1, end1, pattern2, start2, end2);
  } else {
    if (end1- start1 > end2 - start2) {
      results = this->UnevenMatch(pattern1, start1, end1, pattern2, start2, end2);
    } else {
      results = this->UnevenMatch(pattern2, start2, end2, pattern1, start1, end1);
    }
  }
  return results;
}

DynamicPtrArray* Learner::GeneralizeSuffixes(int sufLength1, int sufLength2,
					     PriorityDynPtrArray* seedRuleList,
					     double maxVal)
{
  DynamicPtrArray* newRuleList = new DynamicPtrArray;
  seedRuleList->InitializeIterator();
  int skipRule = 0;
  RuleInProgress* seedRule = (RuleInProgress*) seedRuleList->GetNextItem();
  Rule* rule1;
  Rule* rule2;
  if (seedRule) {
    skipRule = 0;
    rule1 = seedRule->GetBaseRule1();
    rule2 = seedRule->GetBaseRule2();
    if ((sufLength1 > rule1->GetSuffixPattern()->GetLength()) || 
	(sufLength2 > rule2->GetSuffixPattern()->GetLength())) {
      skipRule = 1;
      //      cout << "suffix pattern too short\n";
    }
  }
  while (seedRule) {
    // determine where seedRule's suffix1 and 2 endings are
    int sufStart1 = seedRule->GetSuffixLength1();
    int sufStart2 = seedRule->GetSuffixLength2();
    if ((sufLength1 - sufStart1 <= 0) || (sufLength2 - sufStart2 <= 0) || skipRule || ((sufLength1 - sufStart1 > k_MaxNoGainSearch) && (sufLength2 - sufStart2 > k_MaxNoGainSearch))) {
      // we aren't going to get anything out of this set
    } else {
      //      cout << "suffix Lengths: ";
      //cout << rule1->GetSuffixPattern()->GetLength() << " " << sufLength1 << " and ";
      //cout << rule2->GetSuffixPattern()->GetLength() << " " << sufLength2 << "\n";
      // OK, let's compute the set of suffix additions to make to this rule
      DynamicPtrArray* suffixAdditions = 
	this->GeneralizePattern(rule1->GetSuffixPattern(),sufStart1, 
				 sufLength1-1, rule2->GetSuffixPattern(),
				 sufStart2, sufLength2-1);
      // create the new rules with the suffix additions to seedRules
      if (suffixAdditions) {
      int len = suffixAdditions->GetSize();
      for (int j = 0; j < len; j++) {
	RuleInProgress* newRule = new RuleInProgress(seedRule);
	newRule->AddToSuffix((Pattern*) suffixAdditions->GetItem(j),sufLength1, sufLength2);
	delete (Pattern*) suffixAdditions->GetItem(j);
	double ruleVal = newRule->Evaluate();
	//	cout << "Rule Value: " << ruleVal;
	if (maxVal > ruleVal) {
	  //	  cout << " rule added\n";
	  newRuleList->Add(newRule);
	} else {
	  delete newRule;
	  //	  cout << " rule deleted\n";
	}
      }
      delete suffixAdditions;
      }
    }
    seedRule = (RuleInProgress*) seedRuleList->GetNextItem();
    if (seedRule) {
      skipRule = 0;
      rule1 = seedRule->GetBaseRule1();
      rule2 = seedRule->GetBaseRule2();
      if ((sufLength1 > rule1->GetSuffixPattern()->GetLength()) || 
	  (sufLength2 > rule2->GetSuffixPattern()->GetLength())) {
	skipRule = 1;
	//	cout << "suffix pattern too short\n";
      }
    }
  } // while seedRule
  return newRuleList;
}

DynamicPtrArray* Learner::GeneralizePrefixes(int prefLength1, int prefLength2,
					     PriorityDynPtrArray* seedRuleList,
					     double maxVal)
{
  DynamicPtrArray* newRuleList = new DynamicPtrArray;
  seedRuleList->InitializeIterator();
  RuleInProgress* seedRule = (RuleInProgress*) seedRuleList->GetNextItem();
  int skipRule = 0;
  Rule* rule1;
  Rule* rule2;
  Pattern* prefPattern1;
  Pattern* prefPattern2;
  if (seedRule) {
    skipRule = 0;
    rule1 = seedRule->GetBaseRule1();
    rule2 = seedRule->GetBaseRule2();
    prefPattern1 = rule1->GetPrefixPattern();
    prefPattern2 = rule2->GetPrefixPattern();
    //    cout << "prefLength1: " << prefLength1 << "\n";
    //    cout << "prefPattern1->GetLength(): " << prefPattern1->GetLength() << "\n";
    //    cout << "prefLength2: " << prefLength2 << "\n";
    //    cout << "prefPattern2->GetLength(): " << prefPattern2->GetLength() << "\n";
    //    cout << "checking pattern lengths before while\n";
    if ((prefLength1 > prefPattern1->GetLength()) || 
	(prefLength2 > prefPattern2->GetLength())) {
      skipRule = 1;
      //      cout << "prefix pattern too short\n";
    }
  }
  while (seedRule) {
    // determine where seedRule's prefix1 and 2 endings are
    int prefEnd1 = seedRule->GetPrefixLength1();
    int prefEnd2 = seedRule->GetPrefixLength2();
    if ((prefLength1 - prefEnd1 <= 0) || (prefLength2 - prefEnd2 <= 0) || skipRule || ((prefLength1 - prefEnd1 > k_MaxNoGainSearch) && (prefLength2 - prefEnd2 > k_MaxNoGainSearch))) {
      // we aren't going to get anything out of this set
    } else {
      // OK, let's compute the set of prefix additions to make to this rule
      DynamicPtrArray* prefixAdditions = 
	this->GeneralizePattern(prefPattern1,
				prefPattern1->GetLength() - prefLength1,
				prefPattern1->GetLength() - prefEnd1-1,
				prefPattern2,
				prefPattern2->GetLength() - prefLength2,
				prefPattern2->GetLength() - prefEnd2-1);
      // create the new rules with the prefix additions to seedRules
      if (prefixAdditions) {
      int len = prefixAdditions->GetSize();
      for (int j = 0; j < len; j++) {
	RuleInProgress* newRule = new RuleInProgress(seedRule);
	newRule->AddToPrefix((Pattern*) prefixAdditions->GetItem(j),prefLength1, prefLength2);
	delete (Pattern*) prefixAdditions->GetItem(j);
	double ruleVal = newRule->Evaluate();
	//	cout << "Rule Value: " << ruleVal;
	if (maxVal > ruleVal) {
	  newRuleList->Add(newRule);
	  //	  cout << " rule added\n";
	} else {
	  delete newRule;
	  //	  cout << " rule deleted\n";
	}
      }
      delete prefixAdditions;
      }
    }
    seedRule = (RuleInProgress*) seedRuleList->GetNextItem();
    if (seedRule) {
      skipRule = 0;
      rule1 = seedRule->GetBaseRule1();
      rule2 = seedRule->GetBaseRule2();
      prefPattern1 = rule1->GetPrefixPattern();
      prefPattern2 = rule2->GetPrefixPattern();
      //      cout << "prefLength1: " << prefLength1 << "\n";
      //      cout << "prefPattern1->GetLength(): " << prefPattern1->GetLength() << "\n";
      //      cout << "prefLength2: " << prefLength2 << "\n";
      //      cout << "prefPattern2->GetLength(): " << prefPattern2->GetLength() << "\n";
      //      cout << "checking pattern lengths at bottom of while\n";
      if ((prefLength1 > prefPattern1->GetLength()) || 
	  (prefLength2 > prefPattern2->GetLength())) {
	skipRule = 1;
	//	cout << "prefix pattern too short\n";
      }
    }
  } // while seedRule
  return newRuleList;
}

DynamicPtrArray* Learner::EvenMatch(Pattern* pattern1, int start1, int end1,
				    Pattern* pattern2, int start2, int end2)
{
  int i = start1;
  int j = start2;
  PatternItem* item1;
  PatternItem* item2;
  PatternList* list1;
  PatternList* list2;
  int          isList1 = 0;
  int          isList2 = 0;
  DynamicPtrArray listOfItemLists;

  while (i <= end1) {
    item1 = pattern1->GetPatternItem(i);
    item2 = pattern2->GetPatternItem(j);
    if (2 == item1->GetType()) {
      list1 = (PatternList*) item1;
      isList1 = 1;
    } else isList1 = 0;
    if (2 == item2->GetType()) {
      list2 = (PatternList*) item2;
      isList2 = 1;
    } else isList2 = 0;
    Filler* wordConstraints = new Filler;
    wordConstraints->filler = NULL;
    wordConstraints->next = NULL;
    Filler* tagConstraints = new Filler;
    tagConstraints->filler= NULL;
    tagConstraints->next = NULL;
    Filler* semConstraints = new Filler;
    semConstraints->filler = NULL;
    semConstraints->next = NULL;
    // let's check out the words
    WordList* word1 = item1->GetWord();
    WordList* word2 = item2->GetWord();
    if ((NULL == word1) || (NULL == word2)) { // if either is unconstrained
      wordConstraints->filler = NULL;         // the generalization must be
    } else if (EqualFillers(word1,word2)) { // if the two are equal, the
      wordConstraints->filler = CopyWordList(word1); //ms gen is also equal
    } else if (WordListSubset(word1,word2)) { // if word1 is subset of word2
      wordConstraints->filler = CopyWordList(word2);
    } else if (WordListSubset(word2,word1)) { // if word2 is subset of word1
      wordConstraints->filler = CopyWordList(word1);
    } else { // there are two word generalizations - null constraint or disjunction
      wordConstraints->filler = NULL;
      Filler* filler2 = new Filler;
      filler2->next = NULL;
      wordConstraints->next = filler2;
      filler2->filler = UnionWordLists(word1,word2);
    }
    Filler* fillerIter = wordConstraints;
    WordList* iter = NULL;
    while (fillerIter) {
      iter = fillerIter->filler;
      //      if (!iter) {
      //	cout << "null word constraint\n";
      //      } else {
      //	cout << "[";
      //	while (iter) {
      //	  cout << iter->word;
      //	  if (iter->next)
      //	    cout << ",";
      //	  iter = iter->next;
      //	} 
      //	cout << "]\n";
      //      }
      fillerIter = fillerIter->next;
    }
    // now do the tags
    WordList* tag1 = item1->GetTag();
    WordList* tag2 = item2->GetTag();
    if ((NULL == tag1) || (NULL == tag2)) { // if either is unconstrained
      tagConstraints->filler = NULL;         // the generalization must be
    } else if (EqualFillers(tag1,tag2)) { // if the two are equal, the
      tagConstraints->filler = CopyWordList(tag1); //ms gen is also equal
    } else if (WordListSubset(tag1,tag2)) { // if tag1 is subset of tag2
      tagConstraints->filler = CopyWordList(tag2);
    } else if (WordListSubset(tag2,tag1)) { // if tag2 is subset of tag1
      tagConstraints->filler = CopyWordList(tag1);
    } else { // there are two tag generalizations - null constraint or disjunction
      tagConstraints->filler = NULL;
      Filler* filler2 = new Filler;
      filler2->next = NULL;
      tagConstraints->next = filler2;
      filler2->filler = UnionWordLists(tag1,tag2);
    }
    // OK now it's time for semantics
    WordList* sem1 = item1->GetSem();
    WordList* sem2 = item2->GetSem();
    char* new_class = NULL;
    if ((NULL == sem1) && (NULL == sem2)) { // no sem constraints -- check words
      if (word1 && !(word1->next) && word2 && !(word2->next)) { //we have words
	if (new_class = same_sem_class(word1->word,word2->word)) {
	  //they have a semantic class in common
	  semConstraints->filler = new WordList;
	  semConstraints->filler->word = new_class;
	  semConstraints->filler->next = NULL;
	} // else null constraint
      } // else null constraint
    } else if (sem1 && (NULL == sem2)) { 
      if (word2) { // if each word in word2 fits a semantic class in sem1
	WordList* iSem = sem1;
	WordList* iWord = word2;
	int semFound=1;
	while (semFound && iWord) {
	  iSem = sem1;
	  semFound = 0;
	  while (!semFound && iSem) {
	    semFound = sem_class(iWord->word,iSem->word);
	    iSem = iSem->next;
	  }
	  iWord = iWord->next;
	}
	if (semFound) {
	  // all of the words had semantic meanings corresponding to those in Sem1
	  semConstraints->filler = CopyWordList(sem1);
	} // else sem unconstrained
      }  // else sem unconstrained
    } else if ((NULL == sem1) && sem2) { 
      if (word1) { // 
	WordList* iSem = sem2;
	WordList* iWord = word1;
	int semFound = 1;
	while (semFound && iWord) {
	  iSem = sem2;
	  semFound = 0;
          while (!semFound && iSem) {
            semFound = sem_class(iWord->word,iSem->word);
            iSem = iSem->next;
          }
          iWord = iWord->next;
        }
        if (semFound) {
	  // all of the words had semantic meanings corresponding to those in Sem2
	  semConstraints->filler = CopyWordList(sem2);
	} //else sem unconstrained
      } //else sem unconstrained
    } else { // neither sem1 or sem2
      if (EqualFillers(sem1,sem2)) { // if the two are equal, the 
	semConstraints->filler = CopyWordList(sem1); // ms gen is also equal
      } else if (WordListSubset(sem1,sem2)) { // if sem1 is subset of sem2
	semConstraints->filler = CopyWordList(sem2);
      } else if (WordListSubset(sem2,sem1)) { // if sem2 is subset of sem1
	semConstraints->filler = CopyWordList(sem1);
      } else { // two generalizations - null constraint or disjunction
	semConstraints->filler = NULL;
	Filler* filler2 = new Filler;
	filler2->next = NULL;
	semConstraints->next = filler2;
	filler2->filler = UnionWordLists(sem1,sem2);
      }
    }
    // the negative constraints won't be generalized, always essentially union them
    WordList* wordNotConstraints = NULL;
    WordList* tagNotConstraints = NULL;
    WordList* semNotConstraints = NULL;
    // wordNot constraints
    WordList* wordNot1 = item1->GetWordNot();
    WordList* wordNot2 = item2->GetWordNot();
    if ((NULL == wordNot1) && (NULL == wordNot2)) {
      wordNotConstraints = NULL;
    } else  if ((NULL == wordNot1) && wordNot2) {
      wordNotConstraints = CopyWordList(wordNot2);
    } else if (wordNot1 && (NULL == wordNot2)) {
      wordNotConstraints = CopyWordList(wordNot1);
    } else {
      wordNotConstraints = UnionWordLists(wordNot1,wordNot2);
    }
    // tagNot constraints
    WordList* tagNot1 = item1->GetTagNot();
    WordList* tagNot2 = item2->GetTagNot();
    if ((NULL == tagNot1) && (NULL == tagNot2)) {
      tagNotConstraints = NULL;
    } else if ((NULL == tagNot1) && tagNot2) {
      tagNotConstraints = CopyWordList(tagNot2);
    } else if (tagNot1 && (NULL == tagNot2)) {
      tagNotConstraints = CopyWordList(tagNot1);
    } else {
      tagNotConstraints = UnionWordLists(tagNot1, tagNot2);
    }
    // semNot constraints
    WordList* semNot1 = item1->GetSemNot();
    WordList* semNot2 = item2->GetSemNot();
    if ((NULL == semNot1) && (NULL == semNot2)) {
      semNotConstraints = NULL;
    } else if ((NULL == semNot1) && semNot2) {
      semNotConstraints = CopyWordList(semNot2);
    } else if (semNot1 && (NULL == semNot2)) {
      semNotConstraints = CopyWordList(semNot1);
    } else {
      semNotConstraints = UnionWordLists(semNot1, semNot2);
    }
    // OK, now we have to put together a set of items
    int listLimit = 0;
    if (isList1 && isList2) {
      if (list1->GetListLimit() > list2->GetListLimit()) {
	listLimit = list1->GetListLimit();
      } else {
	listLimit = list2->GetListLimit();
      }
    } else if (isList1) {
      listLimit = list1->GetListLimit();
    } else if (isList2) {
      listLimit = list2->GetListLimit();
    }
    DynamicPtrArray* itemArray = new DynamicPtrArray;
    Filler* wordIterator = wordConstraints;
    Filler* tagIterator = tagConstraints;
    Filler* semIterator = semConstraints;
    while (wordIterator) {
      tagIterator = tagConstraints;
      while (tagIterator) {
	semIterator = semConstraints;
	while (semIterator) {
	  if (listLimit > 0) {
	    // creating a PatternList
	    PatternList* listItem = new PatternList(listLimit,
						    wordIterator->filler,
						    tagIterator->filler,
						    semIterator->filler,
						    wordNotConstraints,
						    tagNotConstraints,
						    semNotConstraints);
	    itemArray->Add(listItem);
	  } else {
	    PatternItem* item = new PatternItem(wordIterator->filler,
					       tagIterator->filler,
					       semIterator->filler,
					       wordNotConstraints,
					       tagNotConstraints,
					       semNotConstraints);
	    itemArray->Add(item);
	  }
	  semIterator = semIterator->next;
	}
	tagIterator = tagIterator->next;
      }
      wordIterator = wordIterator->next;
    }
    // Now Let's delete those constraints
    WordList* temp1;
    WordList* temp2;
    Filler* fillerTemp;
    while (wordConstraints) {
      temp1 = wordConstraints->filler;
      while (temp1) {
	temp2 = temp1->next;
	delete temp1;
	temp1 = temp2;
      }
      fillerTemp = wordConstraints->next;
      delete wordConstraints;
      wordConstraints = fillerTemp;
    }
    while (tagConstraints) {
      temp1 = tagConstraints->filler;
      while (temp1) {
	temp2 = temp1->next;
	delete temp1;
	temp1 = temp2;
      }
      fillerTemp = tagConstraints->next;
      delete tagConstraints;
      tagConstraints = fillerTemp;
    }
    while (semConstraints) {
      temp1 = semConstraints->filler;
      while (temp1) {
	temp2 = temp1->next;
	delete temp1;
	temp1 = temp2;
      }
      fillerTemp = semConstraints->next;
      delete semConstraints;
      semConstraints = fillerTemp;
    }
    while (semNotConstraints) {
      temp1 = semNotConstraints->next;
      delete semNotConstraints;
      semNotConstraints = temp1;
    }
    while (tagNotConstraints) {
      temp1 = tagNotConstraints->next;
      delete tagNotConstraints;
      tagNotConstraints = temp1;
    }
    while (wordNotConstraints) {
      temp1 = wordNotConstraints->next;
      delete wordNotConstraints;
      wordNotConstraints = temp1;
    }
    int templen = itemArray->GetSize();
    //    cout << "produced a list of " << templen << "items\n";
    //    for (int k = 0; k < templen; k++) {
    //      ((PatternItem*) itemArray->GetItem(k))->Write(cout);
    //      cout << "\n";
    //    }
    listOfItemLists.Add(itemArray);
    i++;
    j++;
  }
  // OK, now we need to take the array of item lists and create
  // an array of patterns
  DynamicPtrArray* patternArray = new DynamicPtrArray;
  int patternLength = listOfItemLists.GetSize();
  //  cout << "patternLength is " << patternLength << "\n";
  //  for (int k = 0; k < patternLength; k++) {
  //    int numItems = ((DynamicPtrArray*) listOfItemLists.GetItem(k))->GetSize();
  //    cout << "number of items for position " << k << " is " << numItems << "\n";
  //  }
  // allocate an array of integers the same length as the pattern
  int* indexArray = (int*) malloc(patternLength * sizeof(int));
  int levelIndex;
  for (levelIndex = 0; levelIndex < patternLength; levelIndex++) {
    indexArray[levelIndex] = 0;
  }
  Pattern* newPattern = NULL;
  while (indexArray[0] < 
	 ((DynamicPtrArray*) listOfItemLists.GetItem(0))->GetSize()) {
    // create a Pattern
    newPattern = new Pattern;
    for (levelIndex = 0; levelIndex < patternLength; levelIndex++) {
      PatternItem* item1 = 
	(PatternItem*) ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetItem(indexArray[levelIndex]);
      PatternItem* item2 = NULL;
      PatternList* list2 = NULL;
      if (1 == item1->GetType()) {
	item2 = new PatternItem(item1);
	newPattern->AddItem(item2);
      } else {
	list2 = new PatternList((PatternList*) item1);
	newPattern->AddItem(list2);
      }
    }
    patternArray->Add(newPattern);
    levelIndex = patternLength-1;
    indexArray[levelIndex]++;
    while ((indexArray[levelIndex] >= 
	   ((DynamicPtrArray*)listOfItemLists.GetItem(levelIndex))->GetSize())
	   && (levelIndex > 0))
      { 
	indexArray[levelIndex] = 0;
	levelIndex--;
	indexArray[levelIndex]++;
      }
  }
  for (levelIndex = 0; levelIndex < patternLength; levelIndex++) {
    int ndxLim = ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetSize();
    for (i = 0; i < ndxLim; i++) {
      delete (PatternItem*) ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetItem(i);
    }
    delete (DynamicPtrArray*) listOfItemLists.GetItem(levelIndex);
  }
  free(indexArray);
  return patternArray;
}

typedef struct AnchorPair {
  int index1;
  int index2;
  AnchorPair* next;
} AnchorPair;

AnchorPair* FindAnchors(Pattern* pattern1, int start1, int end1,
			Pattern* pattern2, int start2, int end2)
{
  // pattern1 has the longer portion
  int diff = (end1-start1) - (end2-start2);
  AnchorPair* result = NULL;
  AnchorPair* lastAnchor = NULL;
  int i = start2;
  int j = start1;
  int jLim;
  PatternItem* item1;
  PatternItem* item2;
  int anchorFound = 0;
  while (i <= end2) {
    anchorFound = 0;
    jLim = i - start2 + start1 +diff;
    if (jLim > end1) jLim = end1;
    while ((j <= jLim) && !anchorFound) {
      item1 = pattern1->GetPatternItem(j);
      item2 = pattern2->GetPatternItem(i);
      if ((1 == item1->GetType()) && (1 == item2->GetType()) &&
          (EqualFillers(item1->GetWord(), item2->GetWord())) &&
          (EqualFillers(item1->GetTag(), item2->GetTag())) &&
          (EqualFillers(item1->GetSem(), item2->GetSem())) &&
          (EqualFillers(item1->GetWordNot(), item2->GetWordNot())) &&
          (EqualFillers(item1->GetTagNot(), item2->GetTagNot())) &&
	  (EqualFillers(item1->GetSemNot(), item2->GetSemNot()))) {
	anchorFound = 1;
	AnchorPair* anAnchor = new AnchorPair;
	anAnchor->index1 = j;
        anAnchor->index2 = i;
        anAnchor->next = NULL;
        if (lastAnchor) {
          lastAnchor->next = anAnchor;
          lastAnchor = anAnchor;
        } else {
	  result = anAnchor;
          lastAnchor = anAnchor;
        }
      } // exact match
      if (!anchorFound) j++;
    } // while ( j <= jLim) && !anchorFound
    i++;
    if (anchorFound) {
      j++;
    } else {
      j = start1 + (i - start2);
      if (lastAnchor && (j <= lastAnchor->index1)) {
        j = lastAnchor->index1 + 1;
      }
    }
  }
  return result;
}

typedef struct IndexedItem {
  int start1;
  int end1;
  int index2;
  PatternItem* item;
} ItemList;


DynamicPtrArray* MakePatternLists(Pattern* pattern, int start, int end)
{
  DynamicPtrArray listOfItemLists;
  PatternItem* anItem;
  PatternList* aList;
  int listLen = 0;
  Filler* wordConstraints = new Filler;
  wordConstraints->filler = NULL;
  wordConstraints->next = NULL;
  Filler* tagConstraints = new Filler;
  tagConstraints->filler= NULL;
  tagConstraints->next = NULL;
  Filler* semConstraints = new Filler;
  semConstraints->filler = NULL;
  semConstraints->next = NULL;
  WordList* wordNotConstraints = NULL;
  WordList* tagNotConstraints = NULL;
  WordList* semNotConstraints = NULL;
  // first, we need to initialize everything from the first item
  anItem = pattern->GetPatternItem(start);
  // find out if this item is a list
  if (2 == anItem->GetType()) {
    // anItem is a list
    listLen = ((PatternList*) anItem)->GetListLimit();
  } else {
    listLen = 1;
  }
  // Now, just copy all the constraints
  if (NULL != anItem->GetWord()) {
    wordConstraints->filler = CopyWordList(anItem->GetWord());
  } 
  if (NULL != anItem->GetTag()) {
    tagConstraints->filler = CopyWordList(anItem->GetTag());
  }
  if (NULL != anItem->GetSem()) {
    semConstraints->filler = CopyWordList(anItem->GetSem());
  }
  if (NULL != anItem->GetWordNot()) {
    wordNotConstraints = CopyWordList(anItem->GetWordNot());
  }
  if (NULL != anItem->GetTagNot()) {
    tagNotConstraints = CopyWordList(anItem->GetTagNot());
  } 
  if (NULL != anItem->GetSemNot()) {
    semNotConstraints = CopyWordList(anItem->GetSemNot());
  }
  // OK, now we have initalized things from the first item, now we need
  // to loop through the rest of the items and incorporate their info
  for (int i = start+1; i < end; i++) {
    anItem = pattern->GetPatternItem(i);
    // find out if this item is a list
    if (2 == anItem->GetType()) {
      // anItem is a list
      listLen = listLen + ((PatternList*) anItem)->GetListLimit();
    } else {
      listLen++;
    }
    if (NULL != wordConstraints->filler) {
      // doing the wordConstraints
      if (NULL == anItem->GetWord()) {
	// generalization should be null constraint -- delete existing constraints
	WordList* a;
	WordList* b;
	a = wordConstraints->filler;
	while (a) {
	  b = a->next;
	  delete a;
	  a = b;
	}
	wordConstraints->filler = NULL;
	if (wordConstraints->next) {
	  delete wordConstraints->next;
	  wordConstraints->next = NULL;
	}
      } else { // we need to generalize the word lists appropriately
	if (wordConstraints->next) {
	  // don't need to check whether we need to create the null constraint
	  // just use the destructive union
	  DestUnionWordLists(wordConstraints->filler,anItem->GetWord());
	} else { // we need to deal with each possibility
	  if (!WordListSubset(wordConstraints->filler,anItem->GetWord())) {
	    // need to do the union and create a null constraint
	    DestUnionWordLists(wordConstraints->filler,anItem->GetWord());
	    Filler* newConstraint = new Filler;
	    newConstraint->filler = NULL;
	    newConstraint->next = NULL;
	    wordConstraints->next = newConstraint;
	  }
	}
      }
    }
    // now do the tags
    if (NULL != tagConstraints->filler) {
      // doing the tagConstraints
      if (NULL == anItem->GetTag()) {
	// generalization should be null constraint -- delete existing constraints
	WordList* a;
        WordList* b;
        a = tagConstraints->filler;
	while (a) {
          b = a->next;
          delete a;
          a = b;
        }
	tagConstraints->filler = NULL;
	if (tagConstraints->next) {
	  delete tagConstraints->next;
	  tagConstraints->next = NULL;
	}
      } else { // we need to generalize the word lists appropriately
	if (tagConstraints->next) {
	  // don't need to check whether we need to create the null constraint
          // just use the destructive union
          DestUnionWordLists(tagConstraints->filler,anItem->GetTag());
	} else { // we need to deal with each possibility
	  if (!WordListSubset(tagConstraints->filler,anItem->GetTag())) {
	    // need to do the union and create a null constraint
	    DestUnionWordLists(tagConstraints->filler,anItem->GetTag());
	    Filler* newConstraint = new Filler;
	    newConstraint->filler = NULL;
	    newConstraint->next = NULL;
	    tagConstraints->next = newConstraint;
	  }
	}
      }
    }
    // now do the sem classes
    // for now, at least, we're going to do these just like the word and tag
    if (NULL != semConstraints->filler) {
      // doing the semConstraints
      if (NULL == anItem->GetSem()) {
	// generalization should be null constraint -- delete existing constraints
	WordList* a;
        WordList* b;
        a = semConstraints->filler;
        while (a) {
          b = a->next;
          delete a;
          a = b;
        }
	semConstraints->filler = NULL;
	if (semConstraints->next) {
	  delete semConstraints->next;
	  semConstraints->next = NULL;
	}
      } else { // we need to generalize the word lists appropriately
	if (semConstraints->next) {
	  // don't need to check whether we need to create the null constraint
          // just use the destructive union
          DestUnionWordLists(semConstraints->filler,anItem->GetSem());
	} else { // we need to deal with each possibility
	  if (!WordListSubset(semConstraints->filler,anItem->GetSem())) {
	    // need to do the union and create a null constraint
            DestUnionWordLists(semConstraints->filler,anItem->GetSem());
	    Filler* newConstraint = new Filler;
	    newConstraint->filler = NULL;
            newConstraint->next = NULL;
	    semConstraints->next = newConstraint;
	  }
	}
      }
    }
    // now do negative constraints
    if (anItem->GetWordNot()) {
      if (wordNotConstraints) {
	DestUnionWordLists(wordNotConstraints,anItem->GetWordNot());
      } else {
	wordNotConstraints = CopyWordList(anItem->GetWordNot());
      }
    }
    if (anItem->GetTagNot()) {
      if (tagNotConstraints) {
	DestUnionWordLists(tagNotConstraints,anItem->GetTagNot());
      } else {
	tagNotConstraints = CopyWordList(anItem->GetTagNot());
      }
    }
    if (anItem->GetSemNot()) {
      if (semNotConstraints) {
	DestUnionWordLists(semNotConstraints,anItem->GetSemNot());
      } else {
	semNotConstraints = CopyWordList(anItem->GetSemNot());
      }
    }
  }
  // now create the list(s)
  DynamicPtrArray* itemArray = new DynamicPtrArray;
  Filler* wordIterator = wordConstraints;
  Filler* tagIterator = tagConstraints;
  Filler* semIterator = semConstraints;
  while (wordIterator) {
    tagIterator = tagConstraints;
    while (tagIterator) {
      semIterator = semConstraints;
      while (semIterator) {
	PatternList* listItem = new PatternList(listLen, wordIterator->filler,
					       tagIterator->filler,
					       semIterator->filler,
					       wordNotConstraints,
					       tagNotConstraints,
					       semNotConstraints);
	itemArray->Add(listItem);
	semIterator = semIterator->next;
      } 
      tagIterator = tagIterator->next;
    } 
    wordIterator = wordIterator->next;
  }
  // Now let's delete those constraints
  WordList* temp1;
  WordList* temp2;
  Filler* fillerTemp;
  while (wordConstraints) {
    temp1 = wordConstraints->filler;
    while (temp1) {
      temp2 = temp1->next;
      delete temp1;
      temp1 = temp2;
    }
    fillerTemp = wordConstraints->next;
    delete wordConstraints;
    wordConstraints = fillerTemp;
  }
  while (tagConstraints) {
    temp1 = tagConstraints->filler;
    while (temp1) {
      temp2 = temp1->next;
      delete temp1;
      temp1 = temp2;
    }
    fillerTemp = tagConstraints->next;
    delete tagConstraints;
    tagConstraints = fillerTemp;
  }
  while (semConstraints) {
    temp1 = semConstraints->filler;
    while (temp1) {
      temp2 = temp1->next;
      delete temp1;
      temp1 = temp2;
    }
    fillerTemp = semConstraints->next;
    delete semConstraints;
    semConstraints = fillerTemp;
  }
  while (semNotConstraints) {
    temp1 = semNotConstraints->next;
    delete semNotConstraints;
    semNotConstraints = temp1;
  }
  while (tagNotConstraints) {
    temp1 = tagNotConstraints->next;
    delete tagNotConstraints;
    tagNotConstraints = temp1;
  }
  while (wordNotConstraints) {
    temp1 = wordNotConstraints->next;
    delete wordNotConstraints;
    wordNotConstraints = temp1;
  }
  return itemArray;
}

DynamicPtrArray* GeneralMatch(Pattern* pattern1, int start1, int end1,
			      Pattern* pattern2, int start2, int end2)
{
  // Let's just make the single unconstrained list
  int listLimit = end1 - start1 + 1;
  DynamicPtrArray* patternArray = new DynamicPtrArray;
  Pattern* newPattern = new Pattern;
  PatternList* newlist = new PatternList(listLimit,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 NULL);
  newPattern->AddItem(newlist);
  patternArray->Add(newPattern);
  return patternArray;
}

DynamicPtrArray* TrueUnevenMatch(Pattern* pattern1, int start1, int end1,
                                       Pattern* pattern2, int start2, int end2)
{
  PatternItem* item1;
  PatternItem* item2;
  PatternList* list1;
  PatternList* list2;
  int          isList1 = 0;
  int          isList2 = 0;
  DynamicPtrArray listOfItemLists;  

  int diff = (end1 - start1) - (end2 - start2);
  // here's where we're actually going to use those indexed items
  int minStart;
  int maxStart;
  int minEnd;
  int maxEnd;
  int j;
  int jLen = end2 - start2 + 1;
  int iLen = end1 - start1 + 1;

  if (((iLen > 2) && (diff > 2)) || ((iLen > 5) && (diff > 1)) || (iLen > 6)) {
    return GeneralMatch(pattern1,start1,end1,pattern2,start2,end2);
  }
  for (int ndx = 0; ndx < jLen; ndx++) {
    DynamicPtrArray* itemArray = new DynamicPtrArray;
    j = start2 + ndx;
    minStart = start1 + ndx;
    if (ndx) {
      maxStart = minStart + diff;
    } else {
      maxStart = minStart;
    }
    maxEnd = minStart + diff;
    if (maxEnd < end1) {
      minEnd = minStart;
    } else {
      minEnd = maxEnd;
    }
    int botEnd;
    for (int iStart = minStart; iStart <= maxStart; iStart++) {
      if (minEnd > iStart) {
	botEnd = minEnd; 
      } else {
	botEnd = iStart;
      }
      for (int iEnd = botEnd; iEnd <= maxEnd; iEnd++) {
	int iLen = 0;
	int jLen = 0;
	item1 = pattern1->GetPatternItem(iStart);
	item2 = pattern2->GetPatternItem(j);
	if (2 == item2->GetType()) {
	  list2 = (PatternList*) item2;
	  isList2 = 1;
	  jLen = list2->GetListLimit();
	} else {
	  isList2 = 0;
	  jLen = 1;
	}
	if (2 == item1->GetType()) {
	  list1 = (PatternList*) item1;
	  isList1 = 1;
	  iLen = list1->GetListLimit();
	} else {
	  isList1 = 0;
	  iLen = 1;
	}
	Filler* wordConstraints = new Filler;
	wordConstraints->filler = NULL;
	wordConstraints->next = NULL;
	Filler* tagConstraints = new Filler;
	tagConstraints->filler= NULL;
	tagConstraints->next = NULL;
	Filler* semConstraints = new Filler;
	semConstraints->filler = NULL;
	semConstraints->next = NULL;
	WordList* word1 = item1->GetWord();
	WordList* word2 = item2->GetWord();
	if ((NULL == word1) || (NULL == word2)) { // if either is unconstrained
	  wordConstraints->filler = NULL;         // the generalization must be
	} else if (EqualFillers(word1,word2)) { // if the two are equal, the
	  wordConstraints->filler = CopyWordList(word1); //ms gen is also equal
	} else if (WordListSubset(word1,word2)) { // if word1 is subset of word2
	  wordConstraints->filler = CopyWordList(word2);
	} else if (WordListSubset(word2,word1)) { // if word2 is subset of word1
	  wordConstraints->filler = CopyWordList(word1);
	} else { // there are two word generalizations - null constraint or disjunction
	  Filler* filler2 = new Filler;
	  filler2->filler = NULL;
	  filler2->next = NULL;
	  wordConstraints->next = filler2;
	  wordConstraints->filler = UnionWordLists(word1,word2);
	}
	// now do the tags
	WordList* tag1 = item1->GetTag();
	WordList* tag2 = item2->GetTag();
	if ((NULL == tag1) || (NULL == tag2)) { // if either is unconstrained
	  tagConstraints->filler = NULL;         // the generalization must be
	} else if (EqualFillers(tag1,tag2)) { // if the two are equal, the
	  tagConstraints->filler = CopyWordList(tag1); //ms gen is also equal
	} else if (WordListSubset(tag1,tag2)) { // if tag1 is subset of tag2
	  tagConstraints->filler = CopyWordList(tag2);
	} else if (WordListSubset(tag2,tag1)) { // if tag2 is subset of tag1
	  tagConstraints->filler = CopyWordList(tag1);
	} else { // there are two tag generalizations - null constraint or disjunction
	  tagConstraints->filler = NULL;
	  Filler* filler2 = new Filler;
	  filler2->filler = NULL;
	  filler2->next = NULL;
	  tagConstraints->next = filler2;
	  tagConstraints->filler = UnionWordLists(tag1,tag2);
	}
	// OK now it's time for semantics
	WordList* sem1 = item1->GetSem();
	WordList* sem2 = item2->GetSem();
	char* new_class = NULL;
	if ((NULL == sem1) && (NULL == sem2)) { // no sem constraints -- check words
	  if (word1 && !(word1->next) && word2 && !(word2->next)) { //we have words
	    if (new_class = same_sem_class(word1->word,word2->word)) {
	      //they have a semantic class in common
	      semConstraints->filler = new WordList;
	      semConstraints->filler->word = new_class;
	      semConstraints->filler->next = NULL;
	    } // else null constraint
	  } // else null constraint
	} else if (sem1 && (NULL == sem2)) {
	  if (word2) { // if each word in word2 fits a semantic class in sem1
	    WordList* iSem = sem1;
	    WordList* iWord = word2;
	    int semFound=1;
	    while (semFound && iWord) {
	      iSem = sem1;
	      semFound = 0;
	      while (!semFound && iSem) {
		semFound = sem_class(iWord->word,iSem->word);
		iSem = iSem->next;
	      }
	      iWord = iWord->next;
	    }
	    if (semFound) {
	      // all of the words had semantic meanings corresponding to those in Sem1
	      semConstraints->filler = CopyWordList(sem1);
	    } // else sem unconstrained
	  }  // else sem unconstrained
	} else if ((NULL == sem1) && sem2) {
	  if (word1) { //
	    WordList* iSem = sem2;
	    WordList* iWord = word1;
	    int semFound = 1;
	    while (semFound && iWord) {
	      iSem = sem2;
	      semFound = 0;
	      while (!semFound && iSem) {
		semFound = sem_class(iWord->word,iSem->word);
		iSem = iSem->next;
	      }
	      iWord = iWord->next;
	    }
	    if (semFound) {
	      // all of the words had semantic meanings corresponding to those in Sem2
	      semConstraints->filler = CopyWordList(sem2);
	    } //else sem unconstrained
	  } //else sem unconstrained
	} else { // neither sem1 or sem2
	  if (EqualFillers(sem1,sem2)) { // if the two are equal, the
	    semConstraints->filler = CopyWordList(sem1); // ms gen is also equal
	  } else if (WordListSubset(sem1,sem2)) { // if sem1 is subset of sem2
	    semConstraints->filler = CopyWordList(sem2);
	  } else if (WordListSubset(sem2,sem1)) { // if sem2 is subset of sem1
	    semConstraints->filler = CopyWordList(sem1);
	  } else { // two generalizations - null constraint or disjunction
	    semConstraints->filler = NULL;
	    Filler* filler2 = new Filler;
	    filler2->filler = NULL;
	    filler2->next = NULL;
	    semConstraints->next = filler2;
	    semConstraints->filler = UnionWordLists(sem1,sem2);
	  }
	}
	// the negative constraints who't be generalized, always essentially union them
	WordList* wordNotConstraints = NULL;
	WordList* tagNotConstraints = NULL;
	WordList* semNotConstraints = NULL;
	// wordNot constraints
	WordList* wordNot1 = item1->GetWordNot();
	WordList* wordNot2 = item2->GetWordNot();
	if ((NULL == wordNot1) && (NULL == wordNot2)) {
	  wordNotConstraints = NULL;
	} else  if ((NULL == wordNot1) && wordNot2) {
	  wordNotConstraints = CopyWordList(wordNot2);
	} else if (wordNot1 && (NULL == wordNot2)) {
	  wordNotConstraints = CopyWordList(wordNot1);
	} else {
	  wordNotConstraints = UnionWordLists(wordNot1,wordNot2);
	}
	// tagNot constraints
	WordList* tagNot1 = item1->GetTagNot();
	WordList* tagNot2 = item2->GetTagNot();
	if ((NULL == tagNot1) && (NULL == tagNot2)) {
	  tagNotConstraints = NULL;
	} else if ((NULL == tagNot1) && tagNot2) {
	  tagNotConstraints = CopyWordList(tagNot2);
	} else if (tagNot1 && (NULL == tagNot2)) {
	  tagNotConstraints = CopyWordList(tagNot1);
	} else {
	  tagNotConstraints = UnionWordLists(tagNot1, tagNot2);
	}
	// semNot constraints
	WordList* semNot1 = item1->GetSemNot();
	WordList* semNot2 = item2->GetSemNot();
	if ((NULL == semNot1) && (NULL == semNot2)) {
	  semNotConstraints = NULL;
	} else if ((NULL == semNot1) && semNot2) {
	  semNotConstraints = CopyWordList(semNot2);
	} else if (semNot1 && (NULL == semNot2)) {
	  semNotConstraints = CopyWordList(semNot1);
	} else {
	  semNotConstraints = UnionWordLists(semNot1, semNot2);
	}
	for (int i = iStart+1; i <= iEnd; i++) {
	  // do this for each additional item in the i set
	  item1 = pattern1->GetPatternItem(i);
	  if (2 == item1->GetType()) {
	    list1 = (PatternList*) item1;
	    isList1 = 1;
	    iLen = iLen + list1->GetListLimit();
	  } else {
	    isList1 = 0;
	    iLen++;
	  }
	  // handle words
	  if (NULL != wordConstraints->filler) {
	    // doing the wordConstraints
	    if (NULL == item1->GetWord()) {
	      // generalization should be null constraint -- delete existing constraints
	      WordList* a;
	      WordList* b;
	      a = wordConstraints->filler;
	      while (a) {
		b = a->next;
		delete a;
		a = b;
	      }
	      wordConstraints->filler = NULL;
	      if (wordConstraints->next) {
		delete wordConstraints->next;
		wordConstraints->next = NULL;
	      }
	    } else { // we need to generalize the word lists appropriately
	      if (wordConstraints->next) {
		// don't need to check whether we need to create the null constraint
		// just use the destructive union
		DestUnionWordLists(wordConstraints->filler,item1->GetWord());
	      } else { // we need to deal with each possibility
		if (!WordListSubset(wordConstraints->filler,item1->GetWord())) {
		  // need to do the union and create a null constraint
		  DestUnionWordLists(wordConstraints->filler,item1->GetWord());
		  Filler* newConstraint = new Filler;
		  newConstraint->filler = NULL;
		  newConstraint->next = NULL;
		  wordConstraints->next = newConstraint;
		}
	      }
	    }
	  }
	  // now do the tags
	  if (NULL != tagConstraints->filler) {
	    // doing the tagConstraints
	    if (NULL == item1->GetTag()) {
	      // generalization should be null constraint -- delete existing constraints
	      WordList* a;
	      WordList* b;
	      a = tagConstraints->filler;
	      while (a) {
		b = a->next;
		delete a;
		a = b;
	      }
	      tagConstraints->filler = NULL;
	      if (tagConstraints->next) {
		delete tagConstraints->next;
		tagConstraints->next = NULL;
	      }
	    } else { // we need to generalize the word lists appropriately
	      if (tagConstraints->next) {
		// don't need to check whether we need to create the null constraint
		// just use the destructive union
		DestUnionWordLists(tagConstraints->filler,item1->GetTag());
	      } else { // we need to deal with each possibility
		if (!WordListSubset(tagConstraints->filler,item1->GetTag())) {
		  // need to do the union and create a null constraint
		  DestUnionWordLists(tagConstraints->filler,item1->GetTag());
		  Filler* newConstraint = new Filler;
		  newConstraint->filler = NULL;
		  newConstraint->next = NULL;
		  tagConstraints->next = newConstraint;
		}
	      }
	    }
	  }
	  // now do the sem classes
	  // for now, at least, we're going to do these just like the word and tag
	  if (NULL != semConstraints->filler) {
	    // doing the semConstraints
	    if (NULL == item1->GetSem()) {
	      // generalization should be null constraint -- delete existing constraints
	      WordList* a;
	      WordList* b;
	      a = semConstraints->filler;
	      while (a) {
		b = a->next;
		delete a;
		a = b;
	      }
	      semConstraints->filler = NULL;
	      if (semConstraints->next) {
		delete semConstraints->next;
		semConstraints->next = NULL;
	      }
	    } else { // we need to generalize the word lists appropriately
	      if (semConstraints->next) {
		// don't need to check whether we need to create the null constraint
		// just use the destructive union
		DestUnionWordLists(semConstraints->filler,item1->GetSem());
	      } else { // we need to deal with each possibility
		if (!WordListSubset(semConstraints->filler,item1->GetSem())) {
		  // need to do the union and create a null constraint
		  DestUnionWordLists(semConstraints->filler,item1->GetSem());
		  Filler* newConstraint = new Filler;
		  newConstraint->filler = NULL;
		  newConstraint->next = NULL;
		  semConstraints->next = newConstraint;
		}
	      }
	    }
	  }
	  // now do negative constraints
	  if (item1->GetWordNot()) {
	    if (wordNotConstraints) {
	      DestUnionWordLists(wordNotConstraints,item1->GetWordNot());
	    } else {
	      wordNotConstraints = CopyWordList(item1->GetWordNot());
	    }
	  }
	  if (item1->GetTagNot()) {
	    if (tagNotConstraints) {
	      DestUnionWordLists(tagNotConstraints,item1->GetTagNot());
	    } else {
	      tagNotConstraints = CopyWordList(item1->GetTagNot());
	    }
	  }
	  if (item1->GetSemNot()) {
	    if (semNotConstraints) {
	      DestUnionWordLists(semNotConstraints,item1->GetSemNot());
	    } else {
	      semNotConstraints = CopyWordList(item1->GetSemNot());
	    }
	  }
	}
	int listLimit;
	if ((iLen > 1) || (jLen > 1)) {
	  if (iLen > jLen)
	    listLimit = iLen;
	  else
	    listLimit = jLen;
	} else if (isList1 || isList2) {
	  listLimit = 1;
	} else {
	  listLimit = 0;
	}
	Filler* wordIterator = wordConstraints;
	Filler* tagIterator = tagConstraints;
	Filler* semIterator = semConstraints;
	while (wordIterator) {
	  tagIterator = tagConstraints;
	  while (tagIterator) {
	    semIterator = semConstraints;
	    while (semIterator) {
	      IndexedItem* ndxItem = new IndexedItem;
	      ndxItem->start1 = iStart;
	      ndxItem->end1 = iEnd;
	      ndxItem->index2 = j;
	      if (listLimit > 0) {
		// creating a PatternList
		PatternList* listItem = new PatternList(listLimit,
							wordIterator->filler,
							tagIterator->filler,
							semIterator->filler,
							wordNotConstraints,
							tagNotConstraints,
							semNotConstraints);
		ndxItem->item = listItem;
	      } else {
		PatternItem* item = new PatternItem(wordIterator->filler,
						    tagIterator->filler,
						    semIterator->filler,
						    wordNotConstraints,
						    tagNotConstraints,
						    semNotConstraints);
		ndxItem->item = item;
	      }
	      itemArray->Add(ndxItem);
	      semIterator = semIterator->next;
	    }
	    tagIterator = tagIterator->next;
	  }
	  wordIterator = wordIterator->next;
	}
	WordList* temp1;
	WordList* temp2;
	Filler* fillerTemp;
	while (wordConstraints) {
	  temp1 = wordConstraints->filler;
	  while (temp1) {
	    temp2 = temp1->next;
	    delete temp1;
	    temp1 = temp2;
	  }
	  fillerTemp = wordConstraints->next;
	  delete wordConstraints;
	  wordConstraints = fillerTemp;
	}
	while (tagConstraints) {
	  temp1 = tagConstraints->filler;
	  while (temp1) {
	    temp2 = temp1->next;
	    delete temp1;
	    temp1 = temp2;
	  }
	  fillerTemp = tagConstraints->next;
	  delete tagConstraints;
	  tagConstraints = fillerTemp;
	}
	while (semConstraints) {
	  temp1 = semConstraints->filler;
	  while (temp1) {
	    temp2 = temp1->next;
	    delete temp1;
	    temp1 = temp2;
	  }
	  fillerTemp = semConstraints->next;
	  delete semConstraints;
	  semConstraints = fillerTemp;
	}
	while (semNotConstraints) {
	  temp1 = semNotConstraints->next;
	  delete semNotConstraints;
	  semNotConstraints = temp1;
	}
	while (tagNotConstraints) {
	  temp1 = tagNotConstraints->next;
	  delete tagNotConstraints;
	  tagNotConstraints = temp1;
	}
	while (wordNotConstraints) {
	  temp1 = wordNotConstraints->next;
	  delete wordNotConstraints;
	  wordNotConstraints = temp1;
	}
      } // for iStart
    } // for iEnd
    listOfItemLists.Add(itemArray);
  } // for ndx
  // OK, now we have list of item lists, but items are indexed
  // we need to take this array and create an array of patterns
  DynamicPtrArray* patternArray = new DynamicPtrArray;
  int patternLength = listOfItemLists.GetSize();
  // allocate an array of integers the same length as the pattern
  int* indexArray = (int*) malloc(patternLength * sizeof(int));
  int levelIndex;
  for (levelIndex = 0; levelIndex < patternLength; levelIndex++) {
    indexArray[levelIndex] = 0;
  }
  if (patternLength > 1) {
    levelIndex = patternLength-1;
    // we need to test index compatibility
    while ((levelIndex > 0) && (levelIndex < patternLength) && 
	   (indexArray[levelIndex] >= ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetSize())) {
      indexArray[levelIndex] = 0;
      levelIndex--;
      indexArray[levelIndex]++;
    }
    // avoid useless things
    //    if (0 == levelIndex) levelIndex++;
    levelIndex = 1;
    while ((levelIndex < patternLength) && (levelIndex > 0) && 
	   (indexArray[0] < ((DynamicPtrArray*) listOfItemLists.GetItem(0))->GetSize())) {
      
      DynamicPtrArray* backArray = 
	(DynamicPtrArray*)listOfItemLists.GetItem(levelIndex-1);
      IndexedItem* backItem = 
	(IndexedItem*) backArray->GetItem(indexArray[levelIndex-1]);
      if (!backItem) {
	cout <<  "backItem NULL levelIndex " << levelIndex << "\n";
	cout << indexArray[levelIndex-1] << "\n";
      }
      DynamicPtrArray* curArray =
	(DynamicPtrArray*) listOfItemLists.GetItem(levelIndex);
      IndexedItem* curItem =
	(IndexedItem*) curArray->GetItem(indexArray[levelIndex]);
      if (!curItem) {
	cout << "curItem NULL levelIndex " << levelIndex << "\n";
	cout << indexArray[levelIndex] << "\n";
      }
      while ((levelIndex > 0) && (backItem->end1+1 != curItem->start1 )) {
	indexArray[levelIndex]++;
	if (indexArray[levelIndex] >= curArray->GetSize()) {
	  //	cout << "indexArray[" << levelIndex << "] went too high\n";
	  indexArray[levelIndex] = 0;
	  levelIndex--;
	  indexArray[levelIndex]++;
	  while ((levelIndex > 0) && 
		 (indexArray[levelIndex] >= ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetSize())) {
	    indexArray[levelIndex] = 0;
	    levelIndex--;
	    indexArray[levelIndex]++;
	  }
	  levelIndex--;
	  if (levelIndex == -1) levelIndex = 0;
	} else {
	  curItem = (IndexedItem*) curArray->GetItem(indexArray[levelIndex]);
	}
      }
      levelIndex++;
    }
  }
  Pattern* newPattern = NULL;
  while (indexArray[0] < 
	 ((DynamicPtrArray*) listOfItemLists.GetItem(0))->GetSize()) {
    // create a Pattern
    newPattern = new Pattern;
    for (levelIndex = 0; levelIndex < patternLength; levelIndex++) {
      DynamicPtrArray* curArray = (DynamicPtrArray*) listOfItemLists.GetItem(levelIndex);
      PatternItem* item1 = ((IndexedItem*) curArray->GetItem(indexArray[levelIndex]))->item;
      PatternItem* item2 = NULL;
      PatternList* list2 = NULL;
      if (1 == item1->GetType()) {
        item2 = new PatternItem(item1);
        newPattern->AddItem(item2);
      } else {
        list2 = new PatternList((PatternList*) item1);
        newPattern->AddItem(list2);
      }
    }patternArray->Add(newPattern);
    levelIndex = patternLength-1;
    indexArray[levelIndex]++;
    while ((indexArray[levelIndex] >=
	    ((DynamicPtrArray*)listOfItemLists.GetItem(levelIndex))->GetSize())
           && (levelIndex > 0))
      {
        indexArray[levelIndex] = 0;
        levelIndex--;
	indexArray[levelIndex]++;
      }
    // now we need to avoid useless things
    //if (0 == levelIndex) levelIndex++;
    //levelIndex = patternLength -1;
    levelIndex = 1;
    while ((levelIndex < patternLength) && (levelIndex > 0) &&
	   (indexArray[0] < ((DynamicPtrArray*) listOfItemLists.GetItem(0))->GetSize())) {
      DynamicPtrArray* backArray =
	(DynamicPtrArray*)listOfItemLists.GetItem(levelIndex-1);
      IndexedItem* backItem =
	(IndexedItem*) backArray->GetItem(indexArray[levelIndex-1]);
      if (!backItem) {
	cout << "backItem NULL levelIndex " << levelIndex << "\n";
	cout << indexArray[levelIndex-1] << "\n";
      }
      DynamicPtrArray* curArray =
	(DynamicPtrArray*) listOfItemLists.GetItem(levelIndex);
      IndexedItem* curItem =
	(IndexedItem*) curArray->GetItem(indexArray[levelIndex]);
      if (!curItem) {
	cout << "curItem NULL levelIndex " << levelIndex << "\n";
	cout << indexArray[levelIndex] << "\n";
      }
      while ((levelIndex > 0) && (backItem->end1+1 != curItem->start1)) {
	indexArray[levelIndex]++;
	if (indexArray[levelIndex] >= curArray->GetSize()) {
	  //	  cout << "indexArray[" << levelIndex << "] went too high\n";
	  indexArray[levelIndex] = 0;
	  levelIndex--;
	  indexArray[levelIndex]++;
	  while ((levelIndex > 0) && 
		 (indexArray[levelIndex] >= ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetSize())) {
	    indexArray[levelIndex] = 0;
	    levelIndex--;
	    indexArray[levelIndex]++;
	  }
	  //	  levelIndex--;
	  levelIndex = 0;
	  break;
	} else {
	  curItem = (IndexedItem*) curArray->GetItem(indexArray[levelIndex]);
	}
      }
      levelIndex++;
      if (levelIndex ==0) levelIndex++;
    }
  }
  // now delete all things just used here
  for (levelIndex = 0; levelIndex < patternLength; levelIndex++) {
    int ndxLim = ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetSize();
    for (int i = 0; i < ndxLim; i++) {
      IndexedItem* tempItem = (IndexedItem*) ((DynamicPtrArray*) listOfItemLists.GetItem(levelIndex))->GetItem(i);
      delete tempItem->item;
      delete tempItem;
    }
    delete (DynamicPtrArray*) listOfItemLists.GetItem(levelIndex);
  }
  return patternArray;    
}
	

DynamicPtrArray* Learner::UnevenMatch(Pattern* pattern1, int start1, int end1,
				      Pattern* pattern2, int start2, int end2)
{
  // we require that pattern1 is the longer portion
  DynamicPtrArray* curPatternList = new DynamicPtrArray;
  DynamicPtrArray* newPatternList = NULL;
  Pattern* curPattern = new Pattern();
  curPatternList->Add(curPattern);
  // First we need to find the set of anchor points
  AnchorPair* anchorList = NULL;
  anchorList = FindAnchors(pattern1, start1, end1, pattern2, start2, end2);
  AnchorPair* curAnchor = anchorList;
  int i = start1;
  int j = start2;
  int keepGoing = 1;
  int iLim;
  int jLim;
  while (keepGoing) {
    if (curAnchor) {
      iLim = curAnchor->index1;
      jLim = curAnchor->index2;
    } else {
      iLim = end1+1;
      jLim = end2+1;
    }
    // we need to get the pattern up to curAnchor
    if ((i == iLim) && (j == jLim)) {
      // there's nothing before the anchor
    } else if (i == iLim) {
      // nothing before anchor in i, but there is in j
      // we need to make PatternLists out of what's in pattern2 from j to
      // jLim-1
      DynamicPtrArray* itemList = NULL;
      itemList = MakePatternLists(pattern2, j, jLim-1);
      // Now add those itemList(s) to all of the patterns in curPatternList
      int numItems = itemList->GetSize();
      int numPatterns = curPatternList->GetSize();
      if (numItems == 1) {
	// we get to just spin through the list of current patterns and
	// tack the item on to each of them
	PatternList* anItem = (PatternList*) itemList->GetItem(0);
	for (int n = 0; n < numPatterns; n++) {
	  Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	  PatternList* newItem = new PatternList(anItem);
	  thePattern->AddItem(newItem);
	}
	delete anItem;
      } else {
	newPatternList = new DynamicPtrArray(numPatterns*numItems);
	for (int n = 0; n < numPatterns; n++) {
	  Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	  PatternList* anItem;
	  PatternList* newItem;
	  for (int m = 0; m < numItems-1; m++) {
	    Pattern* newPattern = new Pattern(thePattern);
	    anItem  = (PatternList*) itemList->GetItem(m);
	    newItem = new PatternList(anItem);
	    newPattern->AddItem(newItem);
	    newPatternList->Add(newPattern);
	  }
	  anItem = (PatternList*) itemList->GetItem(numItems-1);
	  newItem = new PatternList(anItem);
	  thePattern->AddItem(anItem);
	  newPatternList->Add(thePattern);
	}
	delete curPatternList;
	curPatternList = newPatternList;
	for (int k = 0; k < numItems; k++) {
	  delete (PatternList*) itemList->GetItem(k);
	}
      }
      delete itemList;
    } else if (j == jLim) {
      // nothing before anchor in j, but there is in i
      // we need to make PatternLists out of what's in pattern1 from i to
      // iLim-1
      DynamicPtrArray* itemList = NULL;
      itemList = MakePatternLists(pattern1, i, iLim-1);
      // Now add those itemList(s) to all of the patterns in curPatternList
      int numItems = itemList->GetSize();
      int numPatterns = curPatternList->GetSize();
      if (numItems == 1) {
	// we get to just spin through the list of current patterns and
	// tack the item on to each of them
	PatternList* anItem = (PatternList*) itemList->GetItem(0);
	for (int n = 0; n < numPatterns; n++) {
	  Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	  PatternList* newItem = new PatternList(anItem);
	  thePattern->AddItem(newItem);
	}
	delete anItem;
      } else {
	newPatternList = new DynamicPtrArray(numPatterns*numItems);
	for (int n = 0; n < numPatterns; n++) {
	  Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	  PatternList* anItem;
	  PatternList* newItem;
	  for (int m = 0; m < numItems-1; m++) {
	    Pattern* newPattern = new Pattern(thePattern);
	    anItem  = (PatternList*) itemList->GetItem(m);
	    newItem = new PatternList(anItem);
	    newPattern->AddItem(newItem);
	    newPatternList->Add(newPattern);
	  }
	  anItem = (PatternList*) itemList->GetItem(numItems-1);
	  newItem = new PatternList(anItem);
	  thePattern->AddItem(newItem);
	  newPatternList->Add(thePattern);
	}
	delete curPatternList;
	curPatternList = newPatternList;
	for (int k = 0; k < numItems; k++) {
	  delete (PatternList*) itemList->GetItem(k);
	}
      }
      delete itemList;
    } else {
      // we need to do the matches of i to iLim-1 in pattern1
      // to j to jLim-1 in pattern2
      DynamicPtrArray* itemsList;
      if (iLim - i == jLim - j) {
	// use EvenMatch
	itemsList = this->EvenMatch(pattern1,i,iLim-1,
				    pattern2,j,jLim-1);
      } else { 
	// we have to do an uneven match between anchors
	itemsList = TrueUnevenMatch(pattern1,i,iLim-1,
				    pattern2,j,jLim-1);
      }
      // now we need to tack the new patterns on the end of the old ones
      int numItems = itemsList->GetSize();
      int numPatterns = curPatternList->GetSize();
      if (numItems == 1) {
	// we get to just sping through the list of current patterns and
	// tack the new pattern on to each of them
	Pattern* newList = (Pattern*) itemsList->GetItem(0);
	for (int n = 0; n < numPatterns; n++) {
	  Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	  thePattern->AddToSuffix(newList);
	}
	delete newList;
      } else {
	newPatternList = new DynamicPtrArray(numPatterns*numItems);
	for (int n = 0; n < numPatterns; n++) {
	  Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	  Pattern* newList;
	  for (int m = 0; m < numItems-1; m++) {
	    Pattern* newPattern = new Pattern(thePattern);
	    newList = (Pattern*) itemsList->GetItem(m);
	    newPattern->AddToSuffix(newList);
	    newPatternList->Add(newPattern);
	  }
	  newList = (Pattern*) itemsList->GetItem(numItems-1);
	  if (newList) {
	    thePattern->AddToSuffix(newList);
	  }
	  newPatternList->Add(thePattern);
	}
	delete curPatternList;
	curPatternList = newPatternList;
	for (int k = 0; k < numItems; k++) {
	  delete (Pattern*) itemsList->GetItem(k);
	}
      }
      delete itemsList;
    } // up to iLim/jLim
    // Now we need to handle the anchor if there is one; otherwise we end
    if (curAnchor) {
      // we need to create a single item from the anchor point and tack
      // it on the end of all the patterns in curPatternList
      int numPatterns = curPatternList->GetSize();
      for (int n = 0; n < numPatterns; n++) {
	Pattern* thePattern = (Pattern*) curPatternList->GetItem(n);
	PatternItem* item = new PatternItem(pattern1->GetPatternItem(curAnchor->index1));
	thePattern->AddItem(item);
      }
      // then we update i and j to be 1 past the anchor indexes
      i = curAnchor->index1 + 1;
      j = curAnchor->index2 + 1;
      curAnchor = curAnchor->next;
    } else {
      keepGoing = 0;
    }
  }// while keep going
  // delete the anchorList
  while (anchorList) {
    curAnchor = anchorList->next;
    delete anchorList;
    anchorList = curAnchor;
  }
  // return the generalized patterns
  return curPatternList;    
}



