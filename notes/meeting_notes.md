Definition Parser
-----------------

66% of definitions found with Perceptron auto weighting (90% of defn sentences
were correct from the general extractor)


Improving Paragraph Classification
==================================


        Category        F1 (pos)
        ------------------------
        Anonymity       30 -> 24
        Copyright       40 -> 31
        Definitions     44 -> 36
        Personal Info   68 -> 62    
        Jurisdiction    32 -> 28    
        Harmless:       63 -> 71    +    
        Law Enforcement: 47 -> 67   +
        Data Collection: 52 -> 63   +
        Business Changes: 55 -> 49  
        Rights:         25 -> 40    +
        Third Party:    40 -> 70    +


        ================================================================================
        Positives: 53
        Negatives: 1422
        Anonynimity
        Correct mean: 0.788273615635
                     precision    recall  f1-score   support

                  0       0.97      0.80      0.88       285
                  1       0.20      0.64      0.30        22

        avg / total       0.91      0.79      0.83       307

        ================================================================================
        Positives: 23
        Negatives: 1452
        Data Copyright
        Correct mean: 0.936877076412
                     precision    recall  f1-score   support

                  0       0.99      0.95      0.97       291
                  1       0.29      0.60      0.39        10

        avg / total       0.96      0.94      0.95       301

        ================================================================================
        Positives: 72
        Negatives: 1403
        Definitions
        Correct mean: 0.83922829582
                     precision    recall  f1-score   support

                  0       0.96      0.86      0.91       281
                  1       0.33      0.67      0.44        30

        avg / total       0.90      0.84      0.86       311

        ================================================================================
        Positives: 243
        Negatives: 1232
        Personal Information
        Correct mean: 0.779710144928
                     precision    recall  f1-score   support

                  0       0.91      0.77      0.83       247
                  1       0.58      0.82      0.68        98

        avg / total       0.82      0.78      0.79       345

        ================================================================================
        Positives: 25
        Negatives: 1450
        Jurisdiction
        Correct mean: 0.916666666667
                     precision    recall  f1-score   support

                  0       0.99      0.93      0.96       290
                  1       0.22      0.60      0.32        10

        avg / total       0.96      0.92      0.93       300

        ================================================================================
        Positives: 84
        Negatives: 1391
        Hold Harmless
        Correct mean: 0.916932907348
                     precision    recall  f1-score   support

                  0       0.96      0.95      0.95       279
                  1       0.61      0.65      0.63        34

        avg / total       0.92      0.92      0.92       313

        ================================================================================
        Positives: 36
        Negatives: 1439
        Law Enforcement
        Correct mean: 0.924342105263
                     precision    recall  f1-score   support

                  0       0.98      0.94      0.96       288
                  1       0.37      0.62      0.47        16

        avg / total       0.95      0.92      0.93       304

        ================================================================================
        Positives: 176
        Negatives: 1299
        Data Collection
        Correct mean: 0.807228915663
                     precision    recall  f1-score   support

                  0       0.86      0.90      0.88       260
                  1       0.57      0.47      0.52        72

        avg / total       0.80      0.81      0.80       332

        ================================================================================
        Positives: 50
        Negatives: 1425
        Policy/Business Changes
        Correct mean: 0.881967213115
                     precision    recall  f1-score   support

                  0       0.97      0.90      0.93       285
                  1       0.30      0.60      0.40        20

        avg / total       0.93      0.88      0.90       305

        ================================================================================
        Positives: 100
        Negatives: 1375
        Rights/Responsibility
        Correct mean: 0.844444444444
                     precision    recall  f1-score   support

                  0       0.89      0.94      0.91       275
                  1       0.32      0.20      0.25        40

        avg / total       0.82      0.84      0.83       315

        ================================================================================
        Positives: 182
        Negatives: 1293
        Third Party
        Correct mean: 0.822822822823
                     precision    recall  f1-score   support

                  0       0.86      0.92      0.89       259
                  1       0.63      0.49      0.55        74

        avg / total       0.81      0.82      0.81       333


