//-*-C++-*-

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include "WordStore.h"
#include "utils.h"
#include "Document.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;



Slot::Slot(char* name, char type, int numFilled, int numStarred)
{
  m_SlotName = name;
  m_SlotType = type;
  m_nNumFillers = 0;
  m_FillerList = (Filler*) NULL;
  m_bMismatch = 0;
  m_bLearnSlot = 0;
  m_nNumFilled = numFilled;
  m_nNumStarred = numStarred;
  m_nMinCertainty = 0;
}

Slot::Slot(char* name, char type, int learner)
{
  m_SlotName = name;
  m_SlotType = type;
  m_nNumFillers = 0;
  m_FillerList = (Filler*) NULL;
  m_bMismatch = 0;
  m_bLearnSlot = learner;
  m_nNumFilled = 0;
  m_nNumStarred = 0;
  m_nMinCertainty = 0;
}

Slot::~Slot()
{
  Filler* fi, *fj;
  WordList* wi, *wj;

  fi = m_FillerList;
  while (fi) {
    wi = fi->filler;
    while (wi) {
      wj = wi->next;
      delete wi;
      wi = wj;
    }
    fj = fi->next;
    delete fi;
    fi = fj;
  }
}


void Slot::SetNewCertainty(int newCertainty) 
{
  if (m_nMinCertainty) {
    if (newCertainty < m_nMinCertainty) {
      m_nMinCertainty = newCertainty;
    }
  } else {
    m_nMinCertainty = newCertainty;
  }
}


int Slot::CountFillers() 
{
  Filler* f = m_FillerList;
  int result = 0;
  while (f) {
    result++;
    f = f->next;
  }
  return result;
}

int Slot::CountMatchingFillers(Filler* matchingFillerList)
{
  
  Filler* f = m_FillerList;
  int result = 0;
  while (f && matchingFillerList) {
    if (FillerMember(f->filler,matchingFillerList)) {
      result++;
    }
    f = f->next;
  }
  return result;
}

int Slot::FillersMatch(Filler* matchingFillerList)
{
  if (!m_FillerList && !matchingFillerList) {
    long temp;
    time(&temp);
    unsigned int seed = temp;
    int compNum = rand_r(&seed) % m_nNumStarred;
    if (compNum < m_nNumFilled) {
      return 0;
    } else {
      return 1;
    }
  }
  return EqualFillerList(m_FillerList,matchingFillerList);
}

void Slot::AddFiller(WordList* newFiller) 
{
  Filler* f = m_FillerList;
  int isNew = 1;
  while (f && isNew) {
    if (EqualFillers(f->filler,newFiller))
      isNew = 0;
    f = f->next;
  }
  if (isNew) {
    f = new Filler;
    f->filler = newFiller;
    f->next = m_FillerList;
    m_FillerList = f;
    m_nNumFillers++;
  }
}

void Slot::AddFillerList(Filler* newFillers)
{
  Filler* f = newFillers;
  Filler* g = NULL;
  while (f) {
    this->AddFiller(f->filler);
    g = f;
    f = f->next;
    delete g;
  }
}

