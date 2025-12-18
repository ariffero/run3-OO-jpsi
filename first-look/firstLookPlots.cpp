// -------------------------------------- //
// macro to produce some basic histos for a first look to the data
// -------------------------------------- //

// All headers defined here
// C++ headers
#include <Riostream.h>
#include <filesystem>
// ROOT headers
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMath.h"
#include "TLatex.h"
// custom headers
#include "../library/dimuVars.h"

//global values: kine cuts
float lowPt = 0;
float upPt = 2.;
float lowMass = 2;
float upMass = 4.5;
float lowRap = -4;
float upRap = -2.5;

// masses
const float massMuon = 0.105658; // gev

// do some cosmetics on TH1
void cosmetics(TH1 *h){
  h->SetMarkerStyle(20);
  h->SetMarkerSize(0.5);

  h->SetMarkerColor(kBlue+2);
  h->SetLineColor(kBlue+2);

  h->SetLineWidth(3);

  h->GetYaxis()->SetTitle("Count per bin");
}

// entry point
void firstLookPlots(string dataType = "OO_full_pt"){

  string filePath = "";
  string treePath = "";
  string outputFile = "";

  if(dataType == "OO_coh"){
    filePath = "../data/train-442464/data-OO-tree.root";
    treePath = "DF_2423890592977248/dimu";
    outputFile = "fl-442464.root";

    lowPt = 0;
    upPt  = 0.25;
  }
  else if(dataType == "OO_full_pt"){
    filePath = "../data/train-442464/data-OO-full-pt-tree.root";
    treePath = "DF_2423890592977248/dimu";
    outputFile = "fl-442464-full-pt.root";

    lowPt = 0;
    upPt  = 1.;
  }
  else if(dataType == "OO_coh_wide_mass"){
    filePath = "../data/train-442464/data-OO-full-pt-tree.root";
    treePath = "DF_2423890592977248/dimu";
    outputFile = "fl-442464-wide-mass.root";

    lowPt = 0;
    upPt  = 0.25;
  }
  else if(dataType == "OO_full_cpass0_coh"){
    filePath = "../data/train-447456/merged-trees.root";
    treePath = "DF_2423890592971072/dimu";
    outputFile = "fl-447456-wide-mass.root";

    lowPt = 0;
    upPt  = 0.25;
  }
  else if(dataType == "OO_apass1_coh"){
    filePath = "../data/train-455531/data-OO-full-pt-tree.root";
    treePath = "DF_2423890593933632/dimu";
    outputFile = "fl-455531-wide-mass.root";

    lowPt = 0;
    upPt  = 0.25;
  }

  // apply kinematic cuts?
  bool applyKine = true;

  if(!filesystem::exists(filePath)){
    cout<<"The file "<<filePath<<" does not exist."<<endl;
    return;
  }

  // opening the file that stores the tree 
  TFile *file = new TFile(filePath.c_str());
  createDataTree(file,treePath.c_str());
  if(globalTree==NULL){
    cout<<"The not found. Bye!"<<endl;
    return;
  }

  // pair distributions: energy, mass, pt, phi, rapidity
  int nBins = 30;
  TH1D *hMass = new TH1D("hMass", Form("Invariant mass distribution - %s", dataType.c_str()), nBins, lowMass, upMass);
  hMass->GetXaxis()->SetTitle("m_{#mu#mu} (GeV/#it{c}^{2})");
  TH1D *hPt = new TH1D("hPt", Form("Transverse momentum distribution - %s", dataType.c_str()), nBins, lowPt, upPt);
  hPt->GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
  TH1D *hPhi = new TH1D("hPhi", Form("#varphi distribution - %s", dataType.c_str()), nBins, -TMath::Pi(), TMath::Pi());
  hPhi->GetXaxis()->SetTitle("#varphi");
  TH1D *hRapidity = new TH1D("hRapidity", Form("Rapidity distribution - %s", dataType.c_str()), nBins, lowRap, upRap);
  hRapidity->GetXaxis()->SetTitle("y");

  // pt vs mass
  TH2D *hMassPt = new TH2D("hMassPt", Form("#it{p}_{T} vs mass- %s", dataType.c_str()), nBins/3, lowMass, upMass, nBins/2, lowPt, upPt);
  hMassPt->GetXaxis()->SetTitle("m_{#mu#mu} (GeV/#it{c}^{2})");
  hMassPt->GetYaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");

  // fill the histos
  for(Long64_t ev=0; ev<globalTree->GetEntries(); ev++){
    globalTree->GetEvent(ev);

    // aply kinematics cuts if requested
    if(applyKine){
      if(fPt < lowPt) continue;
      if(fPt > upPt) continue;

      if(fM < lowMass) continue;
      if(fM > upMass) continue;

      if(fRap < lowRap) continue;
      if(fRap > upRap) continue;
    }

    // pair distribs
    hMass->Fill(fM);
    hPt->Fill(fPt);
    hPhi->Fill(fPhi);
    hRapidity->Fill(fRap);

    // pt vs mass
    hMassPt->Fill(fM, fPt);
  }

  cosmetics(hMass);
  cosmetics(hPt);
  cosmetics(hPhi);
  cosmetics(hRapidity);

  // number of candidates
  // Compute integrals
  double total = hMass->Integral();
  int binLow  = hMass->GetXaxis()->FindBin(2.9);
  int binHigh = hMass->GetXaxis()->FindBin(3.3);
  double inRange = hMass->Integral(binLow, binHigh);
  cout<<"n. candidates in (2.9, 3.3) = "<<inRange<<endl;

  // do some cosmetics for the mass plot
  gStyle->SetOptStat(0);
  TCanvas *c = new TCanvas("c","Canvas for pT label",1920,1080);
  c->cd();

  TLatex latex;
  latex.SetNDC();
  latex.SetTextFont(42);
  latex.SetTextSize(0.04);
  latex.SetTextColor(kBlack);

  hMass->GetYaxis()->SetRangeUser(0,170);
  hMass->Draw("pe");

  latex.DrawLatex(0.15, 0.85, "#bf{Coherent J/#psi in UPC at fwd rapidity}");
  if(dataType == "OO_full_cpass0_coh") latex.DrawLatex(0.55, -0.15+0.8,  Form("Full cpass0 sample"));
  if(dataType == "OO_apass1_coh") latex.DrawLatex(0.55, -0.15+0.8,  Form("Full apass1 sample"));
  latex.DrawLatex(0.55, -0.15+0.75, Form("p_{T} < %.2f GeV",upPt));
  latex.DrawLatex(0.55, -0.15+0.7,  Form("%.1f < y_{#mu#mu} < %.2f GeV",lowRap,upRap));
  latex.DrawLatex(0.55, -0.15+0.65, Form("J/#psi cands. (m_{#mu#mu}#in [2.9, 3.3]) = %d",(int)inRange));
  
  c->Update();

  // save the results
  TFile *fRes = new TFile(outputFile.c_str(),"recreate");
  hMass->Write();
  hPt->Write();
  hPhi->Write();
  hRapidity->Write();
  hMassPt->Write();
  fRes->Close();
}