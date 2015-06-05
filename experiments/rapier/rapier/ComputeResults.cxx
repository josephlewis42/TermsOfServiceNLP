//-*-C++-*-
// Perform.cxx - reads a list of file roots from test_file
// then reads root.out, applies a copy of cs_job_template, and
// prints out the filled template in root.filled

#include <iostream>
#include <string.h>

#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"
#include "SemClass.h"


#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;


typedef struct SlotResults {
  char* slotName;
  int numerator;
  int recalDenom;
  int precDenom;
} SlotResults;


void ComputeResults(char* exFile, char* extension, WordStore& ws)
{
  
  Template* origTemplate = NULL;
  Template* computedTemplate = NULL;
  char     baseName[80];
  char     origName[80];
  char     computedName[80];
  SlotResults  resultArray[30];
  int       numSlots;
  int      totNumerator;
  int      totRecallDenom;
  int      totPrecDenom;
  int      curNumerator;
  int      curRecallDenom;
  int      curPrecDenom;
  Template baseTemplate(NULL);

  

  filebuf fin;
  fin.open("cs_job_template",std::ios::in);
  istream tempin2(&fin);
  baseTemplate.ReadUnFilled(tempin2,ws);
  fin.close();

  fin.open(exFile,std::ios::in);
  istream infile(&fin);
  totNumerator = 0;
  totRecallDenom = 0;
  totPrecDenom = 0;
  for (int m = 0; m < 30 ; m++) {
    resultArray[m].slotName = NULL;
    resultArray[m].numerator = 0;
    resultArray[m].recalDenom = 0;
    resultArray[m].precDenom = 0;
  }
  int templateLen = 0;
  while (infile.good()) {
    // read the base name
    infile.getline(baseName,80);
    strcpy(origName,baseName);
    strcat(origName,".template");
    strcpy(computedName,baseName);
    strcat(computedName,extension);
    cout << "processing " << baseName << "\n";
    origTemplate = new Template(NULL);
    filebuf docin;
    filebuf* fb = docin.open(origName,std::ios::in);
    if (!fb) {
      cout << "could not open file " << origName << "\n";
    } else {
    istream docfile(&docin);
    //    cout << "reading handfilled template\n";
    origTemplate->ReadFilled(docfile,ws);
    //    cout << "handfilled template read\n";
    docin.close();
    Template* filledTemplate = new Template(NULL);
    filebuf filledin;
    fb = filledin.open(computedName,std::ios::in);
    if (!fb) {
      cout << "could not open file " << computedName << "\n";
    } else {
    istream filledfile(&filledin);
    //cout << "reading computed template\n";
    filledTemplate->ReadFilled(filledfile,ws);
    //    cout << "computed template read\n";
    filledin.close();
    templateLen = origTemplate->GetNumSlots();
    curPrecDenom = 0;
    curRecallDenom = 0;
    curNumerator = 0;
    for (int k = 0; k < templateLen; k++) {
      resultArray[k].slotName = origTemplate->GetSlotName(k);
      int precTemp = (filledTemplate->GetSlot(k))->CountFillers();
      int recallTemp = (origTemplate->GetSlot(k))->CountFillers();
      int matchTemp = (filledTemplate->GetSlot(k))->CountMatchingFillers(origTemplate->GetSlotFillers(k));
      if ('s' == (baseTemplate.GetSlot(k))->GetSlotType()) {
	if (recallTemp) recallTemp = 1;
	if (matchTemp) {
	  precTemp = precTemp - matchTemp + 1;
	  matchTemp = 1;
	}
      }	
      resultArray[k].precDenom = resultArray[k].precDenom + precTemp;
      curPrecDenom = curPrecDenom + precTemp;
      resultArray[k].recalDenom = resultArray[k].recalDenom + recallTemp;
      curRecallDenom = curRecallDenom + recallTemp;
      resultArray[k].numerator = resultArray[k].numerator + matchTemp;
      curNumerator = curNumerator + matchTemp;
    }

    // now we need to do the counting
    totNumerator += curNumerator;
    totRecallDenom += curRecallDenom;
    totPrecDenom += curPrecDenom;
    cout << "# Matching Fillers: " << curNumerator << "\n";
    cout << "# Fillers Computed: " << curPrecDenom << "\n";
    cout << "# Fillers in Original: " << curRecallDenom << "\n";

    delete filledTemplate;
    filledTemplate = NULL;
    }
    delete origTemplate;
    origTemplate = NULL;
    }
  }
  for (int l = 0; l < templateLen; l++) {
    cout << "Slot: " << resultArray[l].slotName << "\n";
    cout << " Matching Fillers: " << resultArray[l].numerator << "\n";
    cout << " Fillers Computed: " << resultArray[l].precDenom << "\n";
    cout << " Fillers in Orig:  " << resultArray[l].recalDenom << "\n";
    cout << " Precision: " << (float) resultArray[l].numerator / 
      (float) resultArray[l].precDenom << "\n";
    cout << " Recall: " << (float) resultArray[l].numerator /
      (float) resultArray[l].recalDenom << "\n";
  }
  cout << "Total # Matching Fillers: " << totNumerator << "\n";
  cout << "Total # Fillers Computed: " << totPrecDenom << "\n";
  cout << "Total # Fillers in Original: " << totRecallDenom << "\n";
  double totPrec = (float) totNumerator / (float) totPrecDenom;
  double totRec = (float) totNumerator / (float) totRecallDenom;
  double totFmeas = (2 * totPrec * totRec)/(totPrec + totRec);
  cout << "Precision: " << (float) totNumerator / (float) totPrecDenom << "\n";
  cout << "Recall: " << (float) totNumerator / (float) totRecallDenom << "\n";
  cout << "F-measure: " << totFmeas << "\n";

}