void Slot::ReadFillers(istream& stream, WordStore& ws)
{
  char     buffer[80];
  char     prevBuffer[80];
  char     origBuffer[80];
  int      prevI;
  int      origI;
  char*    word;
  WordList *curWord = NULL;
  WordList *listEnd = NULL;
  WordList *newFiller = NULL;
  char     c, nextc;
  
  stream.get(c);
  while (stream.good() && (c == ' ')) // get rid of the initial spaces
    stream.get(c);
  prevI = 0;
  origI = 0;
  while (stream.good() && (c != 10) && (c != 13)) {

    while (stream.good() && (c != 92) && (c != 10) && (c != 13)) {
      while ((c == ' ') || (c == 9)) {
	prevBuffer[prevI] = c;
	prevI++;
	stream.get(c);
      }
      if (symbol(c)) {
	prevBuffer[prevI] = 0;
	prevI++;
	buffer[0] = c;
	buffer[1] = 0;
	word = ws.StoreWord(buffer);
	curWord = new WordList;
	curWord->word = word;
	curWord->prevText = (char*) malloc(prevI*sizeof(char));
	strcpy(curWord->prevText,prevBuffer);
	curWord->origText = (char*) malloc(2*sizeof(char));
	curWord->origText[0] = c;
	curWord->origText[1] = 0;
	curWord->next = NULL;
	prevI = 0;
	origI = 0;
	if (!listEnd) { // need to set up to Filler
	  listEnd = curWord;
	  newFiller = curWord;
	} else {
	  listEnd->next = curWord;
	  listEnd = curWord;
	}
	stream.get(c);
	while ((c == ' ') || (c == 9)) {
	  prevBuffer[prevI] = c;
	  prevI++;
	  stream.get(c);
	}
      } else { //the word wasn't a single symbol - go into word reading loop
	int i = 0; // counter
	int cont = 1; //flag to continue word
	origI = 0;
	while (stream.good() && cont) { //word loop
	  switch (c) {
	  case 32 :
	  case 10:
	  case 13:
	  case 9 :
	    // end of word
	    cont = 0;
	    break;
	  case '.' :
	    // have to check for mid-word period and for abbreviation
	    stream.get(nextc);
	    if ((nextc == 32) || (nextc == 10) || (nextc == 13) || 
		(nextc == 9)) { //end-word
	      buffer[i] = 0;
	      origBuffer[origI] = 0;
	      origI++;
	      if (abbreviation(buffer)) {
		buffer[i] = c;
		origBuffer[origI] = c;
		origI++;
		i++;
		c = nextc;
	      } else {
		stream.putback(nextc);
		cont = 0;
	      }
	    } else {
	      buffer[i] = c;
	      origBuffer[origI] = c;
	      origI++;
	      i++;
	      c = nextc;
	    }
	    break;
	  case 39 :
	    // what we want to do here is break the word, but stay in the 
	    // word-reading loop
	    buffer[i] = 0;
	    origBuffer[origI] = 0;
	    origI++;
	    prevBuffer[prevI] = 0;
	    prevI++;
	    word = ws.StoreWord(buffer);
	    curWord = new WordList;
	    curWord->word = word;
	    curWord->prevText = (char*) malloc(prevI*sizeof(char));
	    strcpy(curWord->prevText,prevBuffer);
	    curWord->origText = (char*) malloc(origI*sizeof(char));
	    strcpy(curWord->origText,origBuffer);
	    origI = 0;
	    prevI = 0;
	    i = 0;
	    curWord->next = NULL;
	    if (!listEnd) { // need to set up to Filler
	      listEnd = curWord;
	      newFiller = curWord;
	    } else {
	      listEnd->next = curWord;
	      listEnd = curWord;
	    }	
	    buffer[i] = c;
	    origBuffer[origI] = c;
	    origI++;
	    i++;
	    stream.get(c);
	    break;
	  case '!' :
	  case '@' :
	    // need to check whether these are mid-word
	    stream.get(nextc);
	    if ((nextc == 32) || (nextc == 10) || (nextc == 13) || 
		(nextc == 9)) { //end-word
	      stream.putback(nextc);
	      cont = 0;
	    } else {
	      buffer[i] = c;
	      origBuffer[origI] = c;
	      origI++;
	      i++;
	      c = nextc;
	    }
	    break;
	  default: 
	    if (symbol(c)) { // this is a symbol not previously handled
	      cont = 0;
	    } else {
	      buffer[i] = lowercase(c);
	      origBuffer[origI] = c;
	      origI++;
	      i++;
	      stream.get(c);
	    }
	  } //switch
	} // word loop
	buffer[i] = 0;
	origBuffer[origI] = 0;
	origI++;
	prevBuffer[prevI] = 0;
	prevI++;
        word = ws.StoreWord(buffer);
        curWord = new WordList;
        curWord->word = word;
	curWord->prevText = (char*) malloc(prevI*sizeof(char));
	strcpy(curWord->prevText,prevBuffer);
	curWord->origText = (char*) malloc(origI*sizeof(char));
	strcpy(curWord->origText,origBuffer);
	origI = 0;
	prevI = 0;
	curWord->next = NULL;
        if (!listEnd) { // need to set up to Filler
          listEnd = curWord;
          newFiller = curWord;
        } else {
          listEnd->next = curWord;
          listEnd = curWord;
        }	
	while (stream.good() && ((c == ' ') || (c == 9))) {
	  prevBuffer[prevI] = c;
	  prevI++;
	  stream.get(c);
	}
      } // if
    } // while not end of this filler
    this->AddFiller(newFiller);
    listEnd = NULL;
    newFiller = NULL;
    if ((c == 39) || (c == 92)) stream.get(c);
  } // while not end of this line of fillers
  // all done
}

