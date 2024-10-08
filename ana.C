
TGraphErrors *TGData_centre;
TGraphErrors *TGData_right;
TGraphErrors *TGData_left;
TGraph *TGLEDPulse;
TGraph *TGVUVZ = new TGraph();
TGraph *TGVUVTrackL = new TGraph();

TGraph *TGVUVAL1 = new TGraph();
TGraph *TGVUVAL2 = new TGraph();

TH1D *VUVl[9];
void FillDataGraphs(float coordinate_offset_z);
float ApplyAbsorptionLength(float absl, TH1D *hlengths);
void FillLEDPulseshape();
void DrawVUVl();
const int nfiles = 9;
string filenames[9] = {"wls_0_30.root", "wls_0_m7p5.root", "wls_0_m45.root", "wls_50_20.root", "wls_50_m7p5.root", "wls_50_m45.root", "wls_m50_20.root", "wls_m50_m7p5.root", "wls_m50_m45.root"};

int ana() {
	gStyle->SetOptStat(0);

	//float source_geo_frac = 0.35; 
	float source_geo_frac = 1.; // now included in simulation
	float alphaEnergy = 4800; // keV
	float LAr_alphaQ = 0.71; // alpha quenching
	float LAr_py = 40; // photons per keV
	float dLAr_py = 0.1; // fractional uncertainty, about 1sigma
	float PMTEfficiency = 0.16;
	float dPMTEfficiency = 0.1;

	float PENWLSE = 0.8;
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
		cout << filenames[ifiles].c_str() << endl;
		fh = (TH1D*)fin->Get("fraction");
		VUVl[ifiles] = (TH1D*)fin->Get("VUVZ");
		htrack =  (TH1D*)fin->Get("vuvlength");
		
		fnt = (TTree*)fin->Get("info");
		fnt->SetBranchAddress("sourcex", &sourcex);
		fnt->SetBranchAddress("sourcey", &sourcey);
		fnt->SetBranchAddress("sourcez", &sourcez);	
		fnt->SetBranchAddress("LArVUVAbsLength", &LArVUVAbsLength);
		fnt->GetEvent(0);	
		/*fh->Fit("fg","Q");
		mean = fg->GetParameter(1);
		sig = fg->GetParameter(2);
		fh->Fit("fg","Q","",mean-3.*sig, mean+3.*sig);*/
		pointoffset = 0;
		if (sourcex > 0.1) pointoffset = 1.5;
		else if (sourcex < -0.1) pointoffset = -1.5;		
		xpoints[ifiles]=sourcez + pointoffset; 
		cout << "1" << endl;
		fracVUVtop = VUVl[ifiles]->Integral(VUVl[ifiles]->GetXaxis()->FindBin(45), VUVl[ifiles]->GetXaxis()->FindBin(55))/VUVl[ifiles]->Integral(0,200);
		TGVUVZ->SetPoint(ifiles, xpoints[ifiles], fracVUVtop);
		TGVUVAL1->SetPoint(ifiles, xpoints[ifiles], ApplyAbsorptionLength(50., htrack));
		TGVUVAL2->SetPoint(ifiles, xpoints[ifiles], ApplyAbsorptionLength(10., htrack));
		
		cout << "2" << endl;
		htrack->GetXaxis()->SetRangeUser(5,140);
		TGVUVTrackL->SetPoint(ifiles, xpoints[ifiles], htrack->GetMean());
		// Fold the vuv track length with the 
		//y_eta[ifiles]=fg->GetParameter(1); ex_eta[ifiles]=fg->GetParError(1); ey_eta[ifiles]=fg->GetParError(1);
		y_eta[ifiles]=fh->GetMean(); ex_eta[ifiles]=0.0; ey_eta[ifiles]=0.0;
		y_PE[ifiles]= y_eta[ifiles] * PE_alpha_peak; ex_PE[ifiles] = 0.; ey_PE[ifiles] = ey_eta[ifiles]* PE_alpha_peak;
		ex_PEsys[ifiles] = 0.; ey_PEsys[ifiles] = rel_unc*y_PE[ifiles];

		cout << filenames[ifiles] << " (" << sourcex << ", " <<sourcey << ", " << sourcez <<  "):\t" << fg->GetParameter(1) << ",\t" << fh->GetMean() << endl;
		}
	auto tg_eta = new TGraphErrors(nfiles,xpoints,y_eta,ex_eta,ey_eta);	
	auto tg_PE = new TGraphErrors(nfiles,xpoints,y_PE,ex_PE,ey_PE);	
	auto tg_PEsys = new TGraphErrors(nfiles,xpoints,y_PE,ex_PEsys,ey_PEsys);	
	
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
	/*
	new TCanvas();
	tg_eta->SetTitle("");
	tg_eta->GetXaxis()->SetTitle("Source y position [cm]");
	tg_eta->GetYaxis()->SetTitle("fraction of photons detected");
	tg_eta->SetMarkerStyle(2);
	tg_eta->SetMarkerColor(2);
	tg_eta->Draw("AP");
	
	new TCanvas();
	TGVUVAL1->SetMarkerStyle(5); TGVUVAL1->Draw("ap");
	TGVUVAL2->SetMarkerStyle(6); TGVUVAL1->Draw("samep");	
	*/
	return 0;
}

void FillDataGraphs(float coordinate_offset_z) {

/* from Vikas
Z  Rate, Rate Error, PE, PE Error
angle : 0
0.00 14.00 0.67 67.62 0.08
7.50 13.61 0.71 57.45 0.06
37.50 11.96 1.73 42.06 0.11
75.00 11.62 2.41 40.87 0.09
angle : right
10.00 15.28 3.91 108.99 1.50
37.50 15.48 3.93 100.44 1.78
75.00 14.09 3.75 100.51 1.70
angle: left
10.00 14.57 3.82 102.61 1.74
37.50 14.96 3.87 98.51 1.92
75.00 15.02 3.88 95.50 1.57

Update:
so the correct z=0 alpha peak is 58.60 0.30
and if you remove the old z=0 and z=7.5, everything would be consistent

*/

TGData_centre->SetPoint(0,coordinate_offset_z - 0.00, 56.7);
TGData_centre->SetPointError(0, 0, 0.3);
cout << "0. " << coordinate_offset_z - 0.00 << " 0. " << endl;

//TGData_centre->SetPoint(1, coordinate_offset_z - 7.50, 57.45);
//TGData_centre->SetPointError(1, 0, 0.08);
//cout << "0. " << coordinate_offset_z - 7.50 << " 0. " << endl;

TGData_centre->SetPoint(1, coordinate_offset_z - 37.50, 42.06); 
TGData_centre->SetPointError(2, 0, 0.11);
cout << "0. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_centre->SetPoint(2, coordinate_offset_z - 75.00, 40.87); 
TGData_centre->SetPointError(3, 0, 0.09);
cout << "0. " << coordinate_offset_z - 75.00 << " 0. " << endl;

/*


TGData_right->SetPoint(0, coordinate_offset_z - 10.0 + 1.5, 108.99); 
TGData_right->SetPointError(0, 0, 1.50);
cout << "50. " << coordinate_offset_z - 10.0 << " 0. " << endl;


TGData_right->SetPoint(1, coordinate_offset_z - 37.50+ 1.5, 100.44); 
TGData_right->SetPointError(1, 0, 1.78);
cout << "50. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_right->SetPoint(2, coordinate_offset_z - 75.00+ 1.5, 100.51); 
TGData_right->SetPointError(2, 0, 1.7);
cout << "50. " << coordinate_offset_z - 75.00 << " 0. " << endl;

TGData_left->SetPoint(0, coordinate_offset_z - 10.0 - 1.5, 102.61); 
TGData_left->SetPointError(0, 0, 1.74);
cout << "-50. " << coordinate_offset_z - 10.0 << " 0. " << endl;

TGData_left->SetPoint(1, coordinate_offset_z - 37.50 - 1.5, 98.51); 
TGData_left->SetPointError(1, 0, 1.92);
cout << "-50. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_left->SetPoint(2, coordinate_offset_z - 75.00 - 1.5, 95.50); 
TGData_left->SetPointError(2, 0, 1.57);
cout << "-50. " << coordinate_offset_z - 75.00 << " 0. " << endl;
*/


TGData_right->SetPoint(0, coordinate_offset_z - 10.0 + 1.5, 108.99); 
TGData_right->SetPointError(0, 0, 1.50);
cout << "50. " << coordinate_offset_z - 10.0 << " 0. " << endl;


TGData_right->SetPoint(1, coordinate_offset_z - 37.50+ 1.5, 100.44); 
TGData_right->SetPointError(1, 0, 1.78);
cout << "50. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_right->SetPoint(2, coordinate_offset_z - 75.00+ 1.5, 100.51); 
TGData_right->SetPointError(2, 0, 1.7);
cout << "50. " << coordinate_offset_z - 75.00 << " 0. " << endl;

TGData_left->SetPoint(0, coordinate_offset_z - 10.0 - 1.5, 102.61); 
TGData_left->SetPointError(0, 0, 1.74);
cout << "-50. " << coordinate_offset_z - 10.0 << " 0. " << endl;

TGData_left->SetPoint(1, coordinate_offset_z - 37.50 - 1.5, 98.51); 
TGData_left->SetPointError(1, 0, 1.92);
cout << "-50. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_left->SetPoint(2, coordinate_offset_z - 75.00 - 1.5, 95.50); 
TGData_left->SetPointError(2, 0, 1.57);
cout << "-50. " << coordinate_offset_z - 75.00 << " 0. " << endl;


}

