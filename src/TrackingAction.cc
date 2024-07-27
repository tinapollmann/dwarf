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
/// \file optical//src/TrackingAction.cc
/// \brief Implementation of the TrackingAction class
//
//
#include "TrackingAction.hh"
#include "Trajectory.hh"
#include "DetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace Dwarf
{
void TrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
fpTrackingManager->SetTrajectory(new Trajectory(aTrack));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void TrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
   auto trajectory = (Trajectory*)fpTrackingManager->GimmeTrajectory();

  // Count what process generated the optical photons
  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    // particle is optical photon from parent alpha
    if (aTrack->GetParentID() == 1) {
      // particle is secondary
      if (aTrack->GetCreatorProcess()->GetProcessName() == "Scintillation") {
      	G4AnalysisManager::Instance()->FillH1(1, aTrack->GetTrackLength()/cm);
      	trajectory->SetDrawTrajectory(true);
      	}
	  }
	else if (aTrack->GetParentID() > 1) {
		G4AnalysisManager::Instance()->FillH1(2, aTrack->GetTrackLength()/cm);
		      trajectory->WLS();
      trajectory->SetDrawTrajectory(true);
		}
	}
}
}//end namespace