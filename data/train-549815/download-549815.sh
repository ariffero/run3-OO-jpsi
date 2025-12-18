# script to download the data from the output of the HY train n. 549815
# the train ran UPCCandidateProducerOO on the dataset LHC25ae_pass2
echo "Starting download of HY train n. 549815 output"
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351644/AOD/*/AO2D.root file://hy_3351644
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351642/AOD/*/AO2D.root file://hy_3351642
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351641/AOD/*/AO2D.root file://hy_3351641
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351640/AOD/*/AO2D.root file://hy_3351640
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351639/AOD/*/AO2D.root file://hy_3351639
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351638/AOD/*/AO2D.root file://hy_3351638
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351637/AOD/*/AO2D.root file://hy_3351637
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351636/AOD/*/AO2D.root file://hy_3351636
alien_cp alien:///alice/cern.ch/user/a/alihyperloop/jobs/0335/hy_3351635/AOD/*/AO2D.root file://hy_3351635
echo "Download completed."