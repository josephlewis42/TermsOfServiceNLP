//-*-C++-*-

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "WordStore.h"

WordStore::WordStore() 
{
  for (int i = 0; i < 27; i++)
    m_StorageBlocks[i] = (StorageBlock *) NULL;
}

char * WordStore::StoreWord(char* wordToStore)
{
  int wordLen;
  // first we need to determine which storage block the word should go in
  int blockNum;
  if ((wordToStore[0] >= 'a') && (wordToStore[0] <= 'z'))
    blockNum = wordToStore[0] - 'a';
  else
    blockNum = 26;
  wordLen = strlen(wordToStore);
  char* returnVal;

  // Now search for the word in the block
  if (!m_StorageBlocks[blockNum]) {
    // need to create the block
    m_StorageBlocks[blockNum] = new StorageBlock;
    m_StorageBlocks[blockNum]->nextBlock = (StorageBlock *) 0;
    strcpy(m_StorageBlocks[blockNum]->blockData,wordToStore);
    m_StorageBlocks[blockNum]->nextStart = 
      m_StorageBlocks[blockNum]->blockData + (wordLen + 1);
    return m_StorageBlocks[blockNum]->blockData;
  } else { 
    // we do have a block to work with: see if we already have the word
    StorageBlock* curBlock = m_StorageBlocks[blockNum];
    char *wordStart;
    int  found = 0;
    int cont = 1;

    while (!found && cont) {
      wordStart = curBlock->blockData;
      while ((!found) && 
	     (wordStart + wordLen < curBlock->nextStart)) {
	if (0 == strcmp(wordStart,wordToStore)) {
	  found = 1;
	} else {
	  while (wordStart[0] != 0) 
	    wordStart++;
	  wordStart++;
	}
      }
      if (found) {
	return wordStart;
      } else {
	if (curBlock->nextBlock)
	  curBlock = curBlock->nextBlock;
	else 
	  cont = 0;
      }
    }
    if (curBlock->nextStart + wordLen < curBlock->blockData + 4080) {
      // word will fit in this block
      strcpy(curBlock->nextStart,wordToStore);
      returnVal = curBlock->nextStart;
      curBlock->nextStart = curBlock->nextStart + (wordLen + 1);
      return returnVal;
    } else {
      // have to create a new block
      StorageBlock *newBlock = new StorageBlock;
      newBlock->nextBlock = (StorageBlock *) 0;
      strcpy(newBlock->blockData,wordToStore);
      newBlock->nextStart = newBlock->blockData + (wordLen + 1);
      curBlock->nextBlock = newBlock;
      return newBlock->blockData;
    }
  }
}


void WordStore::PrintStats()
{
  char*         curChar;
  StorageBlock* curBlock;
  int           wordCount,blockCount;

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[0];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("a  -- blocks: %5d  words: %6d\n",blockCount,wordCount);
  
  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[1];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("b  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[2];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("c  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[3];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("d  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[4];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("e  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[5];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("f  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[6];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("g  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[7];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("h  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[8];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("i  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[9];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("j  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[10];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("k  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[11];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("l  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[12];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("m  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[13];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("n  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[14];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("o  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[15];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("p  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[16];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("q  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[17];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("r  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[18];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("s  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[19];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("t  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[20];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("u  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[21];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("v  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[22];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("w  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[23];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("x  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[24];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("y  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[25];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("z  -- blocks: %5d  words: %6d\n",blockCount,wordCount);

  wordCount = 0;
  blockCount = 0;
  curBlock = m_StorageBlocks[26];
  while (curBlock) {
    blockCount++;
    for (curChar = curBlock->blockData; curChar < curBlock->nextStart; curChar++) {
      if (curChar[0] == 0)
	wordCount++;
    }
    curBlock = curBlock->nextBlock;
  }
  printf("sym-- blocks: %5d  words: %6d\n",blockCount,wordCount);

}


