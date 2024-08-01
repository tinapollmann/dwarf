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
/// \file Dwarf/src/SteppingAction.cc
/// \brief Implementation of the Dwarf::SteppingAction class

#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "G4DynamicParticle.hh"
#include "G4CrossSectionDataStore.hh"
#include "G4HadronicProcess.hh"
#include "G4UnitsTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4VisManager.hh"
#include "G4Polyline.hh"
namespace Dwarf
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction)
: fEventAction(eventAction)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{

	if (!fScoringVolume) {
		const auto detConstruction = static_cast<const DetectorConstruction*>(
		  G4RunManager::GetRunManager()->GetUserDetectorConstruction());
		fScoringVolume = detConstruction->GetScoringVolume();
	}

	// get volume of the current step
	G4LogicalVolume* volume = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();


	// what particle are we tracking?
  	G4Track* theTrack = step->GetTrack();	
	int pdg_particle = theTrack->GetParticleDefinition()->GetPDGEncoding();
  const G4ParticleDefinition* part = theTrack->GetDefinition();

	
  G4StepPoint* thePrePoint = step->GetPreStepPoint();
  G4VPhysicalVolume* thePrePV = thePrePoint->GetPhysicalVolume();

  G4StepPoint* thePostPoint = step->GetPostStepPoint();
  G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();
	
  G4OpBoundaryProcessStatus boundaryStatus = Undefined;

  // find the boundary process only once
  if (nullptr == fBoundary && pdg_particle == -22) {
    G4ProcessManager* pm = part->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    G4ProcessVector* pv = pm->GetProcessList();
    for (G4int i = 0; i < nprocesses; ++i) {
      if (nullptr != (*pv)[i] && (*pv)[i]->GetProcessName() == "OpBoundary") {
        fBoundary = dynamic_cast<G4OpBoundaryProcess*>((*pv)[i]);
        break;
      }
    }
  }
  // Optical photon only
  if (pdg_particle == -22) {
  	// make distribution of step lengths for optical absorption study
  	G4double stepLength = step->GetStepLength();
	/*if (theTrack->GetParentID() > 1) { // this is a wavelength shifted photon
		G4AnalysisManager::Instance()->FillH1(5, stepLength/cm);
	}// don't do this if not necessary */
  	
    // Was the photon absorbed by the absorption process
    auto proc = thePostPoint->GetProcessDefinedStep();
    if (nullptr != proc && proc->GetProcessName() == "OpAbsorption") {
         //G4cout << "------------------------------------------------------------" << G4endl;
	     //G4cout << "absorption for track " << theTrack->GetTrackID() << "/ in volume pre " << thePrePV->GetName() << " and post "<< thePostPV->GetName() << G4endl;
	     //G4cout << "------------------------------------------------------------" << G4endl;
	    if (thePrePV->GetName() == "PMTCath" && theTrack->GetParentID() > 1) fEventAction->AddPMTHit();
	    if (thePrePV->GetName() == "SourceHolder" && theTrack->GetParentID() == 1) fEventAction->AddVUVAbsorbed();
	    else      fEventAction->IncAbsorption();
    }
    if (nullptr != fBoundary) boundaryStatus = fBoundary->GetStatus();

    if (thePostPoint->GetStepStatus() == fGeomBoundary) {
      // Check to see if the particle was actually at a boundary
      // Otherwise the boundary status may not be valid
      if (fExpectedNextStatus == StepTooSmall) {
      /*
        if (boundaryStatus != StepTooSmall) {
          G4cout << "LXeSteppingAction::UserSteppingAction(): "
                 << "trackID=" << theTrack->GetTrackID() << " parentID=" << theTrack->GetParentID() << G4endl;
                 /*<< " " << part->GetParticleName() << " E(MeV)=" << theTrack->GetKineticEnergy()
                 << "n/ at " << theTrack->GetPosition() << " prePV: " << thePrePV->GetName()
                 << " postPV: " << thePostPV->GetName() << G4endl;
          G4ExceptionDescription ed;
          ed << "LXeSteppingAction: "
             << "No reallocation step after reflection!"
             << "Something is wrong with the surface normal or geometry";
          G4Exception("LXeSteppingAction:", "LXeExpl01", JustWarning, ed, "");
          return;
        }
        */
      }
      fExpectedNextStatus = Undefined;
      switch (boundaryStatus) {
        case Absorption:
        	if (theTrack->GetParentID() == 1) fEventAction->IncBoundaryVUVAbsorption(); 
        	else fEventAction->IncBoundaryAbsorption();          
           break;
        case Detection:  // Note, this assumes that the volume causing detection
                         // is the photocathode because it is the only one with
                         // non-zero efficiency
        {
          break;
        }
        case FresnelReflection:
        case TotalInternalReflection:
        case LambertianReflection:
        case LobeReflection:
        case SpikeReflection:
        case BackScattering:
          fExpectedNextStatus = StepTooSmall;
          break;
        default:
          break;
      }
    }   
    }

 G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();

  if (pVVisManager) {

    G4ThreeVector preStepPoint = step->GetPreStepPoint()->GetPosition();
    G4ThreeVector postStepPoint = step->GetPostStepPoint()->GetPosition();

    G4Polyline polyline;
    G4Colour colour;
    if      (theTrack->GetParentID() == 1) colour = G4Colour(0.65, 0.4, 0.11); // orange for scintillation photons
    else                  colour = G4Colour(0.7, 0.7, 0.88);  // light blue for WLS photons

    G4VisAttributes attribs(colour);
    polyline.SetVisAttributes(attribs);
    polyline.push_back(preStepPoint);
    polyline.push_back(postStepPoint);

    //----- Call a drawing method for G4Polyline
    pVVisManager -> Draw(polyline);
	}
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
