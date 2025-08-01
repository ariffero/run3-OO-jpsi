//
// makes a fit to the invariant mass distribution of muon pairs (at fwd rapidity)
// fit function is J/psi + psi(2s) (optional)
// it is possible to choose the number of bins in phi and the neutron emission class
// it is also possible to select kinematic regions, using a config.cfg file (needed)
//

// -----------------------------------------------------------------
// all headers are defined here
// -----------------------------------------------------------------

// c++ headers
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

// root headers
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TStyle.h"
#include "TMath.h"
#include "TLatex.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TF1.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TLine.h"

// RooFit headers
#include "RooGlobalFunc.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooCBShape.h"
#include "RooExponential.h"
#include "RooFitResult.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooBinning.h"
#include "RooAbsReal.h"
#include "RooGenericPdf.h"
#include "RooCrystalBall.h"
#include "RooFormulaVar.h"
#include "RooHist.h"
#include "RooMsgService.h"

using namespace RooFit;

// My headers: needed to save the results in the tree
#include "../library/savedVarInMassFits.h"

// -----------------------------------------------------------------
// global variables, to be set to drive the fits
// -----------------------------------------------------------------

// --> kinematics: will be set to the values in a config
// the values set here will not be taken into account if values are present in the config
double minPt = 0;
double maxPt = 0;
double minMass = 0;
double maxMass = 0;
double minRapidity = 0;
double maxRapidity = 0;

// --> Crystall ball parameters
double nParL = 140;
double nParR = 0.1;
double sigmaL = 0.073;
double sigmaR = 0.061;
double alphaL = 1.08;
double alphaR = 2.8;

// Expo3 (as Nazar) for the bkg
// func = exp(p_1*m + p_2*m^2)
double p_1 = -2.55;
double p_2 = 0.2;

// --> switches 
// for the CrystalBall
bool isNFixed = false;
bool isSigmaFixed = false;
bool isAlphaFixed = false;
bool includePsi2s = false;
bool isMassFixed = false;

// for the bkg: if true p_2=0 -> bkg = exp(p_1*m)
bool useExpoBkg = true;
// chi2 fit: not used
bool isChi2Fit = false;
// to exclude j/psi, for fits outside the j/psi region
bool excludeJPsi = false;
// to exclude the continuum
bool excludeBkg = false;
// fit data or reco MC
bool gIsMC = false;

// -----------------------------------------------------------------
// global variables useful in the macro
// -----------------------------------------------------------------
// global variable for easy binning in a chosen variable
int gBins = 1;
// global variable for neutron classes
string gNeutronClass = "";
// name of the file with the results
string gSaveFileName = "";
// name of the tree with the results
string gSaveTreeName = "";
// name of the (1st) folder that contains the results
string gIdentifier = "";

// position of the legend info
float gXpos = 0.15;
float gYpos = 0.80;

