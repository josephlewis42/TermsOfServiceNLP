#!/usr/bin/env python3
'''

Copyright 2014 Joseph Lewis <joehms22@gmail.com> | <joseph@josephlewis.net>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the
  distribution.
* Neither the name of the  nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


'''

i = 0

with open("definitions_extracted.txt") as definitions:
    text = definitions.read()
    defs = text.split("---\n")
    
    for definition in defs:
        if not definition:
            continue
        i += 1
        
        lines = definition.split("\n")
        if len(lines) != 10:
            print("fail")
            print definition
            exit()
        raw = lines[0]
        cleaned = lines[1]
        joined_again = lines[3]
        pos_tagged = lines[4]
        hmm_tagged = lines[5]
        
        termline = ""
        defline = ""
        
        for line in lines:
            if "/NN" in line:
                pos_tagged = line
            if "/out" in line:
                hmm_tagged = line
            if "term: " in line:
                termline = line
            elif line.startswith("definition: "):
                defline = line
            
        print "---"
        print termline
        print defline
        
        with open("rapier_output/defn_{}.orig".format(i), 'w') as tmp:
            tmp.write(raw)
        
        with open("rapier_output/defn_{}_out".format(i), 'w') as tmp:
            tmp.write(pos_tagged)
        with open("rapier_output/defn_{}.template".format(i), 'w') as tmp:
            tmp.write("tos_definition\n")
            tmp.write("*" + termline)
            tmp.write("\n*" + defline)

