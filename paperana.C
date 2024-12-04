TGraphErrors *TGData_centre;
TGraphErrors *TGData_right;
TGraphErrors *TGData_left;
TGraph *TGLEDPulse;
TGraph *TGVUVZ = new TGraph();
TGraph *TGVUVTrackL = new TGraph();

TGraph *TGVUVAL1 = new TGraph();
TGraph *TGVUVAL2 = new TGraph();

TFile *fout;

TH1D *VUVl[9];
void FillDataGraphs(float coordinate_offset_z);
const int nfiles = 9;
string filenames[9] = {"wls_0_20.root", "wls_0_m7p5.root", "wls_0_m45.root", "wls_50_20.root", "wls_50_m7p5.root", "wls_50_m45.root", "wls_m50_20.root", "wls_m50_m7p5.root", "wls_m50_m45.root"};

int paperana() {
	gStyle->SetOptStat(0);

	//float source_geo_frac = 0.35; 
	float source_geo_frac = 1.; // now included in simulation
	float alphaEnergy = 4800; // keV
	float LAr_alphaQ = 0.71; // alpha quenching
	float LAr_py = 40; // photons per keV
	float dLAr_py = 0.1; // fractional uncertainty, about 1sigma
	float PMTEfficiency = 0.17;
	float dPMTEfficiency = 0.1;

	float PENWLSE = 0.55;
	float dPENWLSE = 0.2; // 20%, includes unc on PENWLSE and on the relative yield compared to TPB

	float PE_alpha_peak = alphaEnergy * LAr_alphaQ * source_geo_frac * LAr_py * PMTEfficiency * PENWLSE;
	float rel_unc2 = dLAr_py*dLAr_py + dPMTEfficiency*dPMTEfficiency + dPENWLSE*dPENWLSE;
	float rel_unc = sqrt(rel_unc2);

	cout << "Relative uncertainty is " << rel_unc << endl;
	
	// the manipulator scale is vertically offset to the centre of the cage by this value
	// the source was ~5cm above the bottom on the lowest setting (z=75), so ~80cm above bottom on the highest (z=0)
	// relative to the center the offset is then 
	float coordinate_offset_z = 30.; // cm
	TGData_centre = new TGraphErrors();
	TGData_right = new TGraphErrors();
	TGData_left = new TGraphErrors();
	FillDataGraphs(coordinate_offset_z);
	

	TFile *fin;
	TH1D *fh;
	TH1D *htrack;
	TTree *fnt;
	double mean = 0;
	double sig = 0;
	double sourcex, sourcey, sourcez, LArVUVAbsLength;
	Double_t xpoints[nfiles];
	Double_t y_PE[nfiles],y_PEsys[nfiles], y_eta[nfiles] ;
	Double_t ex_PE[nfiles], ex_PEsys[nfiles], ex_eta[nfiles] ;
	Double_t ey_PE[nfiles], ey_PEsys[nfiles], ey_eta[nfiles];
	Double_t  yVUVtop[nfiles];
	TF1 *fg = new TF1("fg","gaus",0,1);
	float pointoffset = 0;
	float fracVUVtop = 0;
	//TString dir("../dwarf_data/NoAbsNoScatter/");
	TString dir("./");
	for (int ifiles = 0; ifiles < nfiles; ifiles++) {
		fin = new TFile(dir + filenames[ifiles].c_str());
		if (!fin) { cout << "Error reading file " << filenames[ifiles].c_str() << endl;} 
		//cout << filenames[ifiles].c_str() << endl;
		fh = (TH1D*)fin->Get("fraction"); // this is the fraction of VUV photons that are detected, based on MC		
		fnt = (TTree*)fin->Get("info"); // this tree has the settings with which the sim was run
		fnt->SetBranchAddress("sourcex", &sourcex);
		fnt->SetBranchAddress("sourcey", &sourcey);
		fnt->SetBranchAddress("sourcez", &sourcez);	
		fnt->SetBranchAddress("LArVUVAbsLength", &LArVUVAbsLength);
		fnt->GetEvent(0);	
		fh->Fit("fg","Q"); // the fitting was superflous, just taking the histogram mean works fine.
		mean = fg->GetParameter(1);
		sig = fg->GetParameter(2);
		fh->Fit("fg","Q","",mean-3.*sig, mean+3.*sig);
		pointoffset = 0; // this is to shift the points a bit so they don't all draw on top of each other
		if (sourcex > 0.1) pointoffset = 1.5;
		else if (sourcex < -0.1) pointoffset = -1.5;		
		xpoints[ifiles]=sourcez + pointoffset; 
		
		y_eta[ifiles]=fh->GetMean(); ex_eta[ifiles]=0.0; ey_eta[ifiles]=0.0;  // this is the 'detection fraction'
		y_PE[ifiles]= y_eta[ifiles] * PE_alpha_peak; ex_PE[ifiles] = 0.; ey_PE[ifiles] = ey_eta[ifiles]* PE_alpha_peak; // this translates the 'detection fraction' into a light yield
		ex_PEsys[ifiles] = 0.; ey_PEsys[ifiles] = rel_unc*y_PE[ifiles];

		//cout << filenames[ifiles] << " (" << sourcex << ", " <<sourcey << ", " << sourcez <<  "):\t" << fg->GetParameter(1) << ",\t" << fh->GetMean() << endl;
		cout << filenames[ifiles] << " (" << sourcex << ", " <<sourcey << ", " << sourcez <<  "):\t" << fg->GetParameter(1) << ",\t" << fh->GetMean() << endl;
		}
	auto tg_eta = new TGraphErrors(nfiles,xpoints,y_eta,ex_eta,ey_eta);	
	auto tg_PE = new TGraphErrors(nfiles,xpoints,y_PE,ex_PE,ey_PE);	
	auto tg_PEsys = new TGraphErrors(nfiles,xpoints,y_PE,ex_PEsys,ey_PEsys);	
	
	TCanvas *tc = new TCanvas("canvas1","canvas1");
	TH1F *hdummy = new TH1F("hdummy",";Source z position [cm]; Alpha peak [PE]",10, -50, 50);
	hdummy->GetYaxis()->SetRangeUser(20, 150);
	hdummy->Draw();
	tg_PE->SetMarkerStyle(2); 
	tg_PEsys->SetMarkerStyle(3); tg_PEsys->SetLineWidth(2);
	//tg_PE->Draw("sameP");
	tg_PEsys->Draw("sameP");
	TGData_centre->SetLineColor(kMagenta); TGData_centre->SetMarkerColor(kMagenta); TGData_centre->SetMarkerStyle(4); TGData_centre->Draw("sameP");
	TGData_right->SetLineColor(kMagenta-3); TGData_right->SetMarkerColor(kMagenta-3); TGData_right->SetMarkerStyle(4); TGData_right->Draw("sameP");
	TGData_left->SetLineColor(kMagenta+3); TGData_left->SetMarkerColor(kMagenta+3); TGData_left->SetMarkerStyle(4); TGData_left->Draw("sameP");	
	TLegend *leg = new TLegend();
	leg->AddEntry(TGData_centre, "Data, Centre", "pl");
	leg->AddEntry(TGData_right, "Data, Left", "pl");
	leg->AddEntry(TGData_left, "Data, Right", "pl");
	leg->AddEntry(tg_PEsys, "MC", "pl");
	
	leg->Draw();		
	
	TCanvas *tc2 = new TCanvas("canvas2","canvas2");
	tg_eta->SetTitle("");
	tg_eta->GetXaxis()->SetTitle("Source y position [cm]");
	tg_eta->GetYaxis()->SetTitle("fraction of photons detected");
	tg_eta->SetMarkerStyle(2);
	tg_eta->SetMarkerColor(2);
	tg_eta->Draw("AP");
	
	fout->Open("dwarf_mc.root","recreate");
	tg_PEsys->Write("ly_mc");	
	tg_eta->Write("detectionfraction_mc");
	tc->Write();
	tc2->Write();
	
	return 0;
}

