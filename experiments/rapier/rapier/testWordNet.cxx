//-*-C++-*-
// test out needed wordnet functions
/**
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;


#include "wn.h"
#include "wnconsts.h"

typedef struct SemClass {
  int pos;
  long offset;
} SemClass;

SemClass* FirstSemMatch(char* word1, char* word2)
{
  SynsetPtr baseStruct1;
  SynsetPtr baseStruct2;
  SynsetPtr tempStruct1;
  SynsetPtr tempStruct2;
  unsigned int pos1;
  unsigned int pos2;
  unsigned long searchType1;
  unsigned long searchType2;
  char tempWord1[50];
  char tempWord2[50];
  char* useWord1;
  char* useWord2;
  char tempWord[50];

  int word1InPOS;
  int word2InPOS;
  long bestMatchOffset;
  long bestMatchPOS;
  int matchFound = 0;
  int matchValue = 999;
  int dist1;
  int dist2;
  int contSearch;
  
  pos1 = in_wn(word1,ALL_POS);
  pos2 = in_wn(word2,ALL_POS);
  // for each bit set in both pos1 and pos2
  for (int i = NOUN; i <= SATELLITE; i++) {
    if (matchValue) {
      // we also need to deal with the issue of forms of the word here
      if (pos1 & bit(i)) {
	word1InPOS = 1;
      } else {
	useWord1 = morphstr(word1,i);
	if (useWord1) {
	  cout << "base form of word1 is " << useWord1 << "\n";
	  word1InPOS = 1;
	} else {
	  word1InPOS = 0;
	}
      }
      if (word1InPOS) { // check word 2 only if word 1 was valid
	if (pos2 & bit(i)) {
	  word2InPOS = 1;
	} else {
	  useWord2 = morphstr(word2,i);
	  if (useWord2) {
	    cout << "base form of word 2 is " << useWord2 << "\n";
	    word2InPOS = 1;
	  } else {
	    word2InPOS = 2;
	  }
	}
      }
      if (word1InPOS && word2InPOS) {
	// gather up the word1 stuff
	int search1defined = 0;
	int search2defined = 0;
	if (pos1 & bit(i)) {
	  searchType1 = is_defined(word1,i);
	  if ((searchType1 & bit(HYPERPTR))) {
	    search1defined = 1;
	  } 
	}
	if (!search1defined) {
	  // check to see if it's defined for any form of the word
	  useWord1 = morphstr(word1,i);
	  while (useWord1 && !search1defined) {
	    searchType1 = is_defined(useWord1,i);
	    if ((searchType1 & bit(HYPERPTR))) {
	      search1defined = 1;
	    } else {
	      useWord1 = morphstr(NULL,i);
	    }
	  }
	}
	// and the word2 stuff
	if (pos2 & bit(i)) {
          searchType2 = is_defined(word2,i);
          if ((searchType2 & bit(HYPERPTR))) {
            search2defined = 1;
          }
        }
        if (!search2defined) {
          // check to see if it's defined for any form of the word
          useWord2 = morphstr(word2,i);
          while (useWord2 && !search2defined) {
            searchType2 = is_defined(useWord2,i);
            if ((searchType2 & bit(HYPERPTR))) {
              search2defined = 1;
            } else {
              useWord2 = morphstr(NULL,i);
            }
          }
        }
	// now if the HYPERPTR search is defined for some form of each word
	// let's do the searches
	if (search1defined && search2defined) {
	  // collect whatever we can for word1
	  baseStruct1 = NULL;
	  tempStruct1 = NULL;
	  if (pos1 & bit(i)) {
	    searchType1  = is_defined(word1,i);
	    if (searchType1 & bit(HYPERPTR)) {
	      baseStruct1 = findtheinfo_ds(word1,i,-HYPERPTR,ALLSENSES);
	    }
	  }
	  // gather up all of the base forms for this POS
	  useWord1 = morphstr(word1,i);
	  while (useWord1) {
	    strcpy(tempWord1,useWord1);
	    searchType1 = is_defined(tempWord1,i);
	    if (searchType1 & bit(HYPERPTR)) {
	      tempStruct1 = findtheinfo_ds(tempWord1,i,-HYPERPTR,ALLSENSES);
	      if (!baseStruct1) {
		baseStruct1 = tempStruct1;
	      } else {
		// attach the lists together
		SynsetPtr a;
		SynsetPtr b;
		a = baseStruct1;
		b = baseStruct1->nextss;
		while (b) {
		  a = b;
		  b = a->nextss;
		}
		a->nextss = tempStruct1;
	      }
	    }
	    useWord1 = morphstr(NULL,i);
	  }
	  // collect whatever we can for word2
	  baseStruct2 = NULL;
	  tempStruct2 = NULL;
	  if (pos2 & bit(i)) {
	    searchType2 = is_defined(word2,i);
	    if (searchType2 & bit(HYPERPTR)) {
	      baseStruct2 = findtheinfo_ds(word2,i,-HYPERPTR,ALLSENSES);
	    }
	  }
	  // gather up all of the base forms for this POS
	  useWord2 = morphstr(word2,i);
	  while (useWord2) {
	    strcpy(tempWord2,useWord2);
	    searchType2 = is_defined(tempWord2,i);
	    if (searchType2 & bit(HYPERPTR)) {
	      tempStruct2 = findtheinfo_ds(tempWord2,i,-HYPERPTR,ALLSENSES);
	      if (!baseStruct2) {
		baseStruct2 = tempStruct2;
	      } else {
		// attach the lists together
		SynsetPtr a;
		SynsetPtr b;
		a = baseStruct2;
		b = baseStruct2->nextss;
		while( b) {
		  a = b;
		  b = a->nextss;
		}
		a->nextss = tempStruct2;
	      }
	    }
	    useWord2 = morphstr(NULL,i);
	  }
	  // now we have the two data structures: we want to find the lowest
	  // match in the hierarchy for the two of them
	  dist1 = 0;
	  dist2 = 0;
	  contSearch = 1;
	  // first let sees if the base structures have any matches	
	  SynsetPtr a1;
	  SynsetPtr a2;
	  a1 = baseStruct1;
	  while (a1 && contSearch) {
	    a2 = baseStruct2;
	    while (a2 && contSearch) {
	      if (a1->hereiam == a2->hereiam) {
		// we found a SynsetMatch
		bestMatchOffset = a1->hereiam;
		bestMatchPOS = i;
		matchFound = 1;
		matchValue = 0;
		contSearch = 0;
		// debugging
		cout << " found a 0th level match\n";
		cout << "hereiam: " << a1->hereiam << "\n";
		cout << "sstype: " << a1->sstype << "\n";
		cout << "fnum: " << a1->fnum << "\n";
		cout << "wcount: " << a1->wcount << "\n";
		for (int k = 0; k < a1->wcount; k++) {
		  cout << " " << a1->words[k] << " " << a1->wnsense[k] << "\n";
		}
	      }
	      a2 = a2->nextss;
	    }
	    a1 = a1->nextss;
	  }
	  // if we didn't find a SynsetMatch, we need to climb the hierarchy
	  // here
	  // first let's try just checking word1 level 1 versus word2 level 0
	  int hyperexists = 1;
	  int curval = 0;
	  while (contSearch && hyperexists) {
	    hyperexists = 0;
	    curval++;
	    dist1 = curval;
	    dist2 = 0;
	    SynsetPtr bottom1;
	    SynsetPtr bottom2;
	    int ctr;
	    while ((dist1 >= 0) && contSearch) {
	      bottom1 = baseStruct1;	    
	      while (bottom1 && contSearch) {
		a1 = bottom1;
		for (ctr = 0; (ctr < dist1) && a1; ctr++) {
		  if (a1->ptrlist) {
		    a1 = a1->ptrlist;
		  } else {
		    // retrieve the hyper from the file
		    SynsetPtr hypersynset = NULL;
		    for (int l = 0; (l < a1->ptrcount) && !hypersynset; l++) {
		      if (a1->ptrtyp[l] == HYPERPTR) {
			tempWord[0] = 0;
			hypersynset = read_synset(a1->ppos[l],a1->ptroff[l],tempWord);
		      }
		    }
		    if (hypersynset) {
		      a1->ptrlist = hypersynset;
		      a1 = hypersynset;
		    } else {
		      a1 = NULL;
		    }
		  }
		}
		if (a1) {
		  if (dist2 == 0) {
		    hyperexists = 1;
		  }
		  bottom2 = baseStruct2;
		  while (bottom2 && contSearch) {
		    a2 = bottom2;
		    for (ctr = 0; (ctr < dist2) && a2; ctr++) {
		      if (a2->ptrlist) {
			a2 = a2->ptrlist;
		      } else {
			// retrieve the hyper from the file
			SynsetPtr hypersynset = NULL;
			for (int l = 0; (l < a2->ptrcount) && !hypersynset; l++) {
			  if (a2->ptrtyp[l] == HYPERPTR) {
			    tempWord[0] = 0;
			    hypersynset = read_synset(a2->ppos[l],
						      a2->ptroff[l],
						      tempWord);
			  }
			}
			if (hypersynset) {
			  a2->ptrlist = hypersynset;
			  a2 = hypersynset;
			} else {
			  a2 = NULL;
			}
		      }
		    }
		    if (a2) {
		      if (dist1 == 0) {
			hyperexists = 1;
		      }
		      if (a1->hereiam == a2->hereiam) {
			// we found a SynsetMatch
			bestMatchOffset = a1->hereiam;
			bestMatchPOS = i;
			matchFound = 1;
			matchValue = dist1 + dist2;
			contSearch = 0;
			// debugging
			cout << "found a " << matchValue << " level match\n";
			cout << "hereiam: " << a1->hereiam << "\n";
			cout << "sstype: " << a1->sstype << "\n";
			cout << "fnum: " << a1->fnum << "\n";
			cout << "wcount: " << a1->wcount << "\n";
			for (int k = 0; k < a1->wcount; k++) {
			  cout << " " << a1->words[k] << " " << a1->wnsense[k] << "\n";
			}
		      } 
		    }
		    bottom2 = bottom2->nextss;
		  }
		}
		bottom1 = bottom1->nextss;
	      }
	      dist1--;
	      dist2++;
	    }
	  }

	  // free the base pointers
	  free_syns(baseStruct1);
	  free_syns(baseStruct2);
	}
      }
    }
  }  
  if (matchFound) {
    SemClass* result = new SemClass;
    result->pos = bestMatchPOS;
    result->offset = bestMatchOffset;
    return result;
  } else {
    return NULL;
  }
  
}

int WordInSemClass(char* word, SemClass* semClass) 
{
  int result = 0;
  int wordInPOS = 0;
  char* useWord;
  char tempWord[50];
  unsigned long searchType1;
  
  // let's try this bottom-up
  // first, make sure the word exists in the appropriate POS database
  int pos1 = in_wn(word,semClass->pos);
  if (pos1) {
    wordInPOS = 1;
  } else {
    useWord = morphstr(word,semClass->pos);
    if (useWord) {
      cout << "base form of word is " << useWord << "\n";
      wordInPOS = 1;
    } else {
      wordInPOS = 0;
    }
  }
  if (wordInPOS) {
    // now handle search stuff
    int searchDefined = 0;
    SynsetPtr baseStruct1 = NULL;
    SynsetPtr tempStruct1 = NULL;
    SynsetPtr classStruct = NULL;
    if (pos1) {
      searchType1 = is_defined(word,semClass->pos);
      if ((searchType1 & bit(HYPERPTR))) {
	baseStruct1 = findtheinfo_ds(word,semClass->pos,-HYPERPTR,ALLSENSES);
      }
    }
    // gather up all the base forms for this POS
    useWord = morphstr(word,semClass->pos);
    while (useWord) {
      strcpy(tempWord,useWord);
      searchType1 = is_defined(tempWord,semClass->pos);
      if (searchType1 & bit(HYPERPTR)) {
	tempStruct1 = findtheinfo_ds(tempWord,semClass->pos,-HYPERPTR,ALLSENSES);
	if (!baseStruct1) {
	  baseStruct1 = tempStruct1;
	} else {
	  // attach the lists together
	  SynsetPtr a;
	  SynsetPtr b;
	  a = baseStruct1;
	  b = baseStruct1->nextss;
	  while (b) {
	    a = b;
	    b = a->nextss;
	  }
	  a->nextss = tempStruct1;
	}
      }
      useWord = morphstr(NULL,semClass->pos);
    }
    // instantiate the sem class
    tempWord[0] = 0;
    classStruct = read_synset(semClass->pos, semClass->offset, tempWord);
    int contSearch = 1;
    // first see whether classStruct matches any of the base structures
    SynsetPtr a1;
    a1 = baseStruct1;
    while (a1 && contSearch) {
      if (a1->hereiam == classStruct->hereiam) {
	// we found a match
	result = 1;
	contSearch = 0;
      }
      a1 = a1->nextss;
    }
    // if we didn't find a match at the bottom level, we need to climb
    int hyperexists = 1;
    int level = 1;
    int ctr = 0;
    while (contSearch && hyperexists) {
      hyperexists = 0;
      SynsetPtr bottom1;
      bottom1 = baseStruct1;
      while (bottom1 && contSearch) {
	a1 = bottom1;
	for (ctr = 0; (ctr < level) && a1; ctr++) {
	  if (a1->ptrlist) {
	    a1 = a1->ptrlist;
	  } else {
	    // retrieve the hyper from the file
	    SynsetPtr hypersynset = NULL;
	    for (int l = 0; (l < a1->ptrcount) && !hypersynset; l++) {
	      if (a1->ptrtyp[l] == HYPERPTR) {
		tempWord[0] = 0;
		hypersynset = read_synset(a1->ppos[l],a1->ptroff[l],tempWord);
	      }
	    }
	    if (hypersynset) {
	      a1->ptrlist = hypersynset;
	      a1 = hypersynset;
	    } else {
	      a1 = NULL;
	    }
	  }
	}
	if (a1) {
	  hyperexists = 1;
	  if (a1->hereiam == classStruct->hereiam) {
	    // we found a match
	    result = 1;
	    contSearch = 0;
	  } 
	}
	bottom1 = bottom1->nextss;
      }
      level++;
    }
  }
  return result;
}

int EqualSemClasses(SemClass* class1, SemClass* class2)
{
  if ((class1->pos == class2->pos) && (class1->offset == class2->offset)) {
    return 1;
  } else {
    return 0;
  }
}

SemClass* GeneralizeWordAndClass(char* word1,SemClass* class1)
{
  if (WordInSemClass(word1,class1))
    return class1;
  // class1 doesn't subsume word1, so we need to look for a generalization
  SemClass* result = NULL;
  int wordInPOS = 0;
  char* useWord;
  char tempWord[50];
  unsigned long searchType1;
  // first, make sure the word exists in the appropriate POS database
  int pos1 = in_wn(word1,class1->pos);
  if (pos1) {
    wordInPOS = 1;
  } else {
    useWord = morphstr(word1,class1->pos);
    if (useWord) {
      cout << "base form of word is " << useWord << "\n";
      wordInPOS = 1;
    } else {
      wordInPOS = 0;
    }
  }
  if (wordInPost) {
    // now handle search stuff
    int searchDefined = 0;
    SynsetPtr baseStruct1 = NULL;
    SynsetPtr baseStruct2 = NULL;
    SynsetPtr tempStruct1 = NULL;
    if (pos1) {
      searchType1 = is_defined(word1,class1->pos);
      if ((searchType1 & bit(HYPERPTR))) {
	baseStruct1 = findtheinfo_ds(word1,class1->pos,-HYPERPTR,ALLSENSES);
      }
    }
    // gather up all the base forms for this POS
    useWord = morphstr(word1,class1->pos);
    while (useWord) {
      strcpy(tempWord,useWord);
      searchType1 = is_defined(tempWord,class1->pos);
      if (searchType1 & bit(HYPERPTR)) {
	tempStruct1 = findtheinfo_ds(tempWord,class1->pos,-HYPERPTR,ALLSENSES);
	if (!baseStruct1) {
	  baseStruct1 = tempStruct1;
	} else {
	  
    
}

main(int argc, char** argv) {

  int result;
  char word1[50];
  char word2[50];
  unsigned int pos1;
  unsigned int pos2;
  unsigned long searchType1;
  unsigned long searchType2;
  char* hyper1;
  SynsetPtr baseStruct1;
  SynsetPtr baseStruct2;
  char tempWord[50];
  
  result = wninit();
  cout << "wninit() returned " << result << "\n";
  cout << "Enter first word: ";
  cin >> word1;
  cout << "Enter second word: ";
  cin >> word2;
  int pos;
  SemClass* answer = FirstSemMatch(word1,word2);
  // check answer
  if (answer) {
    tempWord[0] = 0;
    SynsetPtr hypersynset = read_synset(answer->pos,answer->offset,tempWord);
    // write out the hypersynset
    cout << "synset returned was\n";
    cout << "hereiam: " << hypersynset->hereiam << "\n";
    cout << "sstype: " << hypersynset->sstype << "\n";
    cout << "fnum: " << hypersynset->fnum << "\n";
    cout << "wcount: " << hypersynset->wcount << "\n";
    for (int k=0; k < hypersynset->wcount; k++) {
      cout << " " << hypersynset->words[k] << " " << hypersynset->wnsense[k] << "\n";
    }
    cout << "Enter third word: ";
    cin >> word1;
    if (WordInSemClass(word1,answer)) {
      cout << word1 << " belongs in computed sem class\n";
    } else {
      cout << word1 << " does not belong in computed sem class\n";
    }
  } else {
    cout << "no match found\n";
  }

}

**/
