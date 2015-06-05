//-*-C++-*-
/* 5/29/96 Mary Elaine Califf 
 * handling semantic classes
 */

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;


void InitSemantics();
// initialize the semantic storage

char* sem_class(char* word, int& semNum, int prevSemNum = 0);
// returns a word representing the semantic class of a given word

int sem_class(char* word,char* sem);
// checks to see if sem is a semantic class of word

char* same_sem_class(char* word1, char* word2);

void AddSemPair(char* word, char* sem);

void PrintSemList(ostream& stream);

void ReadSemRules(istream& stream, WordStore& ws);
