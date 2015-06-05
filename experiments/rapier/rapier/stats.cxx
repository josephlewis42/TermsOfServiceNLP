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
#include "ComputeResults.h"
#include "Learner.h"
/**
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;


int main(int argc, char** argv) {

  char* exampleFileName;
  
  if (argc < 2) {
    cerr << "Usage: stats exampleFileName";
  } else {
    exampleFileName = argv[1];
    WordStore ws;
    InitSemantics();
    InitSkipLists();
    InitializeRuleCounters();
    
    
    filebuf fin;
    ComputeResults(exampleFileName,".filled",ws);
    
    cout.flush();
  }
  
  return 0;

}
**/

