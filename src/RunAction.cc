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
/// \file Dwarf/src/RunAction.cc
/// \brief Implementation of the Dwarf::RunAction class

#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
// #include "Run.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "DetectorConstruction.hh"

namespace Dwarf
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction()
{
  // Book predefined histograms
  fHistoManager = new DwarfHistoManager();


}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  const auto detConstruction = static_cast<const DetectorConstruction*>(G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  // Get analysis manager
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  
  if (analysisManager->IsActive()) {
  //  G4String filename = "dwarf_" + str(detConstruction->GetSourceHolderPosition().x()/cm) + "_" ;
//    			detConstruction->GetSourceHolderPosition().y()/cm + "_" +
//    			detConstruction->GetSourceHolderPosition().x()/cm+ "_" +
 //   			detConstruction->GetESRreflectivity() +"_" +
//    			detConstruction->GetLArVUVAbsl();
    			
  //  analysisManager->SetFileName(filename.c_str());
    analysisManager->OpenFile();
    analysisManager->FillNtupleDColumn(0, detConstruction->GetSourceHolderPosition().x()/cm); 
    analysisManager->FillNtupleDColumn(1, detConstruction->GetSourceHolderPosition().y()/cm);
    analysisManager->FillNtupleDColumn(2, detConstruction->GetSourceHolderPosition().z()/cm);
    analysisManager->FillNtupleDColumn(3, detConstruction->GetESRreflectivity()); 
    analysisManager->FillNtupleDColumn(4, detConstruction->GetLArVUVAbsl()/cm); 
    analysisManager->AddNtupleRow(); 
  }
  else {G4cout << "RunAction: Error no analysis manager" << G4endl;}

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{

  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;
  // save histograms
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager->IsActive()) {
    analysisManager->Write();
    analysisManager->CloseFile();
  }
   else {G4cout << "RunAction: Error no analysis manager" << G4endl;}
  // Print
  //
 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::AddEdep(G4double edep, G4double eoriginal, int evid)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
