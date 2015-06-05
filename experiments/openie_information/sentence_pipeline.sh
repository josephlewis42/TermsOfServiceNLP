#!/bin/bash
# Copyright 2015 Joseph Lewis <joseph@josephlewis.net>


CONDENSED="/tmp/$1_condensed.txt"
SENTS="$1_sents.txt"

cat "$1" | sed ':a;N;$!ba;s/\n/ /g' > $CONDENSED
./opennlp SentenceDetector en-sent.bin < $CONDENSED > $SENTS