// This puts the data points from Vikas into a graph with the correct z-axis
void FillDataGraphs(float coordinate_offset_z) {

/* from Vikas
top, middle, bottom:
no angle
[58.96058702151919,54.48267062608748,52.036476733120246]
[0.3738369404534642, 0.3738369404534642, 0.2844411503450271]
right
[91.85177240928907, 84.20041892007117, 85.29476973161962]
[1.3991515580425775, 1.286977998781006, 2.304551457528968]
left
[86.43619252225723, 84.8129779540253, 81.12907854302608]
[1.744909577533989, 2.4224800909430444, 1.5187862005194506]
*/
// center, top
TGData_centre->SetPoint(0,coordinate_offset_z - 10.00, 58.96);
TGData_centre->SetPointError(0, 0, 0.3);
cout << "0. " << coordinate_offset_z - 0.00 << " 0. " << endl;

// center, middle
TGData_centre->SetPoint(1, coordinate_offset_z - 37.50, 54.4); 
TGData_centre->SetPointError(2, 0, 0.11);
cout << "0. " << coordinate_offset_z - 37.50 << " 0. " << endl;

// center, bottom
TGData_centre->SetPoint(2, coordinate_offset_z - 75.00, 52.0); 
TGData_centre->SetPointError(3, 0, 0.09);
cout << "0. " << coordinate_offset_z - 75.00 << " 0. " << endl;

// right, top
TGData_right->SetPoint(0, coordinate_offset_z - 10.0 + 1.5, 91.85); 
TGData_right->SetPointError(0, 0, 1.50);
cout << "50. " << coordinate_offset_z - 10.0 << " 0. " << endl;

// right, middle
TGData_right->SetPoint(1, coordinate_offset_z - 37.50+ 1.5, 84.2); 
TGData_right->SetPointError(1, 0, 1.78);
cout << "50. " << coordinate_offset_z - 37.50 << " 0. " << endl;
// right, bottom
TGData_right->SetPoint(2, coordinate_offset_z - 75.00+ 1.5, 85.2); 
TGData_right->SetPointError(2, 0, 1.7);
cout << "50. " << coordinate_offset_z - 75.00 << " 0. " << endl;

TGData_left->SetPoint(0, coordinate_offset_z - 10.0 - 1.5, 86.43); 
TGData_left->SetPointError(0, 0, 1.74);
cout << "-50. " << coordinate_offset_z - 10.0 << " 0. " << endl;

TGData_left->SetPoint(1, coordinate_offset_z - 37.50 - 1.5, 84.81); 
TGData_left->SetPointError(1, 0, 1.92);
cout << "-50. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_left->SetPoint(2, coordinate_offset_z - 75.00 - 1.5, 81.13); 
TGData_left->SetPointError(2, 0, 1.57);
cout << "-50. " << coordinate_offset_z - 75.00 << " 0. " << endl;


}
