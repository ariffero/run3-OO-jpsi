# Luminosity from HY

This folder contains the macros to compute the luminosity of the sample on which one has run a train using the UPCCandidateProducer.

Use the script `lumi.py` to downolad all the necessary files from the output of a certain train and analyzed them with the macro `getLumi.cpp` (wrote by Roman Lavicka). Run the script with option `--help` to have info on how to use it.

The file `trg-count-25ae.cvs` contains the per run number of trigger counts.

**Important**: for the moment the reference cross section is not the correct one for OO collisions, so the results obtained here are completely wrong. The fraction of the total luminosoty can be estimated looking at the TCE trigger counts.