//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file Dwarf/include/DetectorConstruction.hh
/// \brief Definition of the Dwarf::DetectorConstruction class

#ifndef DwarfDetectorConstruction_h
#define DwarfDetectorConstruction_h 1

#include "DetectorMessenger.hh"

#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4SystemOfUnits.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;

namespace Dwarf
{

/// Detector construction class to define materials and geometry.

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction() ;
    ~DetectorConstruction() override;
	//void UpdateGeometry() ;    
    G4VPhysicalVolume* Construct() override;
    
    void SetDefaults();
 G4LogicalVolume* GetScoringVolume() const {return logicPMTCath;}    
	G4double GetSourceHolderHalfZHeight() const { return sourceHolderHalfZHeight; }
	void SetLArVUVAbsl(G4double absl) { LArVUVAbsl = absl*cm; }
	G4double GetLArVUVAbsl() const { return LArVUVAbsl;}
	void SetESRreflectivity(G4double ref);
	G4double GetESRreflectivity() const { return ESRreflectivity;}
		
	void SetPhotonYield(G4double yield) {MPTLAr->AddConstProperty("SCINTILLATIONYIELD", yield / MeV);}
	
	void SetSourceHolderPositionXYZ(G4ThreeVector pos); // free placement
	void SetSourcePositionByAngleAndZ(G4double angle, G4double z); // confine source to positions available from the rotator arm
	G4ThreeVector GetSourceHolderPosition() const { return sourceHolderPosition; }

  protected:
  	G4VPhysicalVolume* physWorld;
	G4LogicalVolume* logicPMTCath;
	G4LogicalVolume* logicHexShellPEN;
	G4ThreeVector sourceHolderPosition;
	G4double sourceHolderHalfZHeight;
	G4double LArVUVAbsl;
	G4double ESRreflectivity;
	G4MaterialPropertiesTable* MPTLAr;
    DetectorMessenger* fDetectorMessenger = nullptr;
	
};

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
