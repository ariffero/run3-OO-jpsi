# Data from train n. 455531
The train was run on HY: the corresponding wagon is `UPCCandidateProducerOO` that runs the task `UPCCandidateProducer` with service wagons `eventSelectionService` and `UD_fwdtrack-propagation`, on the dataset `LHC25ae_pass1`.

The data have been downloaded using the script `download-455531.sh`.
The file `file_list.txt` contains the directories of all the downloaded `AO2D.root`. The data have been further analyzed using the `FwdMuonsUPC` task and the configuration file `data-OO-full-pt.json`: the results are in this folder, while the results of the single jobs are in the `/jobs` sub-directory.