**After Defns Added**

        ================================================================================
        Positives: 53
        Negatives: 1422
        Anonynimity
        Correct mean: 0.876221498371
                     precision    recall  f1-score   support

                  0       0.94      0.92      0.93       285
                  1       0.21      0.27      0.24        22

        avg / total       0.89      0.88      0.88       307

        ================================================================================
        Positives: 23
        Negatives: 1452
        Data Copyright
        Correct mean: 0.880398671096
                     precision    recall  f1-score   support

                  0       0.99      0.88      0.93       291
                  1       0.19      0.80      0.31        10

        avg / total       0.97      0.88      0.91       301

        ================================================================================
        Positives: 72
        Negatives: 1403
        Definitions
        Correct mean: 0.864951768489
                     precision    recall  f1-score   support

                  0       0.93      0.91      0.92       281
                  1       0.33      0.40      0.36        30

        avg / total       0.88      0.86      0.87       311

        ================================================================================
        Positives: 243
        Negatives: 1232
        Personal Information
        Correct mean: 0.797101449275
                     precision    recall  f1-score   support

                  0       0.84      0.89      0.86       247
                  1       0.67      0.57      0.62        98

        avg / total       0.79      0.80      0.79       345

        ================================================================================
        Positives: 25
        Negatives: 1450
        Jurisdiction
        Correct mean: 0.93
                     precision    recall  f1-score   support

                  0       0.98      0.95      0.96       290
                  1       0.21      0.40      0.28        10

        avg / total       0.95      0.93      0.94       300

        ================================================================================
        Positives: 84
        Negatives: 1391
        Hold Harmless
        Correct mean: 0.926517571885
                     precision    recall  f1-score   support

                  0       0.98      0.94      0.96       279
                  1       0.62      0.82      0.71        34

        avg / total       0.94      0.93      0.93       313

        ================================================================================
        Positives: 36
        Negatives: 1439
        Law Enforcement
        Correct mean: 0.960526315789
                     precision    recall  f1-score   support

                  0       0.99      0.97      0.98       288
                  1       0.60      0.75      0.67        16

        avg / total       0.97      0.96      0.96       304

        ================================================================================
        Positives: 176
        Negatives: 1299
        Data Collection
        Correct mean: 0.849397590361
                     precision    recall  f1-score   support

                  0       0.89      0.92      0.91       260
                  1       0.68      0.58      0.63        72

        avg / total       0.84      0.85      0.85       332

        ================================================================================
        Positives: 50
        Negatives: 1425
        Policy/Business Changes
        Correct mean: 0.875409836066
                     precision    recall  f1-score   support

                  0       0.99      0.87      0.93       285
                  1       0.33      0.90      0.49        20

        avg / total       0.95      0.88      0.90       305

        ================================================================================
        Positives: 100
        Negatives: 1375
        Rights/Responsibility
        Correct mean: 0.847619047619
                     precision    recall  f1-score   support

                  0       0.91      0.91      0.91       275
                  1       0.40      0.40      0.40        40

        avg / total       0.85      0.85      0.85       315

        ================================================================================
        Positives: 182
        Negatives: 1293
        Third Party
        Correct mean: 0.858858858859
                     precision    recall  f1-score   support

                  0       0.93      0.89      0.91       259
                  1       0.66      0.76      0.70        74

        avg / total       0.87      0.86      0.86       333


