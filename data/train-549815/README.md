# Data from train n. 549815

The train was run on HY: the corresponding wagon is `UPCCandidateProducerOO` that runs the task `UPCCandidateProducer` with service wagons `eventSelectionService` and `UD_fwdtrack-propagation`, on the dataset `LHC25ae_pass2`.

The data have been downloaded using the script `download-549815.sh`.
The file `pinot.txt` contains the directories of all the downloaded `AO2D.root`. The data have been further analyzed using the `FwdMuonsUPC` task and the configuration file `data-pinot.json`. This work has been done on the ALICE Torino server `alipinot` aka `alz100xl.to.infn.it`, since it has enough RAM there is no need to split the work in different jobs. Note also that the path of the chucks are defined wrt the position of the repository in the `alipinot` machine.