void Slot::WriteMarkedUnion(ostream& stream)
{
  if (m_bMismatch) {
    stream << "**";
  }
  this->Write(stream);
}

void Slot::Write(ostream& stream) 
{
  stream << m_SlotName << ": ";
  Filler* curFiller = m_FillerList;
  WordList* curWordList;
  
  while (curFiller) {
    curWordList = curFiller->filler;
    while (curWordList) {
      stream << curWordList->prevText << curWordList->origText;
      curWordList = curWordList->next;
    }
    if (curFiller->next)
      stream << " \\ ";
    curFiller = curFiller->next;
  }
  stream << '\n';
}

void Slot::WriteUnfilled(ostream& stream) 
{
  stream << m_SlotName << ": ";
  stream << m_SlotType << " " << m_nNumFilled;
  stream << " " << m_nNumStarred << "\n";
}

// Template routines

Template::Template(RuleBase* ruleBase)
{
  m_TemplateName = NULL;
  m_nNumSlots = 0;
  for (int i = 0; i < 30; i++) 
    m_SlotArray[i] = NULL;
  m_RuleBase = ruleBase;
}

Template::~Template()
{
  for (int i = 0; i < m_nNumSlots; i++)
    delete m_SlotArray[i];
}

int Template::CountFillers() 
{
  int result = 0;
  for (int i = 0; i < m_nNumSlots; i++) {
    result = result + m_SlotArray[i]->CountFillers();
  }
  return result;
}

int Template::GetTemplateCertainty()
{
  int result = 0;
  for (int i = 0; i < m_nNumSlots; i++) {
    result = result + m_SlotArray[i]->GetMinCertainty();
    if (0 == m_SlotArray[i]->GetMinCertainty()) {
      result = result + m_SlotArray[i]->GetNumStarred() -
	m_SlotArray[i]->GetNumFilled();
    }
  }
  return result;
}


int Template::CountMatchingFillers(Template* matchingTemplate) 
{
  int result = 0;
  for (int i = 0; i < m_nNumSlots; i++) {
    result = result +  m_SlotArray[i]->CountMatchingFillers(matchingTemplate->GetSlotFillers(i));
  }
  return result;
}

int Template::FillersMatch(Template* matchingTemplate)
{
  int result = 1;
  for (int i = 0; result && (i < m_nNumSlots); i++) {
    result = m_SlotArray[i]->FillersMatch(matchingTemplate->GetSlotFillers(i));
  }
  return result;
}

int Template::UnionTemplates(Template* temp1, Template* temp2)
{
  int mismatchCount = 0;
  for (int i = 0; i < m_nNumSlots; i++) {
    if (temp1->GetSlot(i)->FillersMatch(temp2->GetSlotFillers(i))) {
      m_SlotArray[i]->AddFillerList(CopyFillerList(temp1->GetSlotFillers(i)));
    } else {
      mismatchCount++;
      m_SlotArray[i]->SetMismatchFlag();
      m_SlotArray[i]->AddFillerList(UnionFillerLists(temp1->GetSlotFillers(i), 
						 temp2->GetSlotFillers(i)));
    }
  }
  return mismatchCount;
}

