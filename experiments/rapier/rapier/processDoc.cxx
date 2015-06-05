//-*-C++-*-

#include <string.h>

#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"
#include "SemClass.h"
#include "Learner.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;


main(int argc, char** argv) {

  char* exampleName;
  char* templateName;
  char* ruleBaseName;

  if (argc < 4) {
    cerr << "Usage: processDoc templateFileName ruleBaseFileName exampleFileName\n";
  } else {
    templateName = argv[1];
    ruleBaseName = argv[2];
    exampleName = argv[3];
    WordStore ws;
    RuleBase aRuleBase;
    Template emptyTemplate(&aRuleBase);

    InitSemantics();
    InitSkipLists();
    InitializeRuleCounters();

    filebuf fin;

    // read the template
    fin.open(templateName,std::ios::in);
    istream tempin2(&fin);
    emptyTemplate.ReadUnFilled(tempin2,ws);
    fin.close();

    fin.open("rb1",std::ios::in);
    istream rulein(&fin);
    aRuleBase.Read(rulein,ws);
    fin.close();

    Document *doc = NULL;
    char     docName[80];
    char     origName[80];
    char     tempName[80];
    
    strcpy(docName,exampleName);
    strcpy(origName,exampleName);
    strcat(docName,"_out");
    strcat(origName,".orig");
    doc = new Document;
    filebuf origin;
    filebuf docin;
    filebuf* fb = docin.open(docName,std::ios::in);
    if (!fb) {
      cout << "could not open file " << docName << "\n";
    } else {
      fb = origin.open(origName,std::ios::in);
      if (!fb) {
	cout << "could not open file " << origName << "\n";
      } else {
	istream docfile(&docin);
	istream origfile(&origin);
	doc->Read(docfile,origfile,ws);
	docin.close();
	origin.close();
	
	// make a copy of the empty template
	Template* filledTemplate1 = emptyTemplate.CopyStructure();
	// apply the copied template to the document
	filledTemplate1->ApplyTo(doc);
	//    cout << "template1 applied\n";
	
	strcpy(tempName,exampleName);
	strcat(tempName,".filled");
	filebuf fileout;
	fileout.open(tempName,std::ios::out);
	ostream stream2(&fileout);
	filledTemplate1->Write(stream2);
	cout << filledTemplate1->GetTemplateCertainty();
	fileout.close();
	
      }
    }
    cout.flush();
  }

}
