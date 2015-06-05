//-*-C++-*-

/**
#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"
#include "SemClass.h"
#include "Perform.h"
#include "ComputeResults.h"
#include "Learner.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;


main(int argc, char** argv) {

  WordStore ws;
  RuleBase aRuleBase;
  Template emptyTemplate(&aRuleBase);
  InitSemantics();
  InitSkipLists();
  InitializeRuleCounters();

  char* templateFileName;
  char* exampleFileName;
  char* ruleBase1FileName;

  if (argc < 4) {
    cerr << "Usage: test2 templateFileName ruleBaseFileName exampleFileName\n";
  } else {
    templateFileName = argv[1];
    ruleBase1FileName = argv[2];
    exampleFileName = argv[3];

    filebuf fin;

  // read the template
    cout << "reading template\n";
    fin.open(templateFileName,std::ios::in);
    istream tempin2(&fin);
    emptyTemplate.ReadUnFilled(tempin2,ws);
    fin.close();

    cout << "reading rulebase\n";
    fin.open(ruleBase1FileName,std::ios::in);
    istream rulein(&fin);
    aRuleBase.Read(rulein,ws);
    fin.close();

    cout << "processing documents\n";
    ProcessDocuments(exampleFileName,ws,&emptyTemplate);
    ComputeResults(exampleFileName,".filled",ws);

    cout.flush();
  }

}
**/