void Template::IntersectTemplates(Template* temp1, Template* temp2)
{
  for (int i = 0; i < m_nNumSlots; i++) {
    m_SlotArray[i]->AddFillerList(IntersectionFillerLists(temp1->GetSlotFillers(i), 
						      temp2->GetSlotFillers(i)));
  }
}

  
void Template::ReadUnFilled(istream& stream, WordStore& ws)
{
  char    buffer[80];
  char*   word;
  int     i;
  char    c;
  int     numFilled;
  int     numStarred;

  // first we need to read the template name
  i = 0;
  stream.get(c);
  while (!((c==10) || (c==13))) {
    buffer[i++] = c;
    stream.get(c);
  }
  buffer[i] = 0;
  word = ws.StoreWord(buffer);
  m_TemplateName = word;

  // Okay, now start collecting slot information
  m_nNumSlots = 0;
  stream.get(c);
  while (stream.good()) {
    // read the slot name
    i = 0;
    while (stream.good() && (c != ':')) {
      buffer[i++] = lowercase(c);
      stream.get(c);
    }
    if (stream.good()) {
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      stream.get(c);
      while ((c == ' ') || (c == 9)) 
	stream.get(c);
      // Now, we need to read the numbers
      stream >> numFilled;
      stream >> numStarred;
      m_SlotArray[m_nNumSlots++] = new Slot(word,c,numFilled,numStarred);
      while (stream.good() && (c != 10) && (c != 13))
	stream.get(c);
      stream.get(c);
    }
    
  }// reading slots

}

void Template::ReadFilled(istream& stream, WordStore& ws)
{
  char     c;
  int      i = 0;
  char     buffer[80];
  char*    word;
  
  // see whether we already know what the template is or not and 
  // work accordingly
  if (m_TemplateName) {  // presumably all we need is the fillers
    stream.get(c);
    while ((c != 10) && (c != 13)) 
      stream.get(c);
    stream.get(c);
    while (stream.good()) {
      if (c == '*') {
	m_SlotArray[i]->SetLearn();
      }
      while (stream.good() && (c != ':') )
	stream.get(c);
      if (stream.good()) {
	m_SlotArray[i]->ReadFillers(stream,ws);
	i++;
	stream.get(c);
      }
    }
  } else { // need to read template and fillers - type values unknown
    // first we need to read the template name
    i = 0;
    stream.get(c);
    while (!((c==10) || (c==13))) {
      buffer[i++] = c;
      stream.get(c);
    }
    buffer[i] = 0;
    word = ws.StoreWord(buffer);
    m_TemplateName = word;

    // Okay, now start collecting slot information
    m_nNumSlots = 0;
    stream.get(c);
    while (stream.good()) {
      // read the slot name
      int learningSlot = 0;
      if (c == '*') {
	learningSlot = 1;
	stream.get(c);
      } else {
	cout << c;
      }
      i = 0;
      while (stream.good() && (c != ':')) {
	buffer[i++] = lowercase(c);
	stream.get(c);
      }
      if (stream.good()) {
	buffer[i] = 0;
	word = ws.StoreWord(buffer);
	m_SlotArray[m_nNumSlots] = new Slot(word,' ',learningSlot);
	m_SlotArray[m_nNumSlots]->ReadFillers(stream,ws);
	m_nNumSlots++;
	stream.get(c);
      }
    } // slots
  }
}

void Template::Write(ostream& stream)
{
  stream << m_TemplateName << '\n';
  for (int i = 0; i < m_nNumSlots; i++) {
    m_SlotArray[i]->Write(stream);
  }
}


void Template::WriteUnfilled(ostream& stream)
{
  stream << m_TemplateName << '\n';
  for (int i = 0; i < m_nNumSlots; i++) {
    m_SlotArray[i]->WriteUnfilled(stream);
  }
}


