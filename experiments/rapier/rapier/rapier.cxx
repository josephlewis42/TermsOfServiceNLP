//-*-C++-*-



#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"
#include "SemClass.h"
#include "Perform.h"
#include "Learner.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

int main(int argc, char** argv) {

  char* templateFileName;
  char* exampleFileName;
  char* ruleBase1FileName;

  if (argc < 4) {
    cerr << "Usage: rapier templateFileName exampleFileName ruleBaseFileName\n";
  } else {
    templateFileName = argv[1];
    exampleFileName = argv[2];
    ruleBase1FileName = argv[3];

    WordStore ws;

    Template emptyTemplate(NULL);
    InitSemantics();
    InitSkipLists();
    InitializeRuleCounters();


    filebuf fin;

    // read the template
    fin.open(templateFileName,std::ios::in);
    istream tempin2(&fin);
    emptyTemplate.ReadUnFilled(tempin2,ws);
    fin.close();

    Learner aLearner(&emptyTemplate);
    aLearner.ReadExamples(exampleFileName,ws);
    WriteRuleCounts();
    aLearner.DoGeneralization();
    WriteRuleCounts();
    aLearner.WriteRuleBase1(ruleBase1FileName);
    aLearner.WriteProtoTemplate("cs_job_template");
    cout.flush();
  }
  
  return 0;
}
