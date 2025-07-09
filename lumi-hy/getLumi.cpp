//
// Created by Roman Lavička on 03.03.2024.
//



void getLumi(string filename = "AnalysedLumi.root"){

	TFile *f = TFile::Open(Form("%s",filename.c_str()), "read");
	Double_t systunc = 0.039; // https://alice-notes.web.cern.ch/node/1515

	if (f){
		TH1F *hLumiTVX  = (TH1F*) f->Get("bc-selection-task/hLumiTVX");
		TH1F *hLumiTCE  = (TH1F*) f->Get("bc-selection-task/hLumiTCE");
		TH1F *hLumiZEM  = (TH1F*) f->Get("bc-selection-task/hLumiZEM");
		TH1F *hLumiZNC  = (TH1F*) f->Get("bc-selection-task/hLumiZNC");
		TH1F *hLumiTVXafterBCcuts  = (TH1F*) f->Get("bc-selection-task/hLumiTVXafterBCcuts");
		TH1F *hLumiTCEafterBCcuts  = (TH1F*) f->Get("bc-selection-task/hLumiTCEafterBCcuts");
		TH1F *hLumiZEMafterBCcuts  = (TH1F*) f->Get("bc-selection-task/hLumiZEMafterBCcuts");
		TH1F *hLumiZNCafterBCcuts  = (TH1F*) f->Get("bc-selection-task/hLumiZNCafterBCcuts");
		TH1F *hTriggerTCEupcCand  = (TH1F*) f->Get("upc-cand-producer/hCountersTrg");
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
	}
	else Printf("no lumi file found");
}