void Template::WriteMarkedUnion(ostream& stream)
{
  stream << m_TemplateName << '\n';
  for (int i = 0; i < m_nNumSlots; i++) {
    m_SlotArray[i]->WriteMarkedUnion(stream);
  }
}

Template* Template::CopyStructure()
{
  Template* newTemplate = new Template(m_RuleBase);
  newTemplate->m_TemplateName = this->m_TemplateName;
  newTemplate->m_nNumSlots = this->m_nNumSlots;
  
  Slot* newSlot;
  for (int i = 0; i < m_nNumSlots; i++) {
    newSlot = new Slot(m_SlotArray[i]->GetSlotName(),m_SlotArray[i]->GetSlotType(),m_SlotArray[i]->GetNumFilled(), m_SlotArray[i]->GetNumStarred());
    newTemplate->m_SlotArray[i] = newSlot;
  }
  return newTemplate;
}


// utility routines
int symbol(char c) {
  return (((c < 48) && (c > 32)) || ((c < 65) && (c > 57)) ||
	  ((c < 97) && (c > 90)) || ((c < 127) && (c > 122)));
}

int abbreviation(char* str) {
  if (strchr(str,'.')) 
    return 1;
  if (1 == strlen(str)) 
    return 1;
  if (0 == strcmp(str, "co"))
    return 1;
  if (0 == strcmp(str, "mr"))
    return 1;
  else if (0 == strcmp(str, "ms"))
    return 1;
  else if (0 == strcmp(str, "mrs"))
    return 1;
  else if (0 == strcmp(str, "dr"))
    return 1;
  else if (0 == strcmp(str, "ph"))
    return 1;
  else if (0 == strcmp(str, "ph.d"))
    return 1;
  else if (0 == strcmp(str, "prof"))
    return 1;
  else if (0 == strcmp(str, "phd"))
    return 1;
  else if (0 == strcmp(str, "inc"))
    return 1;
  else if (0 == strcmp(str, "tx"))
    return 1;
  else if (0 == strcmp(str, "p.a"))
    return 1;
  else if (0 == strcmp(str, "a.p"))
    return 1;
  else if (0 == strcmp(str, "no"))
    return 1;
  else if (0 == strcmp(str, "sr"))
    return 1;
  else if (0 == strcmp(str, "sys"))
    return 1;
  else if (0 == strcmp(str, "chas"))
    return 1;
  else if (0 == strcmp(str, "st"))
    return 1;
  else if (0 == strcmp(str, "ave"))
    return 1;
  else if (0 == strcmp(str, "fla"))
    return 1;
  else if (0 == strcmp(str, "wis"))
    return 1;
  else return 0;
}

void Template::ApplyTo(Document* doc)
{
  char* slotName;
  DynamicPtrArray* ruleList;
  Filler* fillers;
  int findall;
  Rule* curRule;
  int result;

  for (int i = 0; i < m_nNumSlots; i++) {
    // for each slot, we're going to get the rules for that slot and then
    // match them against the document
    slotName = m_SlotArray[i]->GetSlotName();
    if (m_SlotArray[i]->GetSlotType() == 's') {
      findall = 0;
    } else {
      findall = 1;
    }
    ruleList = m_RuleBase->FindRules(slotName,m_TemplateName);
    if (ruleList) {
      int len = ruleList->GetSize();
      for (int j = 0; j < len; j++) {
	curRule = (Rule *) ruleList->GetItem(j);
	result = curRule->Match(doc,fillers,findall);
	if (result == GOOD) {
	  int pos = curRule->GetPosCovered();
	  int negs = curRule->GetNegCovered();
	  int cert = pos - (5 * negs);
	  m_SlotArray[i]->SetNewCertainty(cert);
	  m_SlotArray[i]->AddFillerList(fillers);
	  if (!findall) j = len;
	  fillers = NULL;
	}
      }
      delete ruleList;
    }
  }
}

