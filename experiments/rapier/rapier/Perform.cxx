//-*-C++-*-
// Perform.cxx - reads a list of file roots from test_file
// then reads root.out, applies a copy of cs_job_template, and
// prints out the filled template in root.filled

#include <string.h>

#include "WordStore.h"
#include "Document.h"
#include "utils.h"
#include "Rule.h"
#include "SkipList.h"
#include "RuleBase.h"
#include "Template.h"
#include "SemClass.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;



void ProcessDocuments(char* exampleFileName,WordStore& ws, Template* emptyTemplate)
{
  
  Document *doc = NULL;
  char     baseName[80];
  char     docName[80];
  char     origName[80];
  char     tempName[80];
  
  filebuf fin;
  fin.open(exampleFileName,std::ios::in);
  istream infile(&fin);
  
  while (infile.good()) {
    // read the doc name
    infile.getline(baseName,80);
    strcpy(docName,baseName);
    strcpy(origName,baseName);
    strcat(docName,"_out");
    strcat(origName,".orig");
    cout << "processing " << docName << "\n";
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
	cout << "reading document\n";
	doc->Read(docfile,origfile,ws);
	cout << "document read\n";
	docin.close();
	origin.close();
	
	// make a copy of the empty template
	Template* filledTemplate1 = emptyTemplate->CopyStructure();
	//    cout << "template1 copied\n";
	
	// apply the copied template to the document
	filledTemplate1->ApplyTo(doc);
	//    cout << "template1 applied\n";
	
	// now we need to write the filled templates to files
	strcpy(tempName,baseName);
	strcat(tempName,".filled");
	filebuf tempout;
	tempout.open(tempName,std::ios::out);
	ostream tempfile1(&tempout);
	filledTemplate1->Write(tempfile1);
	//    cout << "template1 written\n";
	tempout.close();
	
	
	delete filledTemplate1;
	filledTemplate1 = NULL;
	delete doc;
	doc = NULL;
      }
    }
  }
}















