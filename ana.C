
TGraphErrors *TGData_centre;
TGraphErrors *TGData_right;
TGraphErrors *TGData_left;
void FillDataGraphs(float coordinate_offset_z);

int ana() {
	gStyle->SetOptStat(0);

	float source_geo_frac = 0.35; 
	float alphaEnergy = 4800; // keV
	float LAr_alphaQ = 0.71; // alpha quenching
	float LAr_py = 40; // photons per keV
	float dLAr_py = 0.1; // fractional uncertainty, about 1sigma
	float PMTEfficiency = 0.16;
	float dPMTEfficiency = 0.01;

	float PENWLSE = 0.5;
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
	const int nfiles = 10;
	//string filenames[9] = {"wls_0_0.root", "wls_0_m20.root", "wls_0_20.root","wls_0_m40.root", "wls_0_40.root", "wls_0_40_am50.root", "wls_0_40_a50.root", "wls_0_m40_am50.root", "wls_0_m40_a50.root"};
	string filenames[10] = {"wls_0_30.root", "wls_0_22p5.root", "wls_0_m7p5.root", "wls_0_m45.root", "wls_50_20.root", "wls_0_50_m7p5.root", "wls_50_m45.root", "wls_0_m50_20.root", "wls_0_m50_m7p5.root", "wls_0_m50_m45.root"};

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
	float pointoffset = 0;
	for (int ifiles = 0; ifiles < nfiles; ifiles++) {
		fin = new TFile(filenames[ifiles].c_str());
		fh = (TH1D*)fin->Get("fraction");
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
	new TCanvas();
	tg_eta->SetTitle("");
	tg_eta->GetXaxis()->SetTitle("Source y position [cm]");
	tg_eta->GetYaxis()->SetTitle("fraction of photons detected");
	tg_eta->SetMarkerStyle(2);
	tg_eta->SetMarkerColor(2);
	tg_eta->Draw("AP");
	
	
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
*/

TGData_centre->SetPoint(0,coordinate_offset_z - 0.00, 67.62);
TGData_centre->SetPointError(0, 0, 0.08);
cout << "0. " << coordinate_offset_z - 0.00 << " 0. " << endl;

TGData_centre->SetPoint(1, coordinate_offset_z - 7.50, 57.45);
TGData_centre->SetPointError(1, 0, 0.08);
cout << "0. " << coordinate_offset_z - 7.50 << " 0. " << endl;

TGData_centre->SetPoint(2, coordinate_offset_z - 37.50, 42.06); 
TGData_centre->SetPointError(2, 0, 0.11);
cout << "0. " << coordinate_offset_z - 37.50 << " 0. " << endl;

TGData_centre->SetPoint(3, coordinate_offset_z - 75.00, 40.87); 
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

TGData_left->SetPoint(0, coordinate_offset_z - 10.0/10. - 1.5, 102.61); 
TGData_left->SetPointError(0, 0, 1.74);
cout << "-50. " << coordinate_offset_z - 10.0/10 << " 0. " << endl;

TGData_left->SetPoint(1, coordinate_offset_z - 37.50/10 - 1.5, 98.51); 
TGData_left->SetPointError(1, 0, 1.92);
cout << "-50. " << coordinate_offset_z - 37.50/10. << " 0. " << endl;

TGData_left->SetPoint(2, coordinate_offset_z - 75.00/10. - 1.5, 95.50); 
TGData_left->SetPointError(2, 0, 1.57);
cout << "-50. " << coordinate_offset_z - 75.00/10. << " 0. " << endl;




}