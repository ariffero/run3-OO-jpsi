// variables saved in data and MC trees

#ifndef DIMU_VARS_H
#define DIMU_VARS_H

  // DATA ------------------------------------------------------------

  // TTree globl pionter
  TTree *globalTree = NULL;

  // Declaration of leaf types
   Int_t           fRunNumber;
   Float_t         fM;
   Float_t         fPt;
   Float_t         fRap;
   Float_t         fPhi;
   Float_t         fPtp;
   Float_t         fEtap;
   Float_t         fPhip;
   Int_t           fTrackTypep;
   Float_t         fPtn;
   Float_t         fEtan;
   Float_t         fPhin;
   Int_t           fTrackTypen;
   Int_t           fNclass;

   void createDataTree(TFile *globalFile, string treePath){
    globalTree = (TTree*)globalFile->Get(treePath.c_str());

    globalTree->SetBranchAddress("fRunNumber", &fRunNumber);
    globalTree->SetBranchAddress("fM", &fM);
    globalTree->SetBranchAddress("fPt", &fPt);
    globalTree->SetBranchAddress("fRap", &fRap);
    globalTree->SetBranchAddress("fPhi", &fPhi);
    globalTree->SetBranchAddress("fPtp", &fPtp);
    globalTree->SetBranchAddress("fEtap", &fEtap);
    globalTree->SetBranchAddress("fPhip", &fPhip);
    globalTree->SetBranchAddress("fTrackTypep", &fTrackTypep);
    globalTree->SetBranchAddress("fPtn", &fPtn);
    globalTree->SetBranchAddress("fEtan", &fEtan);
    globalTree->SetBranchAddress("fPhin", &fPhin);
    globalTree->SetBranchAddress("fTrackTypen", &fTrackTypen);
    globalTree->SetBranchAddress("fNclass", &fNclass);
  }

#endif