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
/// \file optical/Dwarf/src/DwarfStackingAction.cc
/// \brief Implementation of the StackingAction class
//
//
#include "StackingAction.hh"

#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace Dwarf
{
StackingAction::StackingAction(EventAction* ea) : fEventAction(ea) {

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack)
{
  // 
  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    if (aTrack->GetParentID() == 1) {  // particle is scintillation photon from parent alpha decay
      if (aTrack->GetCreatorProcess()->GetProcessName() == "Scintillation") { // make sure it is indeed a scintillation photon
      	fEventAction->IncPhotonCount_Scint();
      	}
	  }
	else if (aTrack->GetParentID() > 1) { // Any parent ID >1 indicates a wavelength shifted photon
		fEventAction->IncPhotonCount_Vis(); 
		//G4cout << aTrack->GetKineticEnergy()/eV << G4endl;
      	// G4AnalysisManager::Instance()->FillH1(4, 1239.847/(aTrack->GetKineticEnergy()/eV));
		}
	}

  return fUrgent;
}
}