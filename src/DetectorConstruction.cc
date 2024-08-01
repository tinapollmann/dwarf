
/// \file Dwarf/src/DetectorConstruction.cc
/// \brief Implementation of the Dwarf::DetectorConstruction class


/*
From Marcin on Slack:
The WLS geometry is hexagonal with:
cage height 1015mm
diameter: 995 mm
860 mm is the shorter diameter (between the walls), probably best to take average between the two
the opening for the pmt is 4 inches

This file creates a nested structure with a hexagonal LAr volume surrounded by a thin layer of PEN, 
surrounded by a thin layer of reflector. A tub volume with the diameter of the PMT opening
serves as the 'detector'.
*/

#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4VisAttributes.hh"
#include "G4Polyhedra.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4UnionSolid.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4VisManager.hh"
#include "globals.hh"
#include "G4Scintillation.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4Threading.hh"
namespace Dwarf
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
DetectorConstruction::DetectorConstruction()
{
	physWorld = nullptr;
	SetDefaults();
  Construct();
  fDetectorMessenger = new DetectorMessenger(this);
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
    
  delete fDetectorMessenger;
  delete logicPMTCath;
  delete logicHexShellPEN;
  delete MPTLAr;
  delete physWorld;
}

void DetectorConstruction::SetDefaults() {
	sourceHolderPosition = G4ThreeVector(0., 0., 0.*cm);

	sourceHolderHalfZHeight = 1.5*cm;
	ESRreflectivity = 0.83;
	LArVUVAbsl = 50.*cm;
	//LArVUVAbsl = 30*cm;
}


