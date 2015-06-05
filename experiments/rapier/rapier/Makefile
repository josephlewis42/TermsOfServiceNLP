rapier : rapier.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Perform.o Learner.o
	clang++ -ggdb -o rapier rapier.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Perform.o Learner.o 

rapier.o : rapier.cxx Document.h WordStore.h Template.h Rule.h Perform.h SkipList.h utils.h RuleBase.h SemClass.h
	clang++ -ggdb  -c rapier.cxx 

test2 : test.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Perform.o Learner.o ComputeResults.o
	clang++ -ggdb -o test2 test.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Perform.o Learner.o ComputeResults.o 

test.o : test.cxx Document.h WordStore.h Template.h Rule.h Perform.h SkipList.h utils.h RuleBase.h SemClass.h
	clang++ -ggdb -c test.cxx 

processDoc : processDoc.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Learner.o
	clang++ -ggdb -o processDoc processDoc.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Learner.o 

processDoc.o : processDoc.cxx Document.h WordStore.h Template.h Rule.h SkipList.h utils.h RuleBase.h SemClass.h
	clang++ -ggdb -c processDoc.cxx 

stats : stats.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Perform.o Learner.o ComputeResults.o
	clang++ -ggdb -o stats stats.o Document.o WordStore.o utils.o Template.o Rule.o SemClass.o RuleBase.o SkipList.o Perform.o Learner.o ComputeResults.o 

stats.o : stats.cxx Document.h WordStore.h Template.h Rule.h Perform.h SkipList.h utils.h RuleBase.h SemClass.h
	clang++ -ggdb -c stats.cxx 


Learner.o : Learner.cxx Document.h WordStore.h Template.h Rule.h Perform.h SkipList.h utils.h RuleBase.h SemClass.h Learner.h
	clang++ -ggdb -c Learner.cxx 

Document.o : Document.cxx Document.h WordStore.h utils.h
	clang++ -ggdb -c Document.cxx 

WordStore.o : WordStore.cxx WordStore.h
	clang++ -ggdb -c WordStore.cxx 

utils.o : utils.cxx utils.h
	clang++ -ggdb -c utils.cxx 

Template.o : Template.cxx Template.h WordStore.h utils.h
	clang++ -ggdb -c Template.cxx 

Rule.o : Rule.cxx Rule.h WordStore.h utils.h SemClass.h Document.h
	clang++ -ggdb -c Rule.cxx 

RuleBase.o : RuleBase.cxx RuleBase.h WordStore.h utils.h SemClass.h Document.h SkipList.h Rule.h
	clang++ -ggdb -c RuleBase.cxx 

SkipList.o : SkipList.cxx SkipList.h utils.h
	clang++ -ggdb -c SkipList.cxx 

SemClass.o : SemClass.cxx SemClass.h
	clang++ -ggdb -c SemClass.cxx 

Perform.o : Perform.cxx Perform.h Document.h WordStore.h Template.h Rule.h SkipList.h utils.h RuleBase.h SemClass.h
	clang++ -ggdb -c Perform.cxx 

ComputeResults.o : ComputeResults.cxx ComputeResults.h Document.h WordStore.h Template.h Rule.h SkipList.h utils.h RuleBase.h SemClass.h
	clang++ -ggdb -c ComputeResults.cxx 