void FillLEDPulseshape() {
	
	int nledpoints = 1000;
	// Feb 7 and 8 data LED16 and LED18
	Double_t x_LED[1000] = { 0.00, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27, 0.28, 0.29, 0.30, 0.31, 0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40, 0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.50, 0.51, 0.52, 0.53, 0.54, 0.55, 0.56, 0.57, 0.58, 0.59, 0.60, 0.61, 0.62, 0.63, 0.64, 0.65, 0.66, 0.67, 0.68, 0.69, 0.70, 0.71, 0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.79, 0.80, 0.81, 0.82, 0.83, 0.84, 0.85, 0.86, 0.87, 0.88, 0.89, 0.90, 0.91, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99, 1.00, 1.01, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08, 1.09, 1.10, 1.11, 1.12, 1.13, 1.14, 1.15, 1.16, 1.17, 1.18, 1.19, 1.20, 1.21, 1.22, 1.23, 1.24, 1.25, 1.26, 1.27, 1.28, 1.29, 1.30, 1.31, 1.32, 1.33, 1.34, 1.35, 1.36, 1.37, 1.38, 1.39, 1.40, 1.41, 1.42, 1.43, 1.44, 1.45, 1.46, 1.47, 1.48, 1.49, 1.50, 1.51, 1.52, 1.53, 1.54, 1.55, 1.56, 1.57, 1.58, 1.59, 1.60, 1.61, 1.62, 1.63, 1.64, 1.65, 1.66, 1.67, 1.68, 1.69, 1.70, 1.71, 1.72, 1.73, 1.74, 1.75, 1.76, 1.77, 1.78, 1.79, 1.80, 1.81, 1.82, 1.83, 1.84, 1.85, 1.86, 1.87, 1.88, 1.89, 1.90, 1.91, 1.92, 1.93, 1.94, 1.95, 1.96, 1.97, 1.98, 1.99, 2.00, 2.01, 2.02, 2.03, 2.04, 2.05, 2.06, 2.07, 2.08, 2.09, 2.10, 2.11, 2.12, 2.13, 2.14, 2.15, 2.16, 2.17, 2.18, 2.19, 2.20, 2.21, 2.22, 2.23, 2.24, 2.25, 2.26, 2.27, 2.28, 2.29, 2.30, 2.31, 2.32, 2.33, 2.34, 2.35, 2.36, 2.37, 2.38, 2.39, 2.40, 2.41, 2.42, 2.43, 2.44, 2.45, 2.46, 2.47, 2.48, 2.49, 2.50, 2.51, 2.52, 2.53, 2.54, 2.55, 2.56, 2.57, 2.58, 2.59, 2.60, 2.61, 2.62, 2.63, 2.64, 2.65, 2.66, 2.67, 2.68, 2.69, 2.70, 2.71, 2.72, 2.73, 2.74, 2.75, 2.76, 2.77, 2.78, 2.79, 2.80, 2.81, 2.82, 2.83, 2.84, 2.85, 2.86, 2.87, 2.88, 2.89, 2.90, 2.91, 2.92, 2.93, 2.94, 2.95, 2.96, 2.97, 2.98, 2.99, 3.00, 3.01, 3.02, 3.03, 3.04, 3.05, 3.06, 3.07, 3.08, 3.09, 3.10, 3.11, 3.12, 3.13, 3.14, 3.15, 3.16, 3.17, 3.18, 3.19, 3.20, 3.21, 3.22, 3.23, 3.24, 3.25, 3.26, 3.27, 3.28, 3.29, 3.30, 3.31, 3.32, 3.33, 3.34, 3.35, 3.36, 3.37, 3.38, 3.39, 3.40, 3.41, 3.42, 3.43, 3.44, 3.45, 3.46, 3.47, 3.48, 3.49, 3.50, 3.51, 3.52, 3.53, 3.54, 3.55, 3.56, 3.57, 3.58, 3.59, 3.60, 3.61, 3.62, 3.63, 3.64, 3.65, 3.66, 3.67, 3.68, 3.69, 3.70, 3.71, 3.72, 3.73, 3.74, 3.75, 3.76, 3.77, 3.78, 3.79, 3.80, 3.81, 3.82, 3.83, 3.84, 3.85, 3.86, 3.87, 3.88, 3.89, 3.90, 3.91, 3.92, 3.93, 3.94, 3.95, 3.96, 3.97, 3.98, 3.99, 4.00, 4.01, 4.02, 4.03, 4.04, 4.05, 4.06, 4.07, 4.08, 4.09, 4.10, 4.11, 4.12, 4.13, 4.14, 4.15, 4.16, 4.17, 4.18, 4.19, 4.20, 4.21, 4.22, 4.23, 4.24, 4.25, 4.26, 4.27, 4.28, 4.29, 4.30, 4.31, 4.32, 4.33, 4.34, 4.35, 4.36, 4.37, 4.38, 4.39, 4.40, 4.41, 4.42, 4.43, 4.44, 4.45, 4.46, 4.47, 4.48, 4.49, 4.50, 4.51, 4.52, 4.53, 4.54, 4.55, 4.56, 4.57, 4.58, 4.59, 4.60, 4.61, 4.62, 4.63, 4.64, 4.65, 4.66, 4.67, 4.68, 4.69, 4.70, 4.71, 4.72, 4.73, 4.74, 4.75, 4.76, 4.77, 4.78, 4.79, 4.80, 4.81, 4.82, 4.83, 4.84, 4.85, 4.86, 4.87, 4.88, 4.89, 4.90, 4.91, 4.92, 4.93, 4.94, 4.95, 4.96, 4.97, 4.98, 4.99, 5.01, 5.02, 5.03, 5.04, 5.05, 5.06, 5.07, 5.08, 5.09, 5.10, 5.11, 5.12, 5.13, 5.14, 5.15, 5.16, 5.17, 5.18, 5.19, 5.20, 5.21, 5.22, 5.23, 5.24, 5.25, 5.26, 5.27, 5.28, 5.29, 5.30, 5.31, 5.32, 5.33, 5.34, 5.35, 5.36, 5.37, 5.38, 5.39, 5.40, 5.41, 5.42, 5.43, 5.44, 5.45, 5.46, 5.47, 5.48, 5.49, 5.50, 5.51, 5.52, 5.53, 5.54, 5.55, 5.56, 5.57, 5.58, 5.59, 5.60, 5.61, 5.62, 5.63, 5.64, 5.65, 5.66, 5.67, 5.68, 5.69, 5.70, 5.71, 5.72, 5.73, 5.74, 5.75, 5.76, 5.77, 5.78, 5.79, 5.80, 5.81, 5.82, 5.83, 5.84, 5.85, 5.86, 5.87, 5.88, 5.89, 5.90, 5.91, 5.92, 5.93, 5.94, 5.95, 5.96, 5.97, 5.98, 5.99, 6.00, 6.01, 6.02, 6.03, 6.04, 6.05, 6.06, 6.07, 6.08, 6.09, 6.10, 6.11, 6.12, 6.13, 6.14, 6.15, 6.16, 6.17, 6.18, 6.19, 6.20, 6.21, 6.22, 6.23, 6.24, 6.25, 6.26, 6.27, 6.28, 6.29, 6.30, 6.31, 6.32, 6.33, 6.34, 6.35, 6.36, 6.37, 6.38, 6.39, 6.40, 6.41, 6.42, 6.43, 6.44, 6.45, 6.46, 6.47, 6.48, 6.49, 6.50, 6.51, 6.52, 6.53, 6.54, 6.55, 6.56, 6.57, 6.58, 6.59, 6.60, 6.61, 6.62, 6.63, 6.64, 6.65, 6.66, 6.67, 6.68, 6.69, 6.70, 6.71, 6.72, 6.73, 6.74, 6.75, 6.76, 6.77, 6.78, 6.79, 6.80, 6.81, 6.82, 6.83, 6.84, 6.85, 6.86, 6.87, 6.88, 6.89, 6.90, 6.91, 6.92, 6.93, 6.94, 6.95, 6.96, 6.97, 6.98, 6.99, 7.00, 7.01, 7.02, 7.03, 7.04, 7.05, 7.06, 7.07, 7.08, 7.09, 7.10, 7.11, 7.12, 7.13, 7.14, 7.15, 7.16, 7.17, 7.18, 7.19, 7.20, 7.21, 7.22, 7.23, 7.24, 7.25, 7.26, 7.27, 7.28, 7.29, 7.30, 7.31, 7.32, 7.33, 7.34, 7.35, 7.36, 7.37, 7.38, 7.39, 7.40, 7.41, 7.42, 7.43, 7.44, 7.45, 7.46, 7.47, 7.48, 7.49, 7.50, 7.51, 7.52, 7.53, 7.54, 7.55, 7.56, 7.57, 7.58, 7.59, 7.60, 7.61, 7.62, 7.63, 7.64, 7.65, 7.66, 7.67, 7.68, 7.69, 7.70, 7.71, 7.72, 7.73, 7.74, 7.75, 7.76, 7.77, 7.78, 7.79, 7.80, 7.81, 7.82, 7.83, 7.84, 7.85, 7.86, 7.87, 7.88, 7.89, 7.90, 7.91, 7.92, 7.93, 7.94, 7.95, 7.96, 7.97, 7.98, 7.99, 8.00, 8.01, 8.02, 8.03, 8.04, 8.05, 8.06, 8.07, 8.08, 8.09, 8.10, 8.11, 8.12, 8.13, 8.14, 8.15, 8.16, 8.17, 8.18, 8.19, 8.20, 8.21, 8.22, 8.23, 8.24, 8.25, 8.26, 8.27, 8.28, 8.29, 8.30, 8.31, 8.32, 8.33, 8.34, 8.35, 8.36, 8.37, 8.38, 8.39, 8.40, 8.41, 8.42, 8.43, 8.44, 8.45, 8.46, 8.47, 8.48, 8.49, 8.50, 8.51, 8.52, 8.53, 8.54, 8.55, 8.56, 8.57, 8.58, 8.59, 8.60, 8.61, 8.62, 8.63, 8.64, 8.65, 8.66, 8.67, 8.68, 8.69, 8.70, 8.71, 8.72, 8.73, 8.74, 8.75, 8.76, 8.77, 8.78, 8.79, 8.80, 8.81, 8.82, 8.83, 8.84, 8.85, 8.86, 8.87, 8.88, 8.89, 8.90, 8.91, 8.92, 8.93, 8.94, 8.95, 8.96, 8.97, 8.98, 8.99, 9.00, 9.01, 9.02, 9.03, 9.04, 9.05, 9.06, 9.07, 9.08, 9.09, 9.10, 9.11, 9.12, 9.13, 9.14, 9.15, 9.16, 9.17, 9.18, 9.19, 9.20, 9.21, 9.22, 9.23, 9.24, 9.25, 9.26, 9.27, 9.28, 9.29, 9.30, 9.31, 9.32, 9.33, 9.34, 9.35, 9.36, 9.37, 9.38, 9.39, 9.40, 9.41, 9.42, 9.43, 9.44, 9.45, 9.46, 9.47, 9.48, 9.49, 9.50, 9.51, 9.52, 9.53, 9.54, 9.55, 9.56, 9.57, 9.58, 9.59, 9.60, 9.61, 9.62, 9.63, 9.64, 9.65, 9.66, 9.67, 9.68, 9.69, 9.70, 9.71, 9.72, 9.73, 9.74, 9.75, 9.76, 9.77, 9.78, 9.79, 9.80, 9.81, 9.82, 9.83, 9.84, 9.85, 9.86, 9.87, 9.88, 9.89, 9.90, 9.91, 9.92, 9.93, 9.94, 9.95, 9.96, 9.97, 9.98, 9.99, 10.00 };
	Double_t y_LED[1000] = { 0.38, 0.37, 0.40, 0.38, 0.36, 0.34, 0.32, 0.31, 0.32, 0.32, 0.29, 0.29, 0.31, 0.34, 0.40, 0.45, 0.43, 0.43, 0.44, 0.50, 0.52, 0.56, 0.56, 0.56, 0.54, 0.56, 0.53, 0.48, 0.45, 0.48, 0.49, 0.47, 0.42, 0.41, 0.45, 0.44, 0.41, 0.44, 0.40, 0.40, 0.43, 0.46, 0.53, 0.58, 0.63, 0.67, 0.68, 0.69, 0.70, 0.66, 0.63, 0.57, 0.53, 0.51, 0.48, 0.46, 0.45, 0.45, 0.44, 0.44, 0.43, 0.46, 0.42, 0.39, 0.39, 0.39, 0.39, 0.34, 0.31, 0.33, 0.31, 0.30, 0.33, 0.55, 1.25, 1.31, 1.10, 1.89, 2.40, 2.67, 2.42, 1.52, 1.84, 1.81, 0.28, -1.24, -1.86, -2.71, -4.13, -4.42, -4.33, -4.58, -4.75, -4.06, -1.93, -0.10, 0.50, 1.83, 3.18, 2.28, -0.06, -1.73, -2.01, -1.42, -0.49, 1.51, 3.90, 5.21, 6.83, 18.75, 53.10, 106.21, 167.72, 236.29, 310.34, 380.46, 434.56, 468.95, 486.03, 489.72, 482.73, 469.70, 454.55, 436.49, 416.64, 394.93, 373.46, 350.86, 326.33, 302.28, 279.80, 258.60, 238.78, 220.30, 203.95, 188.38, 172.53, 157.19, 143.05, 129.84, 118.62, 108.92, 100.47, 92.90, 85.41, 78.20, 71.38, 64.99, 58.80, 53.58, 49.72, 46.85, 43.77, 40.92, 38.39, 35.66, 32.37, 29.31, 27.27, 25.60, 24.23, 23.12, 22.40, 21.68, 20.19, 18.79, 17.28, 16.02, 14.94, 14.24, 14.08, 14.02, 13.61, 12.95, 12.32, 11.72, 11.12, 10.54, 10.29, 10.11, 9.73, 9.28, 8.99, 8.73, 8.36, 7.99, 7.91, 8.07, 8.04, 7.84, 7.62, 7.30, 7.00, 6.68, 6.57, 6.61, 6.74, 6.94, 7.09, 7.08, 6.90, 6.61, 6.45, 6.36, 6.36, 6.43, 6.46, 6.57, 6.61, 6.42, 6.23, 6.22, 6.07, 6.05, 6.27, 6.44, 6.44, 6.37, 6.13, 5.89, 5.74, 5.66, 5.58, 5.71, 5.81, 5.92, 5.86, 5.67, 5.34, 5.18, 5.28, 5.54, 5.74, 5.85, 5.89, 5.87, 5.75, 5.47, 5.22, 5.01, 5.03, 5.11, 5.15, 5.33, 5.39, 5.25, 5.19, 4.95, 4.79, 4.89, 5.06, 5.17, 5.24, 5.31, 5.34, 5.30, 5.15, 5.09, 5.12, 5.22, 5.21, 5.26, 5.20, 5.07, 4.93, 4.79, 4.73, 4.73, 4.91, 5.00, 5.02, 4.92, 4.85, 4.75, 4.61, 4.59, 4.50, 4.58, 4.62, 4.73, 4.80, 4.74, 4.62, 4.56, 4.52, 4.56, 4.60, 4.52, 4.46, 4.45, 4.34, 4.26, 4.22, 4.16, 4.07, 4.09, 4.12, 4.08, 4.06, 3.95, 3.91, 3.87, 3.89, 3.99, 4.06, 4.08, 4.00, 3.88, 3.79, 3.74, 3.72, 3.77, 3.79, 3.79, 3.72, 3.66, 3.51, 3.49, 3.47, 3.38, 3.37, 3.41, 3.41, 3.44, 3.35, 3.31, 3.21, 3.16, 3.16, 3.26, 3.38, 3.46, 3.40, 3.37, 3.30, 3.26, 3.21, 3.25, 3.31, 3.33, 3.30, 3.30, 3.23, 3.13, 3.09, 3.18, 3.22, 3.29, 3.35, 3.32, 3.30, 3.28, 3.21, 3.23, 3.20, 3.22, 3.20, 3.17, 3.18, 3.11, 3.02, 2.99, 2.97, 2.91, 2.92, 2.97, 2.92, 2.89, 2.85, 2.72, 2.65, 2.64, 2.67, 2.65, 2.69, 2.72, 2.72, 2.66, 2.59, 2.53, 2.51, 2.50, 2.59, 2.78, 2.77, 2.75, 2.66, 2.66, 2.69, 2.69, 2.69, 2.75, 2.76, 2.75, 2.84, 2.85, 2.84, 2.78, 2.77, 2.77, 2.76, 2.77, 2.79, 2.80, 2.79, 2.74, 2.67, 2.67, 2.68, 2.67, 2.67, 2.63, 2.58, 2.59, 2.64, 2.74, 2.75, 2.71, 2.72, 2.74, 2.67, 2.63, 2.61, 2.63, 2.65, 2.70, 2.70, 2.72, 2.77, 2.70, 2.66, 2.69, 2.72, 2.76, 2.68, 2.64, 2.61, 2.59, 2.49, 2.46, 2.43, 2.44, 2.38, 2.32, 2.27, 2.25, 2.24, 2.23, 2.22, 2.25, 2.35, 2.47, 2.40, 2.38, 2.35, 2.31, 2.25, 2.24, 2.32, 2.38, 2.35, 2.37, 2.38, 2.33, 2.32, 2.28, 2.25, 2.26, 2.21, 2.21, 2.23, 2.24, 2.17, 2.21, 2.27, 2.33, 2.38, 2.40, 2.38, 2.40, 2.46, 2.47, 2.43, 2.42, 2.45, 2.49, 2.49, 2.56, 2.60, 2.58, 2.59, 2.64, 2.67, 2.66, 2.69, 2.68, 2.66, 2.62, 2.62, 2.58, 2.62, 2.62, 2.56, 2.57, 2.56, 2.50, 2.45, 2.46, 2.42, 2.38, 2.33, 2.28, 2.35, 2.36, 2.35, 2.28, 2.28, 2.21, 2.19, 2.18, 2.13, 2.18, 2.18, 2.14, 2.11, 2.09, 2.03, 2.03, 2.00, 2.02, 2.03, 2.08, 2.11, 2.06, 2.07, 2.05, 2.05, 2.03, 2.04, 2.03, 2.07, 2.15, 2.19, 2.17, 2.14, 2.14, 2.15, 2.17, 2.13, 2.16, 2.15, 2.12, 2.13, 2.10, 2.14, 2.18, 2.21, 2.19, 2.15, 2.12, 2.10, 2.08, 2.07, 2.03, 2.07, 2.06, 2.06, 2.11, 2.14, 2.17, 2.12, 2.08, 2.06, 2.08, 2.08, 2.12, 2.13, 2.12, 2.08, 2.00, 2.05, 2.06, 2.02, 2.03, 2.00, 2.04, 2.06, 2.13, 2.07, 2.08, 2.10, 2.09, 2.10, 2.07, 2.12, 2.16, 2.17, 2.19, 2.19, 2.14, 2.13, 2.14, 2.13, 2.10, 2.12, 2.11, 2.15, 2.13, 2.11, 2.08, 2.11, 2.13, 2.18, 2.19, 2.18, 2.21, 2.26, 2.28, 2.29, 2.21, 2.24, 2.27, 2.27, 2.28, 2.24, 2.25, 2.26, 2.22, 2.19, 2.21, 2.22, 2.32, 2.34, 2.39, 2.41, 2.40, 2.41, 2.44, 2.42, 2.40, 2.34, 2.34, 2.32, 2.27, 2.28, 2.30, 2.28, 2.22, 2.24, 2.20, 2.11, 2.11, 2.12, 2.12, 2.14, 2.18, 2.22, 2.26, 2.37, 2.37, 2.29, 2.28, 2.32, 2.28, 2.25, 2.25, 2.26, 2.19, 2.18, 2.18, 2.18, 2.25, 2.20, 2.18, 2.15, 2.12, 2.15, 2.09, 2.13, 2.21, 2.18, 2.26, 2.22, 2.26, 2.28, 2.36, 2.41, 2.41, 2.40, 2.42, 2.49, 2.48, 2.47, 2.40, 2.45, 2.47, 2.49, 2.47, 2.45, 2.48, 2.45, 2.43, 2.44, 2.44, 2.39, 2.42, 2.43, 2.35, 2.31, 2.33, 2.28, 2.22, 2.23, 2.22, 2.18, 2.27, 2.31, 2.35, 2.40, 2.35, 2.34, 2.31, 2.28, 2.31, 2.26, 2.27, 2.33, 2.30, 2.34, 2.28, 2.21, 2.23, 2.21, 2.25, 2.29, 2.30, 2.36, 2.43, 2.42, 2.43, 2.37, 2.36, 2.31, 2.37, 2.32, 2.27, 2.32, 2.38, 2.28, 2.23, 2.27, 2.25, 2.22, 2.23, 2.27, 2.31, 2.38, 2.41, 2.43, 2.46, 2.50, 2.43, 2.39, 2.39, 2.42, 2.49, 2.52, 2.51, 2.48, 2.44, 2.51, 2.56, 2.56, 2.51, 2.54, 2.54, 2.51, 2.50, 2.48, 2.52, 2.56, 2.61, 2.66, 2.67, 2.66, 2.64, 2.62, 2.58, 2.55, 2.60, 2.64, 2.59, 2.54, 2.55, 2.54, 2.52, 2.50, 2.44, 2.51, 2.56, 2.49, 2.45, 2.43, 2.39, 2.35, 2.33, 2.32, 2.32, 2.27, 2.29, 2.26, 2.29, 2.25, 2.36, 2.40, 2.48, 2.58, 2.63, 2.69, 2.73, 2.68, 2.62, 2.61, 2.64, 2.67, 2.67, 2.55, 2.58, 2.62, 2.58, 2.55, 2.51, 2.48, 2.54, 2.52, 2.47, 2.46, 2.55, 2.61, 2.57, 2.54, 2.52, 2.44, 2.42, 2.37, 2.42, 2.45, 2.35, 2.36, 2.39, 2.42, 2.37, 2.32, 2.30, 2.22, 2.17, 2.18, 2.08, 2.10, 2.16, 2.04, 2.06, 2.03, 1.97, 1.95, 1.97, 1.94, 1.97, 1.90, 1.88, 1.90, 1.77, 1.76, 1.76, 1.79, 1.81, 1.94, 1.91, 1.91, 1.97, 0.62, 0.62, 0.72, 0.83, 0.86, 0.83, 0.80, 0.83, 0.88, 0.93, 0.96, 0.94, 0.90, 0.90, 0.87, 0.82, 0.76, 0.74, 0.72, 0.67, 0.76, 0.86, 0.94, 1.03, 1.02, 1.01, 1.00, 1.07, 1.05, 1.10, 1.11, 1.16, 1.16, 1.24, 1.28, 1.32, 1.28, 1.26, 1.26, 1.24, 1.22, 1.22, 1.21, 1.19, 1.11, 1.06, 1.00, 0.97, 0.94, 0.88, 0.85, 0.81, 0.76, 0.74, 0.74, 0.73, 0.68, 0.65, 0.66, 0.67, 0.67, 0.65, 0.67, 0.69, 0.70, 0.69, 0.67, 0.68, 0.67, 0.68, 0.66, 0.70, 0.70, 0.72, 0.75, 0.73, 0.70, 0.71, 0.67, 0.71, 0.74, 0.79, 0.82, 0.82, 0.79, 0.75, 0.72, 0.71, 0.68, 0.65, 0.63, 0.60, 0.54, 0.53, 0.49, 0.49, 0.49, 0.49, 0.50, 0.51, 0.48, 0.50, 0.48, 0.49, 0.48, 0.46, 0.41, 0.45, 0.48, 0.52, 0.53, 0.52, 0.53, 0.49, 0.50, 0.47, 0.47, 0.44, 0.43, 0.39 };
	
	float vis_group_velocity = 24.; //cm/ns around 420nm
	float peakoffset = 28416.;// + 1000.*vis_group_velocity;
	//convert the axis from ns to cm path travelled
	for (int ipoint = 0; ipoint < nledpoints; ipoint++){
		x_LED[ipoint] = x_LED[ipoint] * 1000. * vis_group_velocity - peakoffset;
	}
	
	TGLEDPulse = new TGraph(nledpoints,x_LED,y_LED);	
	TGLEDPulse->GetYaxis()->SetRangeUser(0.1, 500);
	TFile *f1 = new TFile("wls_0_30.root");
	TH1D *hvislength = (TH1D*)f1->Get("vislength");
	//hvislength->Draw();
	TGLEDPulse->SetMarkerStyle(5);
	TGLEDPulse->Draw("aP");
	
	TGLEDPulse->SetTitle("LED pulse (LED16)");
	TGLEDPulse->GetXaxis()->SetTitle("Path length [cm]");
	TF1 *fexp = new TF1("fexp", "[0]*exp(-x/[1])");
		fexp->SetParameters(9.5, 48054.);
	// box function convolved with fexpmod gives the same fexpmod shape starting at the end of the box function, i.e. at times > 200 ns, since the LED pulse is a box with 200 ns width
	TF1 *fexpmod = new TF1("fexpmod","[0]*[1]/2.*exp( [1]/2. * (2*[2] + [1]*[3]*[3] - 2*x) ) * TMath::Erfc( ([2] + [1]*[3]*[3] - x)/(sqrt(2)*[3]) )",-2000,20000);
	fexpmod->SetParName(1, "lambda"); //note this is 1/lambda_abs
	fexpmod->SetParName(2, "mu");
	fexpmod->SetParName(3, "sigma");		
	TF1 *fprob = new TF1("fprob","[0]*exp(-x/[1]) * pow([2],x/[3])",-1000,20000);
	fprob->SetParName(1, "LAr vis abs. length");
	fprob->SetParName(2, "WLSR reflection probability");	
	fprob->SetParLimits(2, 0.9, 0.98);
	fprob->SetParName(3, "mean photon pathlength");
	fprob->SetParLimits(3, 40, 100);	
	fprob->SetParameters(400,12000,0.98, 50);
	fexpmod->SetParameters(2.6128e+06, 1./3191.9, -921, 857.);
	//TGLEDPulse->Fit("fexp","","",20e3, 60e3);
	cout << "Effective light attenuation length is " << fexp->GetParameter(1) << " cm" << endl;
}