// -----------------------------------------------------------------
// all functions are defined here
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// do one data fit
void doOneDataFit(TTree *dataTree, TFile *saveFile, float binID[3], 
                  TTree *recoCohJPsiTree, TTree *genCohJPsiTree, 
                  TTree *recoMumuMidTree, TTree *genMumuMidTree)
{
  // number of times the function has been called
  static int nCalls = 0;

  // define variables for the tree
  RooRealVar pt("fPt","#it{p}_{T} (GeV/#it{c})",minPt,maxPt);
  RooRealVar mass("fM","m_{#mu#mu} (GeV/#it{c}^{2})",minMass,maxMass);
  RooRealVar rapidity("fRap","rapidity",minRapidity,maxRapidity);
  RooRealVar genPt("fGenPt","gen #it{p}_{T} (GeV/#it{c})",minPt,maxPt);
  RooRealVar genMass("fGenM","gen m_{#mu#mu} (GeV/c^{2})",minMass,maxMass);
  RooRealVar genRapidity("fGenRap","gen rapidity",minRapidity,maxRapidity);
  // neutron class from neutron ID
  // 0n0n == 1
  // Xn0n == 2
  // 0nXn == 3
  // XnXn == 4
  RooRealVar *neutronID = NULL;

    if(gNeutronClass=="noSelection"){
    neutronID = new RooRealVar("fNclass","znClass",1,4);
  }
  else if(gNeutronClass=="0n0n"){
    neutronID = new RooRealVar("fNclass","znClass",1,1);
  }
  else if(gNeutronClass=="Xn0n"){
    neutronID = new RooRealVar("fNclass","znClass",2,2);
  }
  else if(gNeutronClass=="XnXn"){
    neutronID = new RooRealVar("fNclass","znClass",4,4);
  }

  // import the tree of the data
  // define the variables to put in inData
  RooArgSet *vars = NULL;
  // for real data:
  if(!gIsMC) vars =  new RooArgSet(*neutronID, pt, mass, rapidity);
  // for reco MC
  else if(gIsMC) vars =  new RooArgSet(pt, mass, rapidity);
  // create the dataset
  RooDataSet inData("inData", "inData", *vars, Import(*dataTree));

  // number of events
  int nEvents = inData.numEntries();
  // check the structure of the data
  inData.Print();

  // define the variables to put in reco
  RooArgSet *recoVars = new RooArgSet(pt, mass, rapidity);
  // define the variables to put in gen
  RooArgSet *genVars = new RooArgSet(genPt, genMass, genRapidity);

  // create datasets for MC
  // coherent jpsi reco MC
  RooDataSet recoCohJPsiData("recoCohJPsiData", "recoCohJPsiData", recoCohJPsiTree, *recoVars);
  // coherent jpsi gen MC
  RooDataSet genCohJPsiData("genCohJPsiData", "genCohJPsiData", genCohJPsiTree, *genVars);
  // reco MC yy->mumu mid
  RooDataSet recoMumuMidData("recoMumuMidData", "recoMumuMidData", recoMumuMidTree, *recoVars);
  // gen MC yy->mumu mid
  RooDataSet genMumuMidData("genMumuMidData", "genMumuMidData", genMumuMidTree, *genVars);

  // create the histos for MC and AxE
  int binsAxE = 1;
  // j/psi
  TH1D *hRecoCohJpsi = (TH1D*)recoCohJPsiData.createHistogram("hRecoCohJpsi", mass, Binning(binsAxE,minMass,maxMass));
  hRecoCohJpsi->GetXaxis()->SetTitle("m_{#mu#mu} (GeV/#it{c}^{2})");
  TH1D *hGenCohJpsi = (TH1D*)genCohJPsiData.createHistogram("hGenCohJpsi", genMass, Binning(binsAxE,minMass,maxMass));
  hGenCohJpsi->GetXaxis()->SetTitle("m_{#mu#mu} (GeV/#it{c}^{2})");
  TH1D *hAxECohJPsi = (TH1D*)hRecoCohJpsi->Clone("hAxECohJPsi");
  hAxECohJPsi->Divide(hRecoCohJpsi,hGenCohJpsi,1,1,"B");
  hAxECohJPsi->SetTitle("Acc#times#epsilon (coh J/#psi)");
  // yy-> mumu
  TH1D *hRecoMumuMid = (TH1D*)recoMumuMidData.createHistogram("hRecoMumuMid", mass, Binning(binsAxE,minMass,maxMass));
  hRecoMumuMid->GetXaxis()->SetTitle("m_{#mu#mu} (GeV/#it{c}^{2})");
  TH1D *hGenMumuMid = (TH1D*)genMumuMidData.createHistogram("hGenMumuMid", genMass, Binning(binsAxE,minMass,maxMass));
  hGenMumuMid->GetXaxis()->SetTitle("m_{#mu#mu} (GeV/#it{c}^{2})");
  TH1D *hAxEMumuMid = (TH1D*)hRecoCohJpsi->Clone("hAxEMumuMid");
  hAxEMumuMid->SetTitle("Acc#times#epsilon (#gamma#gamma#rightarrow#mu#mu)");


  // Build pdfs
  // --> j/psi

  // Get the PDG database instance
  TDatabasePDG *pdgDB = TDatabasePDG::Instance();
  // Retrieve the J/psi particle using its PDG code 443
  TParticlePDG *jpsiPart = pdgDB->GetParticle(443);

  RooRealVar m0("m0", "m0", jpsiPart->Mass(), 2.9, 3.2);
  if(isMassFixed){
    m0.setConstant(true);
  }
  RooRealVar sL("sigmaL", "sigmaL",sigmaL,0.01,0.2);
  RooRealVar sR("sigmaR", "sigmaR",sigmaR,0.01,0.2);
  if (isSigmaFixed) {
    sL.setConstant(true);
    sR.setConstant(true);    
  }
  RooRealVar nL("nL", "nL", nParL,130,150);
  RooRealVar nR("nR", "nR", nParR,0.01,10);
  if (isNFixed) {
    nL.setConstant(true);
    nR.setConstant(true);    
  }
  RooRealVar aL("alphaL", "alphaL", alphaL,0.9,1.2);
  RooRealVar aR("alphaR", "alphaR", alphaR,2.5,4);
  if (isAlphaFixed) {
    aL.setConstant(true);
    aR.setConstant(true);    
  }
  RooCrystalBall jpsi("jpsi", "jpsi", mass, m0, sL, sR, aL, nL, aR, nR);

  // --> psi2s
  RooFormulaVar m02("m02","@0+3.686097-3.096900",RooArgList(m0));
  // 1.09 = sqrt(Mpsi2s/Mjpsi)
  RooFormulaVar sL2("sigmaL2","@0*(1.09)",RooArgList(sL));
  RooFormulaVar sR2("sigmaR2","@0*(1.09)",RooArgList(sR));
  RooFormulaVar nL2("nL2","@0",RooArgList(nL));
  RooFormulaVar nR2("nR2","@0",RooArgList(nR));
  RooFormulaVar aL2("aL2","@0",RooArgList(aL));
  RooFormulaVar aR2("aR2","@0",RooArgList(aR));
  RooCrystalBall psi2s("psi2s", "psi2s", mass, m02, sL2, sR2, aL2, nL2, aR2, nR2);

  // --> non-resonant bkg, as Nazar
  // Define parameters p1, p2
  // bkg = exp(p1*mass + p2*mass^2)
  RooRealVar p1("p1", "p1", p_1, -100, 0.0);
  RooRealVar p2("p2", "p2", p_2, -5, 5);
  if(useExpoBkg){
    p2.setVal(0);
    p2.setConstant(kTRUE);
  }
  RooGenericPdf nrBg("nrBg", "nrBg", "exp(@1*@0 + @2*@0*@0)", RooArgList(mass, p1, p2));

  // model and fit
  // --> combine the PDFs
  RooRealVar nJpsi("N_{J/#psi}","Number of J/psi events",0.8*nEvents,0.05*nEvents,nEvents);
  RooRealVar nPsi2s("N_{#psi'}","Number of psi(2s) events",0.05*nEvents,0,nEvents);
  RooRealVar nBg("N_{bg}","Number of BG events",0.15*nEvents,0,nEvents);
  if(excludeBkg){
    p1.setVal(0);
    p1.setConstant(kTRUE);
    nBg.setVal(0);
    nBg.setConstant(kTRUE);
  }

  RooAddPdf *fitData = NULL;
  if (includePsi2s) {
    fitData = new RooAddPdf("fitData","J/psi, psi(2s), and background PDF", RooArgList(jpsi,psi2s,nrBg), RooArgList(nJpsi,nPsi2s,nBg));
  } else {
    fitData = new RooAddPdf("fitData","J/psi and background PDF", RooArgList(jpsi,nrBg), RooArgList(nJpsi,nBg));
  }
  if(excludeJPsi){
    fitData = new RooAddPdf("fitData","Background PDF", RooArgList(nrBg), RooArgList(nBg));
  }

  // --> perform the fit
  RooFitResult *r = NULL;
  r = fitData->fitTo(inData,Save(),Extended(kTRUE),Save());

  // check the status of the fit
  int fitStatus = r->status();
  cout<<"Fit status == "<<fitStatus<<endl;

  // Get the output
  // Create a frame with only the histo and the fit curve
  int massBins = 32;
  RooPlot *frameFit = mass.frame(Title("Frame with data and full fit curve"), Bins(massBins));
  // plot the data
  inData.plotOn(frameFit, Binning(massBins), DataError(RooAbsData::SumW2));
  // Plot the full model
  fitData->plotOn(frameFit, LineColor(kBlack));

  // Now compute the residuals and pulls
  RooHist* residualHist = frameFit->residHist();
  residualHist->SetTitle("Residuals");
  RooHist* pullHist     = frameFit->pullHist();
  pullHist->SetTitle("Pulls");

  // use this fram to compute the chi2
  int nParams = r->floatParsFinal().getSize();
  int ndf = massBins - nParams;
  double chi2_red = frameFit->chiSquare(nParams);

  // Get the number of points in the pull histogram
  int nPoints = pullHist->GetN();
  double chi2Pull = 0.0;
  // Get pointer to the array of y-values (the pulls)
  double* pulls = pullHist->GetY();
  // Loop over all points and sum up the squares of the pulls
  for (int i = 0; i < nPoints; ++i) {
    chi2Pull += pulls[i] * pulls[i];
  }
  cout << "Computed chi2 from pulls: " << chi2Pull << std::endl;

  // --> Draw residuals and pulls
  TCanvas *cStat = new TCanvas("cStat", "cStat", 800, 600);
  cStat->Divide(2,0);
  cStat->cd(1);
    residualHist->Draw("AP");
    cStat->Update();
  cStat->cd(2);
    pullHist->Draw("AP");
    cStat->Update();

  // --> Now create a fram with all the components
  RooPlot *frame = mass.frame(Title(Form("m_{#mu#mu} mass distr. - %s", gNeutronClass.c_str())),Bins(massBins));
  // draw the data
  inData.plotOn(frame,Binning(massBins),DataError(RooAbsData::SumW2));
  // draw the fit
  fitData->plotOn(frame, LineColor(kBlack));
  // draw the components
  if(!excludeJPsi){
    fitData->plotOn(frame,Name("jpsi"), Components(jpsi), LineColor(kRed+1));
    if (includePsi2s) fitData->plotOn(frame,Name("psi2s"), Components(psi2s), LineColor(kGreen+2));
  }  
  fitData->plotOn(frame,Name("nrBg"), Components(nrBg), LineColor(kBlue+1));
  fitData->paramOn(frame,Layout(0.61, 0.8, 0.8));

  // --> do the plot
  TCanvas *c = new TCanvas(Form("m_{#mu#mu} mass distr. - %s", gNeutronClass.c_str()),
                           Form("m_{#mu#mu} mass distr. - %s", gNeutronClass.c_str()),1920,1080);

  // Create 3 pads: pad1 for the fit, pad2 for the pulls, and pad3 for the correlation matrix
  TPad *pad1 = new TPad("pad1", "Fit Pad",   0.0, 0.25, 0.7, 1.0);
  TPad *pad2 = new TPad("pad2", "Pulls Pad", 0.0, 0.05, 0.7, 0.28);
  TPad *pad3 = new TPad("pad3", "Corr matrix Pad", 0.65, 0.0, 1.0, 1.0);

  // Draw the pads on the canvas
  c->cd();
  pad1->Draw();
  pad2->Draw();
  pad3->Draw();

  // Draw fit result on pad1
  pad1->cd();
  pad1->SetBottomMargin(1);
  frame->GetXaxis()->SetTitle();
  frame->Draw();

  // Add chi2 and some info of the fit
  TLatex latex;
  latex.SetNDC();
  latex.SetTextSize(0.04); // Set text size
  latex.DrawLatex(gXpos, gYpos, Form("#chi^{2}/ndf = %.2f (%.2f/%d)", chi2_red,chi2_red*ndf,ndf));
  latex.DrawLatex(gXpos, gYpos-0.05, Form("# entries = %.d", inData.numEntries()));
  latex.DrawLatex(gXpos, gYpos-0.10, Form("#it{p}_{T} in (%.2f, %.2f) GeV/#it{c}",minPt,maxPt));
  latex.DrawLatex(gXpos, gYpos-0.15, Form("m_{#mu#mu} in (%.2f, %.2f) GeV/#it{c}^{2}",minMass,maxMass));
  latex.DrawLatex(gXpos, gYpos-0.20, Form("Y in (%.2f, %.2f)",minRapidity, maxRapidity));
  //latex.DrawLatex(gXpos, gYpos-0.25, Form("neutron class = %s",gNeutronClass.c_str()));

  pad1->Update();

  // Draw the pulls on pad2
  pad2->cd();
  pad2->SetBottomMargin(0.28);
  pullHist->SetTitle("");
  pullHist->GetYaxis()->SetTitleSize(0.1);
  pullHist->GetYaxis()->SetLabelSize(0.1);
  pullHist->GetYaxis()->SetTitleOffset(0.35);
  pullHist->GetYaxis()->SetLabelOffset(0.008);
  pullHist->GetXaxis()->SetTitleSize(0.12);
  pullHist->GetXaxis()->SetLabelSize(0.12);
  pullHist->GetXaxis()->SetTitleOffset(1.1);
  pullHist->GetXaxis()->SetLabelOffset(0.01);

  pullHist->Draw();
  TLine *topFit = new TLine(minMass,1,maxMass,1);
  topFit->SetLineColor(kGreen+2);
  topFit->SetLineStyle(3);
  topFit->Draw("same");
  pullHist->Draw("same");

  pad2->Update();

  // obtain and draw the correlation matrix
  pad3->cd();
  TH2* hCorrMatrix = r->correlationHist();
  hCorrMatrix->SetName(Form("hCorrMatrix%d",nCalls));
  hCorrMatrix->SetTitle("Correlation matrix");
  hCorrMatrix->SetMarkerSize(1.2);
  hCorrMatrix->Draw("col,text");
  hCorrMatrix->SetStats(0);
  pad3->Update();

  // --> print some values
  cout << " chi2/ndf = " << chi2_red << endl;
  cout << " Entries  = " << inData.numEntries() << endl;
  cout << " nJpsi    = " << nJpsi.getVal() << " ± " << nJpsi.getError() << endl;

  gStyle->SetOptFit(1);

  // --> save a PDF file with the result
  if(nCalls==0) c->Print(Form("%s/mass-fits-%d-%s.pdf[",gIdentifier.c_str(),gBins,gNeutronClass.c_str()));
  c->Print(Form("%s/mass-fits-%d-%s.pdf",gIdentifier.c_str(),gBins,gNeutronClass.c_str()));
  if(nCalls==gBins-1) c->Print(Form("%s/mass-fits-%d-%s.pdf]",gIdentifier.c_str(),gBins,gNeutronClass.c_str()));

  saveFile = new TFile(Form("%s/%s/%s",gIdentifier.c_str(),gNeutronClass.c_str(),gSaveFileName.c_str()),"update");

  // save the results of the fit in a tree
  createSaveFitTree(gSaveTreeName);

  // fill the variables in the tree
  numJPsi = nJpsi.getVal();
  errNumJPsi = nJpsi.getError();
  numBkg = nBg.getVal();
  errNumBkg = nBg.getError();
  entries = inData.numEntries();
  errEntries = TMath::Sqrt(entries);
  AxECohJPsi = hAxECohJPsi->GetBinContent(1);
  errAxECohJPsi = hAxECohJPsi->GetBinError(1);
  numJPsiCorr = numJPsi/AxECohJPsi;
  numBkgCorr = numBkg/AxEMumuMid;
  // compute the error on the corrected numer of j/psi propagating the errors from the fit and from the AxECohJPsi
  errNumJPsiCorr = TMath::Sqrt((errNumJPsi/AxECohJPsi)*(errNumJPsi/AxECohJPsi) + (numJPsi/(AxECohJPsi*AxECohJPsi)*errAxECohJPsi)*(numJPsi/(AxECohJPsi*AxECohJPsi)*errAxECohJPsi));
  // compute the error on the corrected bkg propagating the errors from the fit and from the AxECohJPsi
  errNumBkgCorr = TMath::Sqrt((errNumBkg/AxEMumuMid)*(errNumBkg/AxEMumuMid) + (numBkg/(AxEMumuMid*AxEMumuMid)*errAxEMumuMid)*(numBkg/(AxEMumuMid*AxEMumuMid)*errAxEMumuMid));

  saveFitTree->Fill();
  saveFile->Write();
  saveFile->Close();

  nCalls++;
} // end of doOneDataFit