void Template::MostSpecificRules(Document* doc, RuleBase* rb1, RuleBase* rb2)
{
  // go through the slots, and for each one with fillers, create all
  // possible most specific rules for that filler from the document
  // add each rule to both rule bases
  for (int i = 0; i < m_nNumSlots; i++) {
    cout << "building rule for slot " << m_SlotArray[i]->GetSlotName() << "\n";
    Filler* fillerList = m_SlotArray[i]->GetFillerList();
    while (fillerList) {
      cout << "locating filler\n";
      int startWord = this->LocateFillerInDoc(doc,fillerList,-1);
      if (startWord < 0) {
	cout << "!!!ERROR!!!\n";
	WordList* a = fillerList->filler;
	while (a) {
	  cout << a->word << " ";
	  a = a->next;
	} 
	cout << " not found in document\n";
      }
      while (startWord >= 0) {
	cout << "creating rule\n";
	Rule* newRule = this->CreateNewRule(doc,fillerList,startWord,
					    m_SlotArray[i]->GetSlotName());
	rb1->AddRule(newRule);
	//	Rule* newRule2 = new Rule(newRule);
	//rb2->AddRule(newRule2);
	cout << "trying to find filler again\n";
	startWord = this->LocateFillerInDoc(doc,fillerList,startWord);
      }
      fillerList = fillerList->next;
    }
  }
}

int Template::MSpRulesForSlot(int slotNum, Document* doc, RuleBase* rb,
			      Template* protoTemplate)
{
  if (!m_SlotArray[slotNum]->UseSlotToLearn()) {
    return 0;
  }
  protoTemplate->GetSlot(slotNum)->IncNumStarred();
  Filler* fillerList = m_SlotArray[slotNum]->GetFillerList();
  if (fillerList) 
    protoTemplate->GetSlot(slotNum)->IncNumFilled();
  while (fillerList) {
    cout << "locating filler\n";
    int startWord = this->LocateFillerInDoc(doc,fillerList,-1);
    if (startWord < 0) {
      cout << "!!!ERROR!!!\n";
      WordList* a = fillerList->filler;
      while (a) {
	cout << a->word << " ";
	a = a->next;
      } 
      cout << " not found in document\n";
    }
    while (startWord >= 0) {
      cout << "creating rule\n";
      Rule* newRule = this->CreateNewRule(doc,fillerList,startWord,
					  m_SlotArray[slotNum]->GetSlotName());
      rb->AddRule(newRule);
      cout << "trying to find filler again\n";
      startWord = this->LocateFillerInDoc(doc,fillerList,startWord+1);
    }
    fillerList = fillerList->next;
  }
  return 1;
}

int Template::LocateFillerInDoc(Document* doc, Filler* filler, 
				     int startWord)
{
  DocWord* i = NULL;
  DocWord* j = NULL;
  WordList* k = NULL;
  
  if (startWord > 0) {
    doc->ResetIterator(startWord);
    doc->AdvanceIterator();
  } else {
    doc->InitializeIterator();
  }
  int found = 0;
  int curIter = -1;
  curIter = doc->GetCurIterPtr();
  i = doc->GetNextWord();
  while (!found && (curIter < doc->Length())) {
    j = i;
    k = filler->filler;
    while (k && j && (j->theWord == k->word)) {
      j = doc->GetNextWord();
      k = k->next;
    }
    if (!k) {
      // we found a filler match
      found = 1;
      return curIter;
      
    } else {
      if (!j) // we hit the end of the document
	return -1;
      else {
	doc->ResetIterator(curIter);
	doc->AdvanceIterator();
	
	curIter = doc->GetCurIterPtr();
	i = doc->GetNextWord();
      }
    }
  }
  return -1;
    
}

Rule* Template::CreateNewRule(Document* doc, Filler* filler, 
			      int startWord,char* slotName)
{
  Rule* theRule = new Rule(m_TemplateName,slotName);

  // now get the info into it
  theRule->CreateFromDoc(doc,filler,startWord);
  return theRule;
}


