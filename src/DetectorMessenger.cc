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
/// \file optical//src/DetectorMessenger.cc
/// \brief Implementation of the DetectorMessenger class
//
//
#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4Scintillation.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace Dwarf
{
DetectorMessenger::DetectorMessenger(DetectorConstruction* detector)
  : fDetector(detector)
{
  // Setup a command directory for detector controls with guidance
  fDetectorDir = new G4UIdirectory("/mydetector/");
  fDetectorDir->SetGuidance("Detector geometry control");

  fVolumesDir = new G4UIdirectory("/mydetector/volumes/");
  fVolumesDir->SetGuidance("Enable/disable volumes");

    
  // Various commands for modifying detector geometry
  fSourcePosCmd = new G4UIcmdWith3VectorAndUnit("/mydetector/SourcePosXYZ", this);
  fSourcePosCmd->SetGuidance("Set the position of the source.");
  fSourcePosCmd->SetParameterName("scint_x", "scint_y", "scint_z", false);
  fSourcePosCmd->SetDefaultUnit("cm");
  fSourcePosCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  fSourcePosCmd->SetToBeBroadcasted(false);

  fSourcePosCmd2 = new G4UIcmdWith3Vector("/mydetector/SourcePosAlphaZ", this);
  fSourcePosCmd2->SetGuidance("Set the position of the source, give angle and Z, last number is ignored.");
  fSourcePosCmd2->SetParameterName("scint_alpha", "scint_z", "ignored", false);
  //fSourcePosCmd->SetDefaultUnit("cm");
  fSourcePosCmd2->AvailableForStates(G4State_PreInit, G4State_Idle);
  fSourcePosCmd2->SetToBeBroadcasted(false);
  

  fESRReflectivityCmd = new G4UIcmdWithADouble("/mydetector/ESRreflectivity", this);
  fESRReflectivityCmd->SetGuidance("Set the reflectivity of the ESR.");
  fESRReflectivityCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  fESRReflectivityCmd->SetToBeBroadcasted(false);

  fMainScintYieldCmd = new G4UIcmdWithADouble("/mydetector/MainScintYield", this);
  fMainScintYieldCmd->SetGuidance("Set scinitillation yield of main volume.");
  fMainScintYieldCmd->SetGuidance("Specified in photons/MeV");
  fMainScintYieldCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  fMainScintYieldCmd->SetToBeBroadcasted(false);
  
  fLArAbsLengthCmd = new G4UIcmdWithADouble("/mydetector/LArAbsLength", this);
  fLArAbsLengthCmd->SetGuidance("Set VUV absorption length.");
  fLArAbsLengthCmd->SetGuidance("Specified in cm");
  fLArAbsLengthCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  fLArAbsLengthCmd->SetToBeBroadcasted(false);
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::~DetectorMessenger()
{
  delete fSourcePosCmd;
  delete fESRReflectivityCmd;
  delete fMainScintYieldCmd;
  delete fLArAbsLengthCmd;
  delete fSourcePosCmd2;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if (command == fSourcePosCmd) {
    fDetector->SetSourceHolderPositionXYZ(fSourcePosCmd->GetNew3VectorValue(newValue));
  }
  if (command == fSourcePosCmd2) {
  	G4ThreeVector vec = fSourcePosCmd2->GetNew3VectorValue(newValue);
    fDetector->SetSourcePositionByAngleAndZ(vec.x(), vec.y()*cm);
  }  
  else if (command == fESRReflectivityCmd) {
    fDetector->SetESRreflectivity(fESRReflectivityCmd->GetNewDoubleValue(newValue));
  }
  else if (command == fMainScintYieldCmd) {
    fDetector->SetPhotonYield(fMainScintYieldCmd->GetNewDoubleValue(newValue));
  }
  else if (command == fLArAbsLengthCmd) {
    fDetector->SetLArVUVAbsl(fLArAbsLengthCmd->GetNewDoubleValue(newValue));
  }
}
}