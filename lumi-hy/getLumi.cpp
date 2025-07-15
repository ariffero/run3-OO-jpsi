//
// Created by Roman Lavička on 03.03.2024.
// Updated by Andrea Giovanni Riffero
//



void getLumi(string filename = "AnalysedLumi.root"){

	TFile *f = TFile::Open(Form("%s",filename.c_str()), "read");
	Double_t systunc = 0.039; // https://alice-notes.web.cern.ch/node/1515

	string dir_new = "lumi-task";
	string dir_old = "eventselection-run3/luminosity";
	string dir = "";
	if (f){
		TH1F *hLumiTCE  = (TH1F*) f->Get(Form("%s/hLumiTCE",dir_old.c_str()));
		if (hLumiTCE != NULL) dir = dir_old;
		else dir = dir_new;
	}
	if (f){
		TH1F *hLumiTVX  = (TH1F*) f->Get(Form("%s/hLumiTVX",dir.c_str()));
		TH1F *hLumiTCE  = (TH1F*) f->Get(Form("%s/hLumiTCE",dir.c_str()));
		TH1F *hLumiZEM  = (TH1F*) f->Get(Form("%s/hLumiZEM",dir.c_str()));
		TH1F *hLumiZNC  = (TH1F*) f->Get(Form("%s/hLumiZNC",dir.c_str()));
		TH1F *hLumiTVXafterBCcuts  = (TH1F*) f->Get(Form("%s/hLumiTVXafterBCcuts",dir.c_str()));
		TH1F *hLumiTCEafterBCcuts  = (TH1F*) f->Get(Form("%s/hLumiTCEafterBCcuts",dir.c_str()));
		TH1F *hLumiZEMafterBCcuts  = (TH1F*) f->Get(Form("%s/hLumiZEMafterBCcuts",dir.c_str()));
		TH1F *hLumiZNCafterBCcuts  = (TH1F*) f->Get(Form("%s/hLumiZNCafterBCcuts",dir.c_str()));
		TH1F *hTriggerTCEupcCand  = (TH1F*) f->Get("upc-cand-producer/hCountersTrg");
		TH1F *hCounterTVX  = (TH1F*) f->Get(Form("%s/hCounterTVX",dir.c_str()));
		TH1F *hCounterTCE  = (TH1F*) f->Get(Form("%s/hCounterTCE",dir.c_str()));
		TH1F *hCounterZEM  = (TH1F*) f->Get(Form("%s/hCounterZEM",dir.c_str()));
		TH1F *hCounterZNC  = (TH1F*) f->Get(Form("%s/hCounterZNC",dir.c_str()));
		TH1F *hCounterTVXafterBCcuts  = (TH1F*) f->Get(Form("%s/hCounterTVXafterBCcuts",dir.c_str()));
		TH1F *hCounterTCEafterBCcuts  = (TH1F*) f->Get(Form("%s/hCounterTCEafterBCcuts",dir.c_str()));
		TH1F *hCounterZEMafterBCcuts  = (TH1F*) f->Get(Form("%s/hCounterZEMafterBCcuts",dir.c_str()));
		TH1F *hCounterZNCafterBCcuts  = (TH1F*) f->Get(Form("%s/hCounterZNCafterBCcuts",dir.c_str()));
		if (hLumiTVX){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiTVX->GetNbinsX();ibin++){
				lumi += hLumiTVX->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by TVX is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no TVX lumi info");
		if (hLumiTCE){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiTCE->GetNbinsX();ibin++){
				lumi += hLumiTCE->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by TCE is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no TCE lumi info");
		if (hLumiZEM){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiZEM->GetNbinsX();ibin++){
				lumi += hLumiZEM->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by ZEM is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no ZNC lumi info");
		if (hLumiZNC){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiZNC->GetNbinsX();ibin++){
				lumi += hLumiZNC->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by ZNC is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no ZNC lumi info");
		if (hLumiTVXafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiTVXafterBCcuts->GetNbinsX();ibin++){
				lumi += hLumiTVXafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by TVX afterBCcuts is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no TVX afterBCcuts lumi info");
		if (hLumiTCEafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiTCEafterBCcuts->GetNbinsX();ibin++){
				lumi += hLumiTCEafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by TCE afterBCcuts is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no TCE afterBCcuts lumi info");
		if (hLumiZEMafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiZEMafterBCcuts->GetNbinsX();ibin++){
				lumi += hLumiZEMafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by ZEM afterBCcuts is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no ZNC afterBCcuts lumi info");
		if (hLumiZNCafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hLumiZNCafterBCcuts->GetNbinsX();ibin++){
				lumi += hLumiZNCafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total luminosity as seen by ZNC afterBCcuts is: %.2f±%.2f /mub", lumi, systunc*lumi);
		}
		else Printf("no ZNC afterBCcuts lumi info");
		if (hTriggerTCEupcCand){
			Double_t triggers = hTriggerTCEupcCand->GetBinContent(1);
			Double_t lumi = triggers / 4100000; // divide by cross section in mub
			Printf("Total TCE triggers from UPCCandProducer: %.0f; Luminosity is: %.2f±%.2f /mub", triggers, lumi, systunc*lumi);
		}
		else Printf("no UPCCandProducer output");
		if (hCounterTVX){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterTVX->GetNbinsX();ibin++){
				lumi += hCounterTVX->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by TVX is: %.2f", lumi);
		}
		else Printf("no TVX triggers info");
		if (hCounterTCE){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterTCE->GetNbinsX();ibin++){
				lumi += hCounterTCE->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by TCE is: %.2f", lumi);
		}
		else Printf("no TCE triggers info");
		if (hCounterZEM){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterZEM->GetNbinsX();ibin++){
				lumi += hCounterZEM->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by ZEM is: %.2f", lumi);
		}
		else Printf("no ZNC triggers info");
		if (hCounterZNC){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterZNC->GetNbinsX();ibin++){
				lumi += hCounterZNC->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by ZNC is: %.2f", lumi);
		}
		else Printf("no ZNC triggers info");
		if (hCounterTVXafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterTVXafterBCcuts->GetNbinsX();ibin++){
				lumi += hCounterTVXafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by TVX afterBCcuts is: %.2f", lumi);
		}
		else Printf("no TVX afterBCcuts triggers info");
		if (hCounterTCEafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterTCEafterBCcuts->GetNbinsX();ibin++){
				lumi += hCounterTCEafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by TCE afterBCcuts is: %.2f", lumi);
		}
		else Printf("no TCE afterBCcuts triggers info");
		if (hCounterZEMafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterZEMafterBCcuts->GetNbinsX();ibin++){
				lumi += hCounterZEMafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by ZEM afterBCcuts is: %.2f", lumi);
		}
		else Printf("no ZNC afterBCcuts triggers info");
		if (hCounterZNCafterBCcuts){
			Double_t lumi = 0.0;
			for (int ibin(1);ibin <= hCounterZNCafterBCcuts->GetNbinsX();ibin++){
				lumi += hCounterZNCafterBCcuts->GetBinContent(ibin);
			}
			Printf("Total triggers as seen by ZNC afterBCcuts is: %.2f", lumi);
		}
		else Printf("no ZNC afterBCcuts triggers info");
	}
	else Printf("no lumi file found");
}