G4VPhysicalVolume* DetectorConstruction::Construct()
{
	if (physWorld) { 
        G4GeometryManager::GetInstance()->OpenGeometry();
        G4PhysicalVolumeStore::Clean();
        G4LogicalVolumeStore::Clean();
        G4SolidStore::Clean();
        G4LogicalSkinSurface::CleanSurfaceTable();
        G4LogicalBorderSurface::CleanSurfaceTable();
        //G4VisManager::GetInstance()->GeometryHasChanged();
    }
	// Define materials
	G4NistManager* nist = G4NistManager::Instance();
	G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* LAr = nist->FindOrBuildMaterial("G4_lAr");
	G4Material* PEN = new G4Material("PEN2", 1.36 * g/cm3, 3); //density from https://www.azom.com/article.aspx?ArticleID=1933
	PEN->AddElement(nist->FindOrBuildElement("H"), 14);
	PEN->AddElement(nist->FindOrBuildElement("C"), 10);
	PEN->AddElement(nist->FindOrBuildElement("O"), 4);
	G4Material* aluminum = nist->FindOrBuildMaterial("G4_Al"); // This will stand in for the ESR
	G4Material* lead = nist->FindOrBuildMaterial("G4_Pb"); // for the "PMT". THe material doesn't matter
	
	
	//Define optical properties
	
	// Scintillation properties of liquid argon
								      // [ 600 ; 413(PEN); 387 ;    160 ;    128 (LAr); 124] nm   actual wavelength of PEN does not matter here, as long as other optical properties are set correctly for this wavelength
	std::vector<G4double> photonEnergy = {2.0*eV, 3. * eV, 4.0 *eV, 9.1 * eV, 9.7 * eV, 10. * eV }; 
	std::vector<G4double> scintillationSpectrumLAr = { 0.0, 0.0 , 0.0 ,0.0 , 1.0, 0.}; // peak at 128 nm
	std::vector<G4double> lar_RIND = { 1.23, 1.23, 1.23, 1.36, 1.36, 1.36};
	std::vector<G4double> lar_ABSL = {85. * m, 85.* m, 85. * m, LArVUVAbsl, LArVUVAbsl, LArVUVAbsl}; // from https://iopscience.iop.org/article/10.1088/1748-0221/17/01/C01012
		// Note: 85m comes from fit to the reflection tail on the LED pulseshape
	std::vector<G4double> lar_RAYL = {30 * m, 30 * m, 30 * m, 30 * m, 1.0 * m, 1.0 * m}; //

	MPTLAr = new G4MaterialPropertiesTable();  
	MPTLAr->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintillationSpectrumLAr);
	MPTLAr->AddProperty("SCINTILLATIONCOMPONENT2", photonEnergy, scintillationSpectrumLAr);
	MPTLAr->AddProperty("RINDEX", photonEnergy, lar_RIND);
	MPTLAr->AddProperty("ABSLENGTH", photonEnergy, lar_ABSL);	
	MPTLAr->AddProperty("RAYLEIGH", photonEnergy, lar_RAYL);
	MPTLAr->AddConstProperty("SCINTILLATIONYIELD", 20000. / MeV); // Example value
	MPTLAr->AddConstProperty("RESOLUTIONSCALE", 1.0);
	MPTLAr->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 6. * ns);
	MPTLAr->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 1400. * ns);
	MPTLAr->AddConstProperty("SCINTILLATIONYIELD1", 1.0);// Assuming only one component
	MPTLAr->AddConstProperty("SCINTILLATIONYIELD2", 0.0);	

	LAr->SetMaterialPropertiesTable(MPTLAr);
	  
	// WLS properties of the PEN
	std::vector<G4double> VisEnergies = { 2.0*eV, 2.49969*eV, 2.52001*eV, 2.54067*eV, 2.56167*eV, 2.58301*eV, 2.60472*eV, 2.62679*eV, 2.64925*eV, 2.67208*eV, 2.69532*eV, 2.71896*eV, 2.74302*eV, 2.76752*eV, 2.79245*eV, 2.81783*eV, 2.84369*eV, 2.87002*eV, 2.89684*eV, 2.92417*eV, 2.95202*eV, 2.9804*eV, 3.00934*eV, 3.03884*eV, 3.06893*eV, 3.09962*eV, 3.13093*eV, 3.16288*eV, 3.19548*eV, 3.22877*eV, 3.26276*eV, 3.29747*eV, 3.33292*eV, 3.36915*eV, 3.40617*eV, 3.44402*eV, 3.48272*eV, 3.52229*eV, 3.56278*eV, 3.60421*eV, 4.0 *eV, 10.*eV};
	//std::vector<G4double> PEN_Emission = {0.0, 14.0547, 18.9554, 24.494, 30.347, 37.6237, 48.3961, 56.8418, 68.6785, 78.2151, 84.7811, 85.2151, 82.9122, 73.8898, 54.9558, 38.7634, 22.3622, 5.5479, 2.16497, 1.88172, 0.0 };
	std::vector<G4double> PEN_Emission = {0.0,0.000857878, 0.00114682, 0.00150297, 0.00193117, 0.00243309, 0.00300639, 0.00364431, 0.00433573, 0.00506611, 0.00581912, 0.00657892, 0.00733269, 0.00807265, 0.00879692, 0.00950834, 0.010211, 0.0109048, 0.0115792, 0.0122077, 0.0127465, 0.0131369, 0.0133137, 0.0132177, 0.0128091, 0.0120794, 0.0110558, 0.0098003, 0.00839989, 0.00695257, 0.00555208, 0.004275, 0.00317268, 0.00226919, 0.00156426, 0.00103966, 0.000666598, 0.000412677, 0.000246975, 0.000143115, 0.0 ,0.0}; // from Gabriela's paper, matches Andreas' paper
	std::vector<G4double> PEN_ABSL  = {9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 9.*m, 0.01 * mm, 0.01 * mm};
	std::vector<G4double> PEN_RIND = { 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23, 1.23};
	std::vector<G4double> PEN_Reflectivity = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	
	/*std::vector<G4double> PEN_ABSL = {90.0 * m, 90.0 * m,90.0 * m, 0.01 * mm, 0.01 * mm, 0.01 * mm};
    std::vector<G4double> PEN_RIND = {1.57, 1.57, 1.57, 1.57, 1.57, 1.57}; // PEN material	 https://refractiveindex.info/?shelf=organic&book=polyethylene_terephthalate&page=Zhang
	std::vector<G4double> PEN_Emission = {0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
//	std::vector<G4double> PEN_Reflectivity = { 0.2, 0.17, 0.0, 0. ,0. ,0. }; // https://link.springer.com/10.1140/epjc/s10052-022-10383-0 Fig 13
	std::vector<G4double> PEN_Reflectivity = { 0.0, 0.0, 0.0, 0. ,0. ,0. }; // the PEN 'is not there' for optical light - all reflection and absorption happens at the ESR
*/
	auto MPT_PEN = new G4MaterialPropertiesTable();
	MPT_PEN->AddProperty("RINDEX", VisEnergies, PEN_RIND); /*** See paper https://doi.org/10.1140/epjc/s10052-021-09870-7 **/
	MPT_PEN->AddProperty("WLSABSLENGTH", VisEnergies, PEN_ABSL);
	MPT_PEN->AddProperty("WLSCOMPONENT", VisEnergies, PEN_Emission);
	MPT_PEN->AddConstProperty("WLSTIMECONSTANT", 0.5 * ns);	
    MPT_PEN->AddProperty("REFLECTIVITY", VisEnergies, PEN_Reflectivity);
    
    PEN->SetMaterialPropertiesTable(MPT_PEN);
	//PEN->GetIonisation()->SetBirksConstant(0.126 * mm / MeV);
	// Define optical properties for the PEN surface
	G4OpticalSurface* PENSurface = new G4OpticalSurface("PENSurface");
	PENSurface->SetType(dielectric_dielectric);
	PENSurface->SetFinish(polished);
	PENSurface->SetModel(unified);
	
	// Define optical properties for the ESR surface
	G4OpticalSurface* aluminumSurface = new G4OpticalSurface("AluminumSurface");
	aluminumSurface->SetType(dielectric_metal);
	aluminumSurface->SetFinish(ground);
	aluminumSurface->SetModel(unified);



	G4MaterialPropertiesTable* MPTAl = new G4MaterialPropertiesTable();
	std::vector<G4double> reflectivity_ESR = { 0.97,  0.96995, 0.96991, 0.969871, 0.969831, 0.968698, 0.966512, 0.965172, 0.967064, 0.967153, 0.964894, 0.962946, 0.962193, 0.96144, 0.959131, 0.958071, 0.958071, 0.955947, 0.951363, 0.95086, 0.948428, 0.945126, 0.942498, 0.937896, 0.929001, 0.913459, 0.887445, 0.819497, 71.4474, 47.7311, 23.6371, 9.26904, 3.23342, 1.07795, 0.353766, 0.115495, 0.0376418, 0.0122612, 0.00399315, 0.00130039, 0.0 , 0.0};
	std::vector<G4double> ESR_ABSL = {0.001*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.01*cm, 0.001*cm, 0.001*cm};
	/*std::vector<G4double>  reflectivity_ESR = { ESRreflectivity, ESRreflectivity, ESRreflectivity, ESRreflectivity ,0. ,0. }; // Reduced because much of the PEN spectrum is in a lower reflectivity region, see https://doi.org/10.1140/epjc/s10052-021-09870-7
	std::vector<G4double> ESR_ABSL = { 0.01 * mm,  0.01 * mm, 0.01 * mm, 0.01 * mm, 0.01 * mm, 0.01 * mm};
	*/
	MPTAl->AddProperty("REFLECTIVITY", VisEnergies, reflectivity_ESR);
	MPTAl->AddProperty("ABSLENGTH", VisEnergies, ESR_ABSL);
	aluminumSurface->SetMaterialPropertiesTable(MPTAl);

	//Photocathode
	std::vector<G4double> cath_ABSL = {0.001 * m, 0.001 * m,0.001 * m, 0.001 * mm, 0.001 * mm, 0.001 * mm};
	std::vector<G4double>  reflectivity_pmt = { 0., 0., 0., 0. ,0. ,0. }; //reflect nothing, absorb everything
	std::vector<G4double> cath_RIND = {1.57, 1.57, 1.57, 1.57, 1.57, 1.57};
	auto photocath_mt = new G4MaterialPropertiesTable();
	photocath_mt->AddProperty("RINDEX", photonEnergy, cath_RIND);
	photocath_mt->AddProperty("ABSLENGTH", photonEnergy, cath_ABSL);
	photocath_mt->AddProperty("REFLECTIVITY", photonEnergy, reflectivity_pmt);
	lead->SetMaterialPropertiesTable(photocath_mt);

	
	//Build the geometry
	G4double PENThickness = 0.5 * mm; // thicker than in reality, or we get in trouble with step sizes; should not matter much
	
	// Define the world volume
	G4double worldSize = 3.0 * m;
	G4Box* solidWorld = new G4Box("World", worldSize / 2, worldSize / 2, worldSize / 2);
	G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air, "World");
	physWorld = new G4PVPlacement(0, G4ThreeVector(0,0,0), logicWorld, "World", 0, false, 0);


	// Dimensions for the LAr hexagonal column
	G4double outerRadiusLAr = 861.7/2. * mm; // what Geant4 calls the "outer Radius" is actually the 
											 // radius of the circle inside the polygon, based on tests. 
	G4double hzLAr = 1015.0 * mm / 2.0; // Half-height of the hexagon

	// Define the LAr hexagonal column
	G4Polyhedra* solidHexColumnLAr = new G4Polyhedra("HexColumnLAr", 0.0 * deg, 360.0 * deg, 6, 2,
													 new G4double[2]{ -hzLAr, hzLAr },
													 new G4double[2]{ 0, 0 },
													 new G4double[2]{ outerRadiusLAr, outerRadiusLAr });
	

	G4LogicalVolume* logicHexColumnLAr = new G4LogicalVolume(solidHexColumnLAr, LAr, "HexColumnLAr");


	// Define the PEN hexagonal shell
	G4double outerRadiusPEN = outerRadiusLAr + PENThickness;
	G4double hzPEN          = hzLAr + PENThickness;
	G4Polyhedra* solidHexShellPEN = new G4Polyhedra("HexShellPEN", 0.0 * deg, 360.0 * deg, 6, 2,
													new G4double[2]{ -hzPEN, hzPEN },
													new G4double[2]{ 0, 0 },
													new G4double[2]{ outerRadiusPEN, outerRadiusPEN });

	logicHexShellPEN = new G4LogicalVolume(solidHexShellPEN, PEN, "HexShellPEN");

	// Define the aluminum shell - this is actually the ESR foil
	G4double outerRadiusAl = outerRadiusPEN + 1.0 * mm;
	G4double hzAl         = hzPEN + 1*mm;
	G4Polyhedra* solidHexShellAl = new G4Polyhedra("HexShellAl", 0.0 * deg, 360.0 * deg, 6, 2,
												   new G4double[2]{ -hzAl, hzAl },
												   new G4double[2]{ 0, 0 },
												   new G4double[2]{ outerRadiusAl, outerRadiusAl });

	G4LogicalVolume* logicHexShellAl = new G4LogicalVolume(solidHexShellAl, aluminum, "HexShellAl");
	
	// Define the gap in the foil between the walls and the lid
	G4double hzGap = 4*cm;
	G4double outerRadiusGap = outerRadiusPEN;
	G4double innerRadiusGap = outerRadiusPEN - 1.*mm;
	G4Polyhedra* solidHexGap = new G4Polyhedra("HexShellGap", 0.0 * deg, 360.0 * deg, 6, 2,
													new G4double[2]{ -hzGap, hzGap },
													new G4double[2]{ innerRadiusGap, innerRadiusGap },
													new G4double[2]{ outerRadiusGap, outerRadiusGap });

	G4LogicalVolume* logicHexShellGap = new G4LogicalVolume(solidHexGap, lead, "HexShellGap");	// we use lead here just because it is defined to absorb all photons, which is what we want the gap to do

	// Define the vis PMT cathode just as a cylinder
	G4double PMTCathRadius = 5.08 * cm; // 4 inch diameter opening in the PEN foil; 2inch radius = 5.08 cm		
	G4double PMTCathHeight = 2.0 * cm;
	G4double PMTCathHalfHeight = PMTCathHeight / 2.0;

	G4Tubs* solidPMTCath = new G4Tubs("PMTCath", 0.0, PMTCathRadius, PMTCathHalfHeight, 0.0 , 360.0 );
	logicPMTCath = new G4LogicalVolume(solidPMTCath, lead, "PMTCath");

	// Define the VUV PMT cathode (this will be a dead area, since the PMT was broken)
	G4double VUVPMTCathRadius = 5.08 * cm; // 4 inch diameter opening in the PEN foil; 2inch radius = 5.08 cm		
	G4double VUVPMTCathHeight = 2.0 * cm;
	G4double VUVPMTCathHalfHeight = PMTCathHeight / 2.0;

	G4Tubs* solidVUVPMTCath = new G4Tubs("PMTVUVCath", 0.0, VUVPMTCathRadius, VUVPMTCathHalfHeight, 0.0 , 360.0 );
	G4LogicalVolume* logicVUVPMTCath = new G4LogicalVolume(solidVUVPMTCath, lead, "PMTVUVCath");


	//Define the source holder; this is a cylinder, with a hollow cylinder on top
	G4Tubs* solidSourceHolderBase = new G4Tubs("solidSourceHolderBase", 0.0, 1.5*cm, sourceHolderHalfZHeight, 0.0 , 360.0 );
	G4Tubs* solidSourceHolderRim = new G4Tubs("solidSourceHolderRim", 1.3*cm, 1.5*cm, 0.3*cm, 0.0 , 360.0 );
	G4UnionSolid *solidSourceHolder = new G4UnionSolid("SourceHolder", solidSourceHolderBase, solidSourceHolderRim, nullptr, G4ThreeVector(0., 0., sourceHolderHalfZHeight) );
	G4LogicalVolume* logicSourceHolder = new G4LogicalVolume(solidSourceHolder, lead, "SourceHolder");	


	// Place the aluminum, that is ESR, hexagonal shell in the world volume
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicHexShellAl, "HexShellAl", logicWorld, false, 0);
	// Place the PEN hexagonal shell inside the aluminum hexagonal shell
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicHexShellPEN, "HexShellPEN", logicHexShellAl, false, 0);
	
	// Place the LAr hexagonal column inside the PEN hexagonal shell
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicHexColumnLAr, "HexColumnLAr", logicHexShellPEN, false, 0);
	
	// Place the gap/absorber just below the top
	G4double gapz = hzPEN - hzGap/2.;
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,gapz), logicHexShellGap, "HexShellGap", logicHexShellPEN, false, 0);

	// Place the PMT; place 12.5cm to the centre of face of the PEN hexagon
	// Calculate the position of the PMTCath
	G4double PMTCathPositionZ = hzLAr - (PMTCathHalfHeight - 1.0 * cm);	
	new G4PVPlacement(nullptr, G4ThreeVector(12.5*cm, 0, PMTCathPositionZ), logicPMTCath, "PMTCath", logicHexColumnLAr, false, 0);

	// Place the VUV PMT; place -13.5cm to the centre of face of the PEN hexagon in x, and approximately +3cm in y based on engineering drwaing
	// Put at same Z position as the other PMT
	new G4PVPlacement(nullptr, G4ThreeVector(-13.5*cm, 3.*cm, PMTCathPositionZ), logicVUVPMTCath, "PMTVUVCath", logicHexColumnLAr, false, 0);


	// Place the source holder
	new G4PVPlacement(nullptr,sourceHolderPosition, logicSourceHolder, "SourceHolder", logicHexColumnLAr, false, 0);
	//new G4PVPlacement(nullptr,G4ThreeVector(sourceHolderPosition.x(),sourceHolderPosition.y(),sourceHolderPosition.z()+sourceHolderHalfZHeight), logicSourceHolderRim, "SourceHolderRim", logicHexColumnLAr, false, 0);


	// Attach the optical surfaces to the volumes
   new G4LogicalSkinSurface("AluminumSurface", logicHexShellAl, aluminumSurface); // this is the PEN
 //  new G4LogicalSkinSurface("photocath_surf", logicPMTCath, photocath_opsurf); // this is the photocathode of the PM
	

	G4double PMTArea = PMTCathRadius/cm *PMTCathRadius/cm * 3.1415;
	G4cout << "*_*______*******_______*********_______*********"<< G4endl;									 
	G4cout << "Fractional area covered by photo sensor: " << PMTArea/(solidHexColumnLAr->GetSurfaceArea()/cm2) << G4endl;
	G4cout << "Sensor area: " << PMTArea << G4endl;
	G4cout << "Hexagon column area: " << (solidHexColumnLAr->GetSurfaceArea()/cm2) << " for outer radius of " << outerRadiusLAr<<G4endl;
	G4cout << "*_*______*******_______*********_______*********"<< G4endl;
	
	
	
	// Visualization attributes
	G4VisAttributes* hexVisAttrLAr = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // Green
	hexVisAttrLAr->SetVisibility(0);
	logicHexColumnLAr->SetVisAttributes(hexVisAttrLAr);

	G4VisAttributes* hexVisAttrPEN = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red
	hexVisAttrPEN->SetVisibility(0);
	logicHexShellPEN->SetVisAttributes(hexVisAttrPEN);
	

	G4VisAttributes* hexVisAttrESR = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)); // gray
	logicHexShellAl->SetVisAttributes(hexVisAttrESR);

	G4VisAttributes* tubeVisAttr = new G4VisAttributes(G4Colour(0.91, 0.65, 0.22)); // Yellow
	logicPMTCath->SetVisAttributes(tubeVisAttr);	

	G4VisAttributes* tubeVUVVisAttr = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1)); // dark grey
	logicVUVPMTCath->SetVisAttributes(tubeVUVVisAttr);		
	
	G4VisAttributes* sourceVisAttr = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red
	logicSourceHolder->SetVisAttributes(sourceVisAttr);	
	
	G4VisAttributes* hexVisGap = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1)); // dark grey
	hexVisGap->SetVisibility(1);
	logicHexShellGap->SetVisAttributes(hexVisGap);	
	
	
	//
	//always return the physical World
	//
	return physWorld;
}


void DetectorConstruction::SetSourceHolderPositionXYZ(G4ThreeVector pos) {
	sourceHolderPosition = pos;
    if (G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::RMType::masterRM) {
		G4RunManager::GetRunManager()->ReinitializeGeometry();
		//G4VisManager::GetConcreteInstance()->GeometryHasChanged();
	}
}
void DetectorConstruction::SetSourcePositionByAngleAndZ(G4double angle, G4double z) {
	if (abs(angle) > 60) { 
	G4cout << "************** WARNING *******************" << G4endl;
		G4cout << "DetectorConstruction warning: Placing source outside active volume. Valid angles must be between -60 and +60 degree"<< G4endl;
	}
	// This calculation assumes that the source arm is placed in one of the two corners of
	// the hexagon that are furthest away from the PMT. Angle 0deg makes the arm point toward
	// the center of the volume (60 deg away from each of the two adjacent hexagon faces)
	G4double armlength = 42.4*cm; // the length of the manipulator arm as measured by Kevin
	G4double degToRad = 3.14159/180.;
	G4double x = armlength * cos( (60-angle)*degToRad)  - armlength/2.;
	G4double y = armlength *( sin((60-angle)*degToRad) - sqrt(3.)/2. );
	sourceHolderPosition = G4ThreeVector(x, y, z);
	G4cout << "************** INFO *******************" << G4endl;
	G4cout << "DetectorConstruction info: Placing source at " << x/cm << ", " << y/cm << ", " << z/cm << G4endl;	
    if (G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::RMType::masterRM) {
		G4RunManager::GetRunManager()->ReinitializeGeometry();
		//G4VisManager::GetInstance()->GeometryHasChanged();
	}
}

void DetectorConstruction::SetESRreflectivity(G4double ref) {
	ESRreflectivity=ref;
 }

    

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