void FillLEDPulseshape2() {
	
	int nledpoints = 1000;
	// Feb 7 and 8 data LED16 and LED18
	Double_t x_LED[1000] = { 0.00, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27, 0.28, 0.29, 0.30, 0.31, 0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40, 0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.50, 0.51, 0.52, 0.53, 0.54, 0.55, 0.56, 0.57, 0.58, 0.59, 0.60, 0.61, 0.62, 0.63, 0.64, 0.65, 0.66, 0.67, 0.68, 0.69, 0.70, 0.71, 0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.79, 0.80, 0.81, 0.82, 0.83, 0.84, 0.85, 0.86, 0.87, 0.88, 0.89, 0.90, 0.91, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99, 1.00, 1.01, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08, 1.09, 1.10, 1.11, 1.12, 1.13, 1.14, 1.15, 1.16, 1.17, 1.18, 1.19, 1.20, 1.21, 1.22, 1.23, 1.24, 1.25, 1.26, 1.27, 1.28, 1.29, 1.30, 1.31, 1.32, 1.33, 1.34, 1.35, 1.36, 1.37, 1.38, 1.39, 1.40, 1.41, 1.42, 1.43, 1.44, 1.45, 1.46, 1.47, 1.48, 1.49, 1.50, 1.51, 1.52, 1.53, 1.54, 1.55, 1.56, 1.57, 1.58, 1.59, 1.60, 1.61, 1.62, 1.63, 1.64, 1.65, 1.66, 1.67, 1.68, 1.69, 1.70, 1.71, 1.72, 1.73, 1.74, 1.75, 1.76, 1.77, 1.78, 1.79, 1.80, 1.81, 1.82, 1.83, 1.84, 1.85, 1.86, 1.87, 1.88, 1.89, 1.90, 1.91, 1.92, 1.93, 1.94, 1.95, 1.96, 1.97, 1.98, 1.99, 2.00, 2.01, 2.02, 2.03, 2.04, 2.05, 2.06, 2.07, 2.08, 2.09, 2.10, 2.11, 2.12, 2.13, 2.14, 2.15, 2.16, 2.17, 2.18, 2.19, 2.20, 2.21, 2.22, 2.23, 2.24, 2.25, 2.26, 2.27, 2.28, 2.29, 2.30, 2.31, 2.32, 2.33, 2.34, 2.35, 2.36, 2.37, 2.38, 2.39, 2.40, 2.41, 2.42, 2.43, 2.44, 2.45, 2.46, 2.47, 2.48, 2.49, 2.50, 2.51, 2.52, 2.53, 2.54, 2.55, 2.56, 2.57, 2.58, 2.59, 2.60, 2.61, 2.62, 2.63, 2.64, 2.65, 2.66, 2.67, 2.68, 2.69, 2.70, 2.71, 2.72, 2.73, 2.74, 2.75, 2.76, 2.77, 2.78, 2.79, 2.80, 2.81, 2.82, 2.83, 2.84, 2.85, 2.86, 2.87, 2.88, 2.89, 2.90, 2.91, 2.92, 2.93, 2.94, 2.95, 2.96, 2.97, 2.98, 2.99, 3.00, 3.01, 3.02, 3.03, 3.04, 3.05, 3.06, 3.07, 3.08, 3.09, 3.10, 3.11, 3.12, 3.13, 3.14, 3.15, 3.16, 3.17, 3.18, 3.19, 3.20, 3.21, 3.22, 3.23, 3.24, 3.25, 3.26, 3.27, 3.28, 3.29, 3.30, 3.31, 3.32, 3.33, 3.34, 3.35, 3.36, 3.37, 3.38, 3.39, 3.40, 3.41, 3.42, 3.43, 3.44, 3.45, 3.46, 3.47, 3.48, 3.49, 3.50, 3.51, 3.52, 3.53, 3.54, 3.55, 3.56, 3.57, 3.58, 3.59, 3.60, 3.61, 3.62, 3.63, 3.64, 3.65, 3.66, 3.67, 3.68, 3.69, 3.70, 3.71, 3.72, 3.73, 3.74, 3.75, 3.76, 3.77, 3.78, 3.79, 3.80, 3.81, 3.82, 3.83, 3.84, 3.85, 3.86, 3.87, 3.88, 3.89, 3.90, 3.91, 3.92, 3.93, 3.94, 3.95, 3.96, 3.97, 3.98, 3.99, 4.00, 4.01, 4.02, 4.03, 4.04, 4.05, 4.06, 4.07, 4.08, 4.09, 4.10, 4.11, 4.12, 4.13, 4.14, 4.15, 4.16, 4.17, 4.18, 4.19, 4.20, 4.21, 4.22, 4.23, 4.24, 4.25, 4.26, 4.27, 4.28, 4.29, 4.30, 4.31, 4.32, 4.33, 4.34, 4.35, 4.36, 4.37, 4.38, 4.39, 4.40, 4.41, 4.42, 4.43, 4.44, 4.45, 4.46, 4.47, 4.48, 4.49, 4.50, 4.51, 4.52, 4.53, 4.54, 4.55, 4.56, 4.57, 4.58, 4.59, 4.60, 4.61, 4.62, 4.63, 4.64, 4.65, 4.66, 4.67, 4.68, 4.69, 4.70, 4.71, 4.72, 4.73, 4.74, 4.75, 4.76, 4.77, 4.78, 4.79, 4.80, 4.81, 4.82, 4.83, 4.84, 4.85, 4.86, 4.87, 4.88, 4.89, 4.90, 4.91, 4.92, 4.93, 4.94, 4.95, 4.96, 4.97, 4.98, 4.99, 5.01, 5.02, 5.03, 5.04, 5.05, 5.06, 5.07, 5.08, 5.09, 5.10, 5.11, 5.12, 5.13, 5.14, 5.15, 5.16, 5.17, 5.18, 5.19, 5.20, 5.21, 5.22, 5.23, 5.24, 5.25, 5.26, 5.27, 5.28, 5.29, 5.30, 5.31, 5.32, 5.33, 5.34, 5.35, 5.36, 5.37, 5.38, 5.39, 5.40, 5.41, 5.42, 5.43, 5.44, 5.45, 5.46, 5.47, 5.48, 5.49, 5.50, 5.51, 5.52, 5.53, 5.54, 5.55, 5.56, 5.57, 5.58, 5.59, 5.60, 5.61, 5.62, 5.63, 5.64, 5.65, 5.66, 5.67, 5.68, 5.69, 5.70, 5.71, 5.72, 5.73, 5.74, 5.75, 5.76, 5.77, 5.78, 5.79, 5.80, 5.81, 5.82, 5.83, 5.84, 5.85, 5.86, 5.87, 5.88, 5.89, 5.90, 5.91, 5.92, 5.93, 5.94, 5.95, 5.96, 5.97, 5.98, 5.99, 6.00, 6.01, 6.02, 6.03, 6.04, 6.05, 6.06, 6.07, 6.08, 6.09, 6.10, 6.11, 6.12, 6.13, 6.14, 6.15, 6.16, 6.17, 6.18, 6.19, 6.20, 6.21, 6.22, 6.23, 6.24, 6.25, 6.26, 6.27, 6.28, 6.29, 6.30, 6.31, 6.32, 6.33, 6.34, 6.35, 6.36, 6.37, 6.38, 6.39, 6.40, 6.41, 6.42, 6.43, 6.44, 6.45, 6.46, 6.47, 6.48, 6.49, 6.50, 6.51, 6.52, 6.53, 6.54, 6.55, 6.56, 6.57, 6.58, 6.59, 6.60, 6.61, 6.62, 6.63, 6.64, 6.65, 6.66, 6.67, 6.68, 6.69, 6.70, 6.71, 6.72, 6.73, 6.74, 6.75, 6.76, 6.77, 6.78, 6.79, 6.80, 6.81, 6.82, 6.83, 6.84, 6.85, 6.86, 6.87, 6.88, 6.89, 6.90, 6.91, 6.92, 6.93, 6.94, 6.95, 6.96, 6.97, 6.98, 6.99, 7.00, 7.01, 7.02, 7.03, 7.04, 7.05, 7.06, 7.07, 7.08, 7.09, 7.10, 7.11, 7.12, 7.13, 7.14, 7.15, 7.16, 7.17, 7.18, 7.19, 7.20, 7.21, 7.22, 7.23, 7.24, 7.25, 7.26, 7.27, 7.28, 7.29, 7.30, 7.31, 7.32, 7.33, 7.34, 7.35, 7.36, 7.37, 7.38, 7.39, 7.40, 7.41, 7.42, 7.43, 7.44, 7.45, 7.46, 7.47, 7.48, 7.49, 7.50, 7.51, 7.52, 7.53, 7.54, 7.55, 7.56, 7.57, 7.58, 7.59, 7.60, 7.61, 7.62, 7.63, 7.64, 7.65, 7.66, 7.67, 7.68, 7.69, 7.70, 7.71, 7.72, 7.73, 7.74, 7.75, 7.76, 7.77, 7.78, 7.79, 7.80, 7.81, 7.82, 7.83, 7.84, 7.85, 7.86, 7.87, 7.88, 7.89, 7.90, 7.91, 7.92, 7.93, 7.94, 7.95, 7.96, 7.97, 7.98, 7.99, 8.00, 8.01, 8.02, 8.03, 8.04, 8.05, 8.06, 8.07, 8.08, 8.09, 8.10, 8.11, 8.12, 8.13, 8.14, 8.15, 8.16, 8.17, 8.18, 8.19, 8.20, 8.21, 8.22, 8.23, 8.24, 8.25, 8.26, 8.27, 8.28, 8.29, 8.30, 8.31, 8.32, 8.33, 8.34, 8.35, 8.36, 8.37, 8.38, 8.39, 8.40, 8.41, 8.42, 8.43, 8.44, 8.45, 8.46, 8.47, 8.48, 8.49, 8.50, 8.51, 8.52, 8.53, 8.54, 8.55, 8.56, 8.57, 8.58, 8.59, 8.60, 8.61, 8.62, 8.63, 8.64, 8.65, 8.66, 8.67, 8.68, 8.69, 8.70, 8.71, 8.72, 8.73, 8.74, 8.75, 8.76, 8.77, 8.78, 8.79, 8.80, 8.81, 8.82, 8.83, 8.84, 8.85, 8.86, 8.87, 8.88, 8.89, 8.90, 8.91, 8.92, 8.93, 8.94, 8.95, 8.96, 8.97, 8.98, 8.99, 9.00, 9.01, 9.02, 9.03, 9.04, 9.05, 9.06, 9.07, 9.08, 9.09, 9.10, 9.11, 9.12, 9.13, 9.14, 9.15, 9.16, 9.17, 9.18, 9.19, 9.20, 9.21, 9.22, 9.23, 9.24, 9.25, 9.26, 9.27, 9.28, 9.29, 9.30, 9.31, 9.32, 9.33, 9.34, 9.35, 9.36, 9.37, 9.38, 9.39, 9.40, 9.41, 9.42, 9.43, 9.44, 9.45, 9.46, 9.47, 9.48, 9.49, 9.50, 9.51, 9.52, 9.53, 9.54, 9.55, 9.56, 9.57, 9.58, 9.59, 9.60, 9.61, 9.62, 9.63, 9.64, 9.65, 9.66, 9.67, 9.68, 9.69, 9.70, 9.71, 9.72, 9.73, 9.74, 9.75, 9.76, 9.77, 9.78, 9.79, 9.80, 9.81, 9.82, 9.83, 9.84, 9.85, 9.86, 9.87, 9.88, 9.89, 9.90, 9.91, 9.92, 9.93, 9.94, 9.95, 9.96, 9.97, 9.98, 9.99, 10.00 };
	Double_t y_LED[1000] = { 0.38, 0.37, 0.40, 0.38, 0.36, 0.34, 0.32, 0.31, 0.32, 0.32, 0.29, 0.29, 0.31, 0.34, 0.40, 0.45, 0.43, 0.43, 0.44, 0.50, 0.52, 0.56, 0.56, 0.56, 0.54, 0.56, 0.53, 0.48, 0.45, 0.48, 0.49, 0.47, 0.42, 0.41, 0.45, 0.44, 0.41, 0.44, 0.40, 0.40, 0.43, 0.46, 0.53, 0.58, 0.63, 0.67, 0.68, 0.69, 0.70, 0.66, 0.63, 0.57, 0.53, 0.51, 0.48, 0.46, 0.45, 0.45, 0.44, 0.44, 0.43, 0.46, 0.42, 0.39, 0.39, 0.39, 0.39, 0.34, 0.31, 0.33, 0.31, 0.30, 0.33, 0.55, 1.25, 1.31, 1.10, 1.89, 2.40, 2.67, 2.42, 1.52, 1.84, 1.81, 0.28, -1.24, -1.86, -2.71, -4.13, -4.42, -4.33, -4.58, -4.75, -4.06, -1.93, -0.10, 0.50, 1.83, 3.18, 2.28, -0.06, -1.73, -2.01, -1.42, -0.49, 1.51, 3.90, 5.21, 6.83, 18.75, 53.10, 106.21, 167.72, 236.29, 310.34, 380.46, 434.56, 468.95, 486.03, 489.72, 482.73, 469.70, 454.55, 436.49, 416.64, 394.93, 373.46, 350.86, 326.33, 302.28, 279.80, 258.60, 238.78, 220.30, 203.95, 188.38, 172.53, 157.19, 143.05, 129.84, 118.62, 108.92, 100.47, 92.90, 85.41, 78.20, 71.38, 64.99, 58.80, 53.58, 49.72, 46.85, 43.77, 40.92, 38.39, 35.66, 32.37, 29.31, 27.27, 25.60, 24.23, 23.12, 22.40, 21.68, 20.19, 18.79, 17.28, 16.02, 14.94, 14.24, 14.08, 14.02, 13.61, 12.95, 12.32, 11.72, 11.12, 10.54, 10.29, 10.11, 9.73, 9.28, 8.99, 8.73, 8.36, 7.99, 7.91, 8.07, 8.04, 7.84, 7.62, 7.30, 7.00, 6.68, 6.57, 6.61, 6.74, 6.94, 7.09, 7.08, 6.90, 6.61, 6.45, 6.36, 6.36, 6.43, 6.46, 6.57, 6.61, 6.42, 6.23, 6.22, 6.07, 6.05, 6.27, 6.44, 6.44, 6.37, 6.13, 5.89, 5.74, 5.66, 5.58, 5.71, 5.81, 5.92, 5.86, 5.67, 5.34, 5.18, 5.28, 5.54, 5.74, 5.85, 5.89, 5.87, 5.75, 5.47, 5.22, 5.01, 5.03, 5.11, 5.15, 5.33, 5.39, 5.25, 5.19, 4.95, 4.79, 4.89, 5.06, 5.17, 5.24, 5.31, 5.34, 5.30, 5.15, 5.09, 5.12, 5.22, 5.21, 5.26, 5.20, 5.07, 4.93, 4.79, 4.73, 4.73, 4.91, 5.00, 5.02, 4.92, 4.85, 4.75, 4.61, 4.59, 4.50, 4.58, 4.62, 4.73, 4.80, 4.74, 4.62, 4.56, 4.52, 4.56, 4.60, 4.52, 4.46, 4.45, 4.34, 4.26, 4.22, 4.16, 4.07, 4.09, 4.12, 4.08, 4.06, 3.95, 3.91, 3.87, 3.89, 3.99, 4.06, 4.08, 4.00, 3.88, 3.79, 3.74, 3.72, 3.77, 3.79, 3.79, 3.72, 3.66, 3.51, 3.49, 3.47, 3.38, 3.37, 3.41, 3.41, 3.44, 3.35, 3.31, 3.21, 3.16, 3.16, 3.26, 3.38, 3.46, 3.40, 3.37, 3.30, 3.26, 3.21, 3.25, 3.31, 3.33, 3.30, 3.30, 3.23, 3.13, 3.09, 3.18, 3.22, 3.29, 3.35, 3.32, 3.30, 3.28, 3.21, 3.23, 3.20, 3.22, 3.20, 3.17, 3.18, 3.11, 3.02, 2.99, 2.97, 2.91, 2.92, 2.97, 2.92, 2.89, 2.85, 2.72, 2.65, 2.64, 2.67, 2.65, 2.69, 2.72, 2.72, 2.66, 2.59, 2.53, 2.51, 2.50, 2.59, 2.78, 2.77, 2.75, 2.66, 2.66, 2.69, 2.69, 2.69, 2.75, 2.76, 2.75, 2.84, 2.85, 2.84, 2.78, 2.77, 2.77, 2.76, 2.77, 2.79, 2.80, 2.79, 2.74, 2.67, 2.67, 2.68, 2.67, 2.67, 2.63, 2.58, 2.59, 2.64, 2.74, 2.75, 2.71, 2.72, 2.74, 2.67, 2.63, 2.61, 2.63, 2.65, 2.70, 2.70, 2.72, 2.77, 2.70, 2.66, 2.69, 2.72, 2.76, 2.68, 2.64, 2.61, 2.59, 2.49, 2.46, 2.43, 2.44, 2.38, 2.32, 2.27, 2.25, 2.24, 2.23, 2.22, 2.25, 2.35, 2.47, 2.40, 2.38, 2.35, 2.31, 2.25, 2.24, 2.32, 2.38, 2.35, 2.37, 2.38, 2.33, 2.32, 2.28, 2.25, 2.26, 2.21, 2.21, 2.23, 2.24, 2.17, 2.21, 2.27, 2.33, 2.38, 2.40, 2.38, 2.40, 2.46, 2.47, 2.43, 2.42, 2.45, 2.49, 2.49, 2.56, 2.60, 2.58, 2.59, 2.64, 2.67, 2.66, 2.69, 2.68, 2.66, 2.62, 2.62, 2.58, 2.62, 2.62, 2.56, 2.57, 2.56, 2.50, 2.45, 2.46, 2.42, 2.38, 2.33, 2.28, 2.35, 2.36, 2.35, 2.28, 2.28, 2.21, 2.19, 2.18, 2.13, 2.18, 2.18, 2.14, 2.11, 2.09, 2.03, 2.03, 2.00, 2.02, 2.03, 2.08, 2.11, 2.06, 2.07, 2.05, 2.05, 2.03, 2.04, 2.03, 2.07, 2.15, 2.19, 2.17, 2.14, 2.14, 2.15, 2.17, 2.13, 2.16, 2.15, 2.12, 2.13, 2.10, 2.14, 2.18, 2.21, 2.19, 2.15, 2.12, 2.10, 2.08, 2.07, 2.03, 2.07, 2.06, 2.06, 2.11, 2.14, 2.17, 2.12, 2.08, 2.06, 2.08, 2.08, 2.12, 2.13, 2.12, 2.08, 2.00, 2.05, 2.06, 2.02, 2.03, 2.00, 2.04, 2.06, 2.13, 2.07, 2.08, 2.10, 2.09, 2.10, 2.07, 2.12, 2.16, 2.17, 2.19, 2.19, 2.14, 2.13, 2.14, 2.13, 2.10, 2.12, 2.11, 2.15, 2.13, 2.11, 2.08, 2.11, 2.13, 2.18, 2.19, 2.18, 2.21, 2.26, 2.28, 2.29, 2.21, 2.24, 2.27, 2.27, 2.28, 2.24, 2.25, 2.26, 2.22, 2.19, 2.21, 2.22, 2.32, 2.34, 2.39, 2.41, 2.40, 2.41, 2.44, 2.42, 2.40, 2.34, 2.34, 2.32, 2.27, 2.28, 2.30, 2.28, 2.22, 2.24, 2.20, 2.11, 2.11, 2.12, 2.12, 2.14, 2.18, 2.22, 2.26, 2.37, 2.37, 2.29, 2.28, 2.32, 2.28, 2.25, 2.25, 2.26, 2.19, 2.18, 2.18, 2.18, 2.25, 2.20, 2.18, 2.15, 2.12, 2.15, 2.09, 2.13, 2.21, 2.18, 2.26, 2.22, 2.26, 2.28, 2.36, 2.41, 2.41, 2.40, 2.42, 2.49, 2.48, 2.47, 2.40, 2.45, 2.47, 2.49, 2.47, 2.45, 2.48, 2.45, 2.43, 2.44, 2.44, 2.39, 2.42, 2.43, 2.35, 2.31, 2.33, 2.28, 2.22, 2.23, 2.22, 2.18, 2.27, 2.31, 2.35, 2.40, 2.35, 2.34, 2.31, 2.28, 2.31, 2.26, 2.27, 2.33, 2.30, 2.34, 2.28, 2.21, 2.23, 2.21, 2.25, 2.29, 2.30, 2.36, 2.43, 2.42, 2.43, 2.37, 2.36, 2.31, 2.37, 2.32, 2.27, 2.32, 2.38, 2.28, 2.23, 2.27, 2.25, 2.22, 2.23, 2.27, 2.31, 2.38, 2.41, 2.43, 2.46, 2.50, 2.43, 2.39, 2.39, 2.42, 2.49, 2.52, 2.51, 2.48, 2.44, 2.51, 2.56, 2.56, 2.51, 2.54, 2.54, 2.51, 2.50, 2.48, 2.52, 2.56, 2.61, 2.66, 2.67, 2.66, 2.64, 2.62, 2.58, 2.55, 2.60, 2.64, 2.59, 2.54, 2.55, 2.54, 2.52, 2.50, 2.44, 2.51, 2.56, 2.49, 2.45, 2.43, 2.39, 2.35, 2.33, 2.32, 2.32, 2.27, 2.29, 2.26, 2.29, 2.25, 2.36, 2.40, 2.48, 2.58, 2.63, 2.69, 2.73, 2.68, 2.62, 2.61, 2.64, 2.67, 2.67, 2.55, 2.58, 2.62, 2.58, 2.55, 2.51, 2.48, 2.54, 2.52, 2.47, 2.46, 2.55, 2.61, 2.57, 2.54, 2.52, 2.44, 2.42, 2.37, 2.42, 2.45, 2.35, 2.36, 2.39, 2.42, 2.37, 2.32, 2.30, 2.22, 2.17, 2.18, 2.08, 2.10, 2.16, 2.04, 2.06, 2.03, 1.97, 1.95, 1.97, 1.94, 1.97, 1.90, 1.88, 1.90, 1.77, 1.76, 1.76, 1.79, 1.81, 1.94, 1.91, 1.91, 1.97, 0.62, 0.62, 0.72, 0.83, 0.86, 0.83, 0.80, 0.83, 0.88, 0.93, 0.96, 0.94, 0.90, 0.90, 0.87, 0.82, 0.76, 0.74, 0.72, 0.67, 0.76, 0.86, 0.94, 1.03, 1.02, 1.01, 1.00, 1.07, 1.05, 1.10, 1.11, 1.16, 1.16, 1.24, 1.28, 1.32, 1.28, 1.26, 1.26, 1.24, 1.22, 1.22, 1.21, 1.19, 1.11, 1.06, 1.00, 0.97, 0.94, 0.88, 0.85, 0.81, 0.76, 0.74, 0.74, 0.73, 0.68, 0.65, 0.66, 0.67, 0.67, 0.65, 0.67, 0.69, 0.70, 0.69, 0.67, 0.68, 0.67, 0.68, 0.66, 0.70, 0.70, 0.72, 0.75, 0.73, 0.70, 0.71, 0.67, 0.71, 0.74, 0.79, 0.82, 0.82, 0.79, 0.75, 0.72, 0.71, 0.68, 0.65, 0.63, 0.60, 0.54, 0.53, 0.49, 0.49, 0.49, 0.49, 0.50, 0.51, 0.48, 0.50, 0.48, 0.49, 0.48, 0.46, 0.41, 0.45, 0.48, 0.52, 0.53, 0.52, 0.53, 0.49, 0.50, 0.47, 0.47, 0.44, 0.43, 0.39 };
	
	float vis_group_velocity = 24.; //cm/ns around 420nm
	float peakoffset = 1200.;
	for (int ipoint = 0; ipoint < nledpoints; ipoint++){
		x_LED[ipoint] = x_LED[ipoint]*1000. - peakoffset;
	}
	
	TGLEDPulse = new TGraph(nledpoints,x_LED,y_LED);	
	TGLEDPulse->GetYaxis()->SetRangeUser(0.1, 500);
	TGLEDPulse->SetMarkerStyle(3);
	TGLEDPulse->Draw("aP");
	
	TGLEDPulse->SetTitle("LED pulse (LED16)");
	TGLEDPulse->GetXaxis()->SetTitle("Time [ns]");	
	TF1 *fprob = new TF1("fprob","[0]*exp(-x*[4]/[1]) * pow([2],x*[4]/[3]) * pow((1-0.0018),x*[4]/[3])",-1000,20000);
	fprob->SetParName(1, "LAr vis abs. length");
	fprob->SetParName(2, "WLSR reflection probability");	
	fprob->SetParLimits(2, 0.95,0.989);
	fprob->SetParName(3, "mean photon pathlength");
	fprob->SetParLimits(3, 59, 63);	
	fprob->SetParName(4, "group velocity");
	fprob->FixParameter(4, vis_group_velocity);
	fprob->SetParameters(400,5000,0.95, 40 ,vis_group_velocity);
	TGLEDPulse->Fit("fprob","","",80, 400);
	cout << "Effective light attenuation length is " << fprob->GetParameter(1) << " cm" << endl;
}

void DrawVUVl() {
for(int ifile = 0; ifile < nfiles; ifile++) {
	new TCanvas();
	VUVl[ifile]->SetTitle(filenames[ifile].c_str());
	VUVl[ifile]->Draw();
}
}


float ApplyAbsorptionLength(float absl, TH1D *hlengths) {
	float I_before = 0.;
	float I_after = 0.;	
	for (int ibin = 5; ibin < hlengths->GetNbinsX(); ibin++) {
		I_before = I_before + hlengths->GetBinContent(ibin);
		I_after = I_after + hlengths->GetBinContent(ibin) * exp(-hlengths->GetBinCenter(ibin)/absl);
	}
	return I_after/I_before;
}







