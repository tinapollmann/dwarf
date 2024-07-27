

{
gStyle->SetOptStat(0);

float source_geo_frac = 0.35; 
float alphaEnergy = 4800; // keV
float LAr_alphaQ = 0.71; // alpha quenching
float LAr_py = 40; // photons per keV
float dLAr_py = 0.1; // fractional uncertainty, about 1sigma
float PMTEfficiency = 0.16;
float dPMTEfficiency = 0.01;

float PENWLSE = 0.6;
float dPENWLSE = 0.2; // 20%, includes unc on PENWLSE and on the relative yield compared to TPB

float PE_alpha_peak = alphaEnergy * LAr_alphaQ * source_geo_frac * LAr_py * PMTEfficiency * PENWLSE;
float rel_unc2 = dLAr_py*dLAr_py + dPMTEfficiency*dPMTEfficiency + dPENWLSE*dPENWLSE;
float rel_unc = sqrt(rel_unc2);

cout << "Relative uncertainty is " << rel_unc << endl;
const int nfiles = 9;
//string filenames[5] = {"wls_20_0.root", "wls_20_m20.root", "wls_20_20.root","wls_20_m40.root", "wls_20_40.root"};
string filenames[9] = {"wls_0_0.root", "wls_0_m20.root", "wls_0_20.root","wls_0_m40.root", "wls_0_40.root", "wls_0_40_am50.root", "wls_0_40_a50.root", "wls_0_m40_am50.root", "wls_0_m40_a50.root"};
//string filenames[nfiles] = {"wls_1.root", "wls_2.root", "wls_3.root", "wls_4.root"};
TFile *fin;
TH1D *fh;
TTree *fnt;
double mean = 0;
double sig = 0;
double sourcex, sourcey, sourcez, LArVUVAbsLength;
Double_t xpoints[nfiles];
Double_t y_PE[nfiles],y_PEsys[nfiles], y_eta[nfiles] ;
Double_t ex_PE[nfiles], ex_PEsys[nfiles], ex_eta[nfiles] ;
Double_t ey_PE[nfiles], ey_PEsys[nfiles], ey_eta[nfiles];

TF1 *fg = new TF1("fg","gaus",0,1);

for (int ifiles = 0; ifiles < nfiles; ifiles++) {
	fin = new TFile(filenames[ifiles].c_str());
	fh = (TH1D*)fin->Get("fraction");
	fnt = (TTree*)fin->Get("info");
	fnt->SetBranchAddress("sourcex", &sourcex);
	fnt->SetBranchAddress("sourcey", &sourcey);
	fnt->SetBranchAddress("sourcez", &sourcez);	
	fnt->SetBranchAddress("LArVUVAbsLength", &LArVUVAbsLength);
	fnt->GetEvent(0);	
	fh->Fit("fg");
	mean = fg->GetParameter(1);
	sig = fg->GetParameter(2);
	fh->Fit("fg","","",mean-3.*sig, mean+3.*sig);
	xpoints[ifiles]=sourcez; 
	y_eta[ifiles]=fg->GetParameter(1); ex_eta[ifiles]=fg->GetParError(1); ey_eta[ifiles]=fg->GetParError(1);
	y_PE[ifiles]= y_eta[ifiles] * PE_alpha_peak; ex_PE[ifiles] = 0.; ey_PE[ifiles] = ey_eta[ifiles]* PE_alpha_peak;
	ex_PEsys[ifiles] = 0.; ey_PEsys[ifiles] = rel_unc*y_PE[ifiles];

	cout << filenames[ifiles] << " : " << fg->GetParameter(1) << " , " << fh->GetMean() << endl;
	}
auto tg_eta = new TGraphErrors(nfiles,xpoints,y_eta,ex_eta,ey_eta);	
auto tg_PE = new TGraphErrors(nfiles,xpoints,y_PE,ex_PE,ey_PE);	
auto tg_PEsys = new TGraphErrors(nfiles,xpoints,y_PE,ex_PEsys,ey_PEsys);	

TH1F *hdummy = new TH1F("hdummy",";Source z position [cm]; Alpha peak [PE]",10, -50, 50);
hdummy->GetYaxis()->SetRangeUser(20, 150);
hdummy->Draw();
tg_PE->SetMarkerStyle(2); 
tg_PEsys->SetMarkerStyle(3); tg_PEsys->SetLineWidth(3);
tg_PE->Draw("sameP");
tg_PEsys->Draw("same []");

new TCanvas();
tg_eta->SetTitle("");
tg_eta->GetXaxis()->SetTitle("Source y position [cm]");
tg_eta->GetYaxis()->SetTitle("fraction of photons detected");
tg_eta->SetMarkerStyle(2);
tg_eta->SetMarkerColor(2);
tg_eta->Draw("AP");

}