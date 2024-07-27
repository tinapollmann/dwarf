
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
	//SetSourcePositionByAngleAndZ(0., 0.); // places source in the center of the volume
	sourceHolderPosition = G4ThreeVector(0., 0., 0.*cm);

	sourceHolderHalfZHeight = 1.5*cm;
	ESRreflectivity = 0.95;
	LArVUVAbsl = 1700.*cm;
	G4cout << "0" << G4endl;
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
	std::vector<G4double> photonEnergy = {2.5*eV, 3. * eV, 3.5 *eV, 9.1 * eV, 9.7 * eV, 10. * eV }; 
	std::vector<G4double> scintillationSpectrumLAr = { 0.0, 0.0 , 0.0 ,0.0 , 1.0, 0.}; // peak at 128 nm
	std::vector<G4double> lar_RIND = { 1.23, 1.23, 1.23, 1.36, 1.36, 1.36};
	std::vector<G4double> lar_ABSL = {2000. * m, 2000. * m, 2000. * m, LArVUVAbsl, LArVUVAbsl, LArVUVAbsl}; // from https://iopscience.iop.org/article/10.1088/1748-0221/17/01/C01012
			// Note: abs length for visible is essentially infinite here; we eat visible photons once they hit the ESR
			// and assume the ESR absorptoin probability is much higher the the prob they are absorbed in the LAr on their path.
	std::vector<G4double> lar_RAYL = {1.7 * m, 1.7 * m, 1.7 * m, 1.7 * m, 1.7 * m, 1.7 * m}; //

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
	//LAr->GetIonisation()->SetBirksConstant(0.126 * mm / MeV);
	  
	// WLS properties of the PEN
	std::vector<G4double> PEN_ABSL = {90.0 * m, 90.0 * m,90.0 * m, 0.01 * mm, 0.01 * mm, 0.01 * mm};
    std::vector<G4double> PEN_RIND = {1.57, 1.57, 1.57, 1.57, 1.57, 1.57}; // PEN material	 https://refractiveindex.info/?shelf=organic&book=polyethylene_terephthalate&page=Zhang
	std::vector<G4double> PEN_Emission = {0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
//	std::vector<G4double> PEN_Reflectivity = { 0.2, 0.17, 0.0, 0. ,0. ,0. }; // https://link.springer.com/10.1140/epjc/s10052-022-10383-0 Fig 13
	std::vector<G4double> PEN_Reflectivity = { 0.0, 0.0, 0.0, 0. ,0. ,0. }; // the PEN 'is not there' for optical light - all reflection and absorption happens at the ESR

	auto MPT_PEN = new G4MaterialPropertiesTable();
	MPT_PEN->AddProperty("RINDEX", photonEnergy, lar_RIND); /*** See paper https://doi.org/10.1140/epjc/s10052-021-09870-7 **/
	MPT_PEN->AddProperty("WLSABSLENGTH", photonEnergy, PEN_ABSL);
	MPT_PEN->AddProperty("WLSCOMPONENT", photonEnergy, PEN_Emission);
	MPT_PEN->AddConstProperty("WLSTIMECONSTANT", 0.5 * ns);	
    MPT_PEN->AddProperty("REFLECTIVITY", photonEnergy, PEN_Reflectivity);
    
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
	//std::vector<G4double>  reflectivity_ESR = { 0.97, 0.97, 0.97, 0.97 ,0. ,0. }; // 97% reflectivity across the visible spectrum https://resources.perkinelmer.com/lab-solutions/resources/docs/far_measurement-of-enhanced-specular-reflector-films-using-lambda-1050-and-ura-accessory-012190_01.pdf
	std::vector<G4double>  reflectivity_ESR = { ESRreflectivity, ESRreflectivity, ESRreflectivity, ESRreflectivity ,0. ,0. }; // Reduced because much of the PEN spectrum is in a lower reflectivity region, see https://doi.org/10.1140/epjc/s10052-021-09870-7
	//std::vector<G4double>  reflectivity_ESR = { 0., 0., 0., 0. ,0. ,0. }; //  no reflectivity, for test of sensor coverage
	std::vector<G4double> ESR_ABSL = { 0.01 * mm,  0.01 * mm, 0.01 * mm, 0.01 * mm, 0.01 * mm, 0.01 * mm};
	MPTAl->AddProperty("REFLECTIVITY", photonEnergy, reflectivity_ESR);
	MPTAl->AddProperty("ABSLENGTH", photonEnergy, ESR_ABSL);
	aluminumSurface->SetMaterialPropertiesTable(MPTAl);

	//Photocathode
	std::vector<G4double> cath_ABSL = {0.001 * m, 0.001 * m,0.001 * m, 0.001 * mm, 0.001 * mm, 0.001 * mm};
	std::vector<G4double>  reflectivity_pmt = { 0., 0., 0., 0. ,0. ,0. }; //reflect nothing, absorb everything
	
	auto photocath_mt = new G4MaterialPropertiesTable();
	photocath_mt->AddProperty("RINDEX", photonEnergy, PEN_RIND);
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
	//G4double innerRadiusLAr = 860.0/2. * mm;
	//G4double outerRadiusLAr = 995.0/2. * mm;
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

	// Define the aluminum shell
	G4double outerRadiusAl = outerRadiusPEN + 1.0 * mm;
	G4double hzAl         = hzPEN + 1*mm;
	G4Polyhedra* solidHexShellAl = new G4Polyhedra("HexShellAl", 0.0 * deg, 360.0 * deg, 6, 2,
												   new G4double[2]{ -hzAl, hzAl },
												   new G4double[2]{ 0, 0 },
												   new G4double[2]{ outerRadiusAl, outerRadiusAl });

	G4LogicalVolume* logicHexShellAl = new G4LogicalVolume(solidHexShellAl, aluminum, "HexShellAl");


	// Define the PMT cathode just as a cylinder
	G4double PMTCathRadius = 5.08 * cm; // 4 inch diameter opening in the PEN foil; 2inch radius = 5.08 cm		
	G4double PMTCathHeight = 2.0 * cm;
	G4double PMTCathHalfHeight = PMTCathHeight / 2.0;

	G4Tubs* solidPMTCath = new G4Tubs("PMTCath", 0.0, PMTCathRadius, PMTCathHalfHeight, 0.0 , 360.0 );
	logicPMTCath = new G4LogicalVolume(solidPMTCath, lead, "PMTCath");

	G4double PMTArea = PMTCathRadius/cm *PMTCathRadius/cm * 3.1415;
		G4cout << "*_*______*******_______*********_______*********"<< G4endl;									 
	G4cout << "Fractional area covered by photo sensor: " << PMTArea/(solidHexColumnLAr->GetSurfaceArea()/cm2) << G4endl;
	G4cout << "Sensor area: " << PMTArea << G4endl;
	G4cout << "Hexagon column area: " << (solidHexColumnLAr->GetSurfaceArea()/cm2) << " for outer radius of " << outerRadiusLAr<<G4endl;
	G4cout << "*_*______*******_______*********_______*********"<< G4endl;
	
		
	
	//Source holder; this is a cylinder, with a hollow cylinder on top
	G4Tubs* solidSourceHolderBase = new G4Tubs("solidSourceHolderBase", 0.0, 1.5*cm, sourceHolderHalfZHeight, 0.0 , 360.0 );
	G4Tubs* solidSourceHolderRim = new G4Tubs("solidSourceHolderRim", 1.3*cm, 1.5*cm, 0.3*cm, 0.0 , 360.0 );
	
	G4UnionSolid *solidSourceHolder = new G4UnionSolid("SourceHolder", solidSourceHolderBase, solidSourceHolderRim, nullptr, G4ThreeVector(0., 0., sourceHolderHalfZHeight) );
	G4LogicalVolume* logicSourceHolder = new G4LogicalVolume(solidSourceHolder, lead, "SourceHolder");	

	//G4LogicalVolume* logicSourceHolderRim = new G4LogicalVolume(solidSourceHolderRim, lead, "SourceHolderRim");	


	// Place the aluminum, that is ESR, hexagonal shell in the world volume
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicHexShellAl, "HexShellAl", logicWorld, false, 0);
	// Place the PEN hexagonal shell inside the aluminum hexagonal shell
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicHexShellPEN, "HexShellPEN", logicHexShellAl, false, 0);
	
	// Place the LAr hexagonal column inside the PEN hexagonal shell
	new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicHexColumnLAr, "HexColumnLAr", logicHexShellPEN, false, 0);

	// Place the PMT; place 12.5cm to the centre of face of the PEN hexagon
	// Calculate the position of the PMTCath
	G4double PMTCathPositionZ = hzLAr - (PMTCathHalfHeight - 1.0 * cm);	
	new G4PVPlacement(nullptr, G4ThreeVector(12.5*cm, 0, PMTCathPositionZ), logicPMTCath, "PMTCath", logicHexColumnLAr, false, 0);

	// Place the source holder
	new G4PVPlacement(nullptr,sourceHolderPosition, logicSourceHolder, "SourceHolder", logicHexColumnLAr, false, 0);
	//new G4PVPlacement(nullptr,G4ThreeVector(sourceHolderPosition.x(),sourceHolderPosition.y(),sourceHolderPosition.z()+sourceHolderHalfZHeight), logicSourceHolderRim, "SourceHolderRim", logicHexColumnLAr, false, 0);


	// Attach the optical surfaces to the volumes
   new G4LogicalSkinSurface("AluminumSurface", logicHexShellAl, aluminumSurface); // this is the PEN
 //  new G4LogicalSkinSurface("photocath_surf", logicPMTCath, photocath_opsurf); // this is the photocathode of the PM
	

	// Visualization attributes
	G4VisAttributes* hexVisAttrLAr = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // Green
	hexVisAttrLAr->SetVisibility(0);
	logicHexColumnLAr->SetVisAttributes(hexVisAttrLAr);

	G4VisAttributes* hexVisAttrPEN = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red
	hexVisAttrPEN->SetVisibility(0);
	logicHexShellPEN->SetVisAttributes(hexVisAttrPEN);
	

	G4VisAttributes* hexVisAttrESR = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)); // gray
	logicHexShellAl->SetVisAttributes(hexVisAttrESR);

	G4VisAttributes* tubeVisAttr = new G4VisAttributes(G4Colour(0.93, 0.73, 0.25)); // Yellow
	logicPMTCath->SetVisAttributes(tubeVisAttr);	
	
	G4VisAttributes* sourceVisAttr = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red
	logicSourceHolder->SetVisAttributes(sourceVisAttr);	
	
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
	G4double armlength = 50.*cm; // the length of the manipulator arm
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
