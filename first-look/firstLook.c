void firstLook() {
    // Open the ROOT file
    TFile *f = TFile::Open("../data/train-442464/data-OO-AnalysisResults.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: could not open file" << std::endl;
        return;
    }

    // Retrieve the histogram
    TH1D *h = dynamic_cast<TH1D*>(f->Get("fwd-muons-u-p-c/registry/hMass"));
    if (!h) {
        std::cerr << "Error: histogram not found" << std::endl;
        return;
    }

    // Compute integrals
    double total = h->Integral();
    int binLow  = h->GetXaxis()->FindBin(2.9);
    int binHigh = h->GetXaxis()->FindBin(3.3);
    double inRange = h->Integral(binLow, binHigh);

    // Print results
    std::cout << "Total = " << total << std::endl;
    std::cout << "Total in [2.9, 3.3] = " << inRange << std::endl;
    std::cout << "Expected (scaled to 5 /nb) = " << (inRange / 0.8) * 5 << std::endl;

    // Customize plot titles
    h->SetTitle("J/#psi Mass Spectrum");
    h->GetXaxis()->SetTitle("#mu#mu inv. mass [GeV/#it{c}^{2}]");
    h->GetYaxis()->SetTitle("Counts per Bin");
    h->GetXaxis()->SetRangeUser(2,4.5);

    // Draw and save
    TCanvas *c = new TCanvas("c_mass", "Mass Plot", 800, 600);
    h->SetLineColor(kBlue+2);
    h->SetLineWidth(2);
    h->SetFillColor(kBlue+2);
    h->Draw();
    c->SaveAs("mass_spectrum.png");
}