// -----------------------------------------------------------------
// entry point: set up and call fitting function
// fitJPsiInPhiBins(string nClass = "noSelection", int nPhiBins = 12, const char *config = "jPsi", bool isMC = false, bool notShow = true)
void fitJPsi(const char *config = "jPsi", string nClass = "noSelection", int nBins = 1, bool isMC = false, bool notShow = false)
{
  // choose to show or not the canvas and info from RooFit
  if(notShow){
    gROOT->SetBatch(kTRUE);
    RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
  }

  // set the global values to the values in input
  // identifier = name of the config, it identifies the kine for a certain process
  gIdentifier = config;
  // nutron class
  gNeutronClass = nClass;
  // is MC or data?
  gIsMC = isMC;

  // get the tree with the data
  TFile *dataFile = NULL;
  TTree *dataTree = NULL;

  // get the trees with the reco MC of coh j/psi
  TFile *recoCohJPsiFile = NULL;
  TTree *recoCohJPsiTree = NULL;

  // get the trees with the reco MC of coh j/psi
  TFile *genCohJPsiFile = NULL;
  TTree *genCohJPsiTree = NULL;

  // get the trees with the reco MC of yy to mumu in the j/psi mass region
  TFile *recoMumuMidFile = NULL;
  TTree *recoMumuMidTree = NULL;

  // get the trees with the reco MC of yy to mumu in the j/psi mass region
  TFile *genMumuMidFile = NULL;
  TTree *genMumuMidTree = NULL;

  recoCohJPsiFile = new TFile("~/Desktop/run3-jpsi-analysis/MonteCarlo/reco_tree.root");
  recoCohJPsiTree = (TTree*) recoCohJPsiFile->Get("DF_2336518085565631/dimu"); 
  
  genCohJPsiFile = new TFile("~/Desktop/run3-jpsi-analysis/MonteCarlo/gen_tree.root");
  genCohJPsiTree = (TTree*) genCohJPsiFile->Get("DF_2336518085565631/dimu"); 

  recoMumuMidFile = new TFile("~/Desktop/run3-jpsi-analysis/MonteCarloMumuMid/reco_tree.root");
  recoMumuMidTree = (TTree*) recoMumuMidFile->Get("DF_2336518081075359/dimu");

  genMumuMidFile = new TFile("~/Desktop/run3-jpsi-analysis/MonteCarloMumuMid/gen_tree.root");
  genMumuMidTree = (TTree*) genMumuMidFile->Get("DF_2336518081075359/dimu");

  if(!isMC){
    dataFile = new TFile("../data/train-447456/merged-trees.root");
    dataTree = (TTree*) dataFile->Get("DF_2423890592971072/dimu");
  }
  else if(isMC){
    dataTree = recoCohJPsiTree;
    gNeutronClass = "noSelection";
  }

  // check if the data are there
  if(dataTree==NULL || recoCohJPsiTree==NULL || genCohJPsiTree==NULL || 
     recoMumuMidTree==NULL || genMumuMidTree==NULL){
    cout<<"At least one file is missing. Bye!"<<endl;
    return;
  }
  // read the config file
  const char* configFile = "config.cfg";
  TEnv *env = new TEnv(configFile);
  
  // check if the config is there
  if(!filesystem::exists(configFile)){
    cout<<"Config file missing. Bye!"<<endl;
    return;
  }

  // construct the keys using the input configuration name.
  TString keyPtLow   = TString::Format("%s.minPt", config);
  TString keyPtUp    = TString::Format("%s.maxPt", config);
  TString keyMassLow = TString::Format("%s.minMass", config);
  TString keyMassUp  = TString::Format("%s.maxMass", config);
  TString keyRapLow  = TString::Format("%s.minRapidity", config);
  TString keyRaPUp   = TString::Format("%s.maxRapidity", config);

  // retireve the values: set the values to the ones in the config
  minPt       = env->GetValue(keyPtLow.Data(), 0.0);
  maxPt       = env->GetValue(keyPtUp.Data(), 0.0);
  minMass     = env->GetValue(keyMassLow.Data(), 0.0);
  maxMass     = env->GetValue(keyMassUp.Data(), 0.0);
  minRapidity = env->GetValue(keyRapLow.Data(), 0.0);
  maxRapidity = env->GetValue(keyRaPUp.Data(), 0.0);

  // useful to carry info for the bin (not used now!)
  float binID[3];

  // save the results in a tree
  string fileName = to_string(nBins) + "phi_1pt";
  gSaveFileName = "massFitRes_" + fileName + ".root";
  gSystem->mkdir(gIdentifier.c_str());
  gSystem->mkdir(Form("%s/%s",gIdentifier.c_str(),gNeutronClass.c_str()));
  TFile *saveFile = new TFile(Form("%s/%s/%s",gIdentifier.c_str(),gNeutronClass.c_str(),gSaveFileName.c_str()),"recreate");
	saveFile->Close();

  // loop on a specific bins (for later)
	for(int i=1; i<=nBins; i++){

		binID[0] = i;
    binID[1] = 0;
    binID[2] = 0;

    gSaveTreeName = "tree_phi" + to_string(i) + "_pt1";

		// do the fit
  	doOneDataFit(dataTree, saveFile, binID, recoCohJPsiTree, genCohJPsiTree, recoMumuMidTree, genMumuMidTree);

	} // end of the loop on the bins

} // end of the main