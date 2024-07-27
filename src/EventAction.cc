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
/// \file Dwarf/src/EventAction.cc
/// \brief Implementation of the Dwarf::EventAction class

#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

namespace Dwarf
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction* runAction)
: fRunAction(runAction)
{
	man = G4AnalysisManager::Instance(); 

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{
  fNScint = 0.;
  fTotalHits = 0.;
  fNVis = 0.0;
  fNAbs = 0;
  fNAbsBound = 0; 
	fNVUVAbsorbed = 0;
	fNVUVBoundaryAbsorbed = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* g4ev)
{
  // accumulate statistics in run action
// G4cout << "Event " << g4ev->GetEventID() << ", total energy deposited " << fEdep/CLHEP::MeV << " MeV (" << fEdepEM << "+" << fEdepNR << ") -------------- missing code " << fMissingCode << G4endl;  
	
	
	
	float hitfraction = (float)fTotalHits/((float)(fNVis));
  	G4AnalysisManager::Instance()->FillH1(0, hitfraction);
  	G4AnalysisManager::Instance()->FillH1(3, (float)fNVUVAbsorbed/((float)fNScint));
	G4UImanager* UI = G4UImanager::GetUIpointer();
	trackingVerboseLevel = std::stoi(UI->GetCurrentStringValue("/tracking/verbose"));
  if (trackingVerboseLevel > 0 ) {

     G4cout << "------------------------------------------------------------" << G4endl;
     G4cout << "------------------------------------------------------------" << G4endl;     
     G4cout << "Scintillation photons: " << fNScint << G4endl;
     G4cout << "Visible photons: " <<fNVis << G4endl;
     G4cout << "PMT hits: " << fTotalHits << G4endl;
    G4cout <<  "Absorbed :" << fNAbs << G4endl;
     G4cout << "VUV photons Absorbed on source holder:" << fNVUVAbsorbed << G4endl;
     G4cout << "VUV photons absorbed at boundary: " << fNVUVBoundaryAbsorbed << G4endl;      
     G4cout << "Absorbed at boundary: " << fNAbsBound << G4endl;          
     G4cout << "fraction: " << hitfraction << G4endl;
     G4cout << "------------------------------------------------------------" << G4endl;
     G4cout << "------------------------------------------------------------" << G4endl;          
     }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