After Entities Recognized
-------------------------


        Category        F1      F1def   F1 ent 
        ---------------------------------------
        Anonymity       30      24      38
        Copyright       40      31      50
        Definitions     44      36      50
        Personal Info   68      62      62
        Jurisdiction    32      28      14
        Harmless:       63      71      73 
        Law Enforcement 47      67      55
        Data Collection 52      63      64
        Business Chng.  55      49      71
        Rights          25      40      25
        Third Party     40      70      62



        ================================================================================
        Positives: 53
        Negatives: 1422
        Anonynimity
        Correct mean: 0.892508143322
                     precision    recall  f1-score   support

                  0       0.96      0.93      0.94       285
                  1       0.32      0.45      0.38        22

        avg / total       0.91      0.89      0.90       307

        ================================================================================
        Positives: 23
        Negatives: 1452
        Data Copyright
        Correct mean: 0.946843853821
                     precision    recall  f1-score   support

                  0       0.99      0.95      0.97       291
                  1       0.36      0.80      0.50        10

        avg / total       0.97      0.95      0.96       301

        ================================================================================
        Positives: 72
        Negatives: 1403
        Definitions
        Correct mean: 0.88424437299
                     precision    recall  f1-score   support

                  0       0.96      0.91      0.93       281
                  1       0.43      0.60      0.50        30

        avg / total       0.90      0.88      0.89       311

        ================================================================================
        Positives: 243
        Negatives: 1232
        Personal Information
        Correct mean: 0.802898550725
                     precision    recall  f1-score   support

                  0       0.84      0.89      0.87       247
                  1       0.68      0.57      0.62        98

        avg / total       0.80      0.80      0.80       345

        ================================================================================
        Positives: 25
        Negatives: 1450
        Jurisdiction
        Correct mean: 0.92
                     precision    recall  f1-score   support

                  0       0.97      0.94      0.96       290
                  1       0.11      0.20      0.14        10

        avg / total       0.94      0.92      0.93       300

        ================================================================================
        Positives: 84
        Negatives: 1391
        Hold Harmless
        Correct mean: 0.932907348243
                     precision    recall  f1-score   support

                  0       0.98      0.95      0.96       279
                  1       0.65      0.82      0.73        34

        avg / total       0.94      0.93      0.94       313

        ================================================================================
        Positives: 36
        Negatives: 1439
        Law Enforcement
        Correct mean: 0.934210526316
                     precision    recall  f1-score   support

                  0       0.99      0.94      0.96       288
                  1       0.43      0.75      0.55        16

        avg / total       0.96      0.93      0.94       304

        ================================================================================
        Positives: 176
        Negatives: 1299
        Data Collection
        Correct mean: 0.834337349398
                     precision    recall  f1-score   support

                  0       0.91      0.88      0.89       260
                  1       0.61      0.67      0.64        72

        avg / total       0.84      0.83      0.84       332

        ================================================================================
        Positives: 50
        Negatives: 1425
        Policy/Business Changes
        Correct mean: 0.95737704918
                     precision    recall  f1-score   support

                  0       0.99      0.97      0.98       285
                  1       0.64      0.80      0.71        20

        avg / total       0.96      0.96      0.96       305

        ================================================================================
        Positives: 100
        Negatives: 1375
        Rights/Responsibility
        Correct mean: 0.806349206349
                     precision    recall  f1-score   support

                  0       0.89      0.89      0.89       275
                  1       0.24      0.25      0.25        40

        avg / total       0.81      0.81      0.81       315

        ================================================================================
        Positives: 182
        Negatives: 1293
        Third Party
        Correct mean: 0.834834834835
                     precision    recall  f1-score   support

                  0       0.89      0.90      0.89       259
                  1       0.64      0.59      0.62        74

        avg / total       0.83      0.83      0.83       333
        
        
Privacy Heirarchy
-----------------

        Technologies:
            Location:               2222
                Network Adapter:    1
                    Wi-Fi,          151
                GPS signals,        150
                cell tower          47
            
            Tracking:               1348
                cookies,            9252
                JavaScript,         133
                Web Beacons         1788
                LSO                 112
                Flash Cookies       598
                clear gifs          469
                IP                  2566
                MAC                 53
                pixel tags          449
            Control:
                hardware            ??
                software            ??
                filtering services  ??
                
            SSL                     1480
        
        Information:
            mobile device type      49
            location                2222

            Personal Information:   11378
                Personally Identifiable Information: 4386
                    Contact Information:
                        name,       5298
                        address,    8807
                        email,      8666
                        phone number,   1808
                        zip code        376
                    
                    credit card number, 1714
                    payment information 218
                    drivers license number 2
                    social security number 270 (government identification number)
                
                age             4214
                date of birth   193
                credit information  42
                
                demographic information 935
                lifestyle information   33  
                    personal interests  38
                    product preferences 10
                
                survey questions        1224

                
                Non Personally Identifiable Information:
                    anonymous usage data    122
                    referring pages         4
                    platform types          44
                    clickstream             188        
                    IP Address              2566
                    Cookies                 9252
                    log data                121

        Business transaction:
            merger,             414
            acquisition         238

        Business Process
            marketing           ?? Combined with services > advertising
            customer service    1005
            fraud prevention    103
            public safety       31
            legal purposes      25
            sales               435

        Messages:
            newsletters,        1057
            announcements       163

        Services
            advertising         ?? Combined with processes > marketing
            social media plugins    838
            widgets                 187
            behavioral advertising  290

Goal
----

**Short Term**

* Finish ramping up performance for categories
* Generalize across website categories
* Find what everyone is doing (rights given, rights taken away, technologies used and implications)
* construct a condensed narrative?

**Long term**

Provide user with interesting info about privacy policy versus others; taking into account personal preferences.

Next steps
----------

* Use the improved parsers to categorize more documents -> need more data for confusion of some categories
    * semisupervised?
* add in information about website "type" -> may improve performance
* Look for common definitions/terms/paragraphs sections between types -> classify who has what using defns for types and/or differences
* Run Illinois SRL on paragraphs to look for common arguments/verbs for different types -> find who uses what, gives what
* Use definitions of entities to resolve over whole document

Resources
---------

* Mozilla Collusion
* Illinois SRL Toolkit
* Conceptnet
* Wikidata
