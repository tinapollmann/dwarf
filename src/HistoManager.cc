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
/// \file optical/Dwarf/src/HistoManager.cc
/// \brief Implementation of the DwarfHistoManager class
//
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "HistoManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DwarfHistoManager::DwarfHistoManager() : fFileName("Dwarf")
{
  Book();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DwarfHistoManager::Book()
{
  // Create or get analysis manager
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");
  analysisManager->SetFileName(fFileName);
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetActivation(true);  // enable inactivation of histograms

  // Define histogram indices, titles

  // Default values (to be reset via /analysis/h1/set command)
  G4int nbins = 100;
  G4double vmin = 0.;
  G4double vmax = 100.;
  // 0
  analysisManager->CreateH1("fraction", "PMT hits / Scintillation photons", 1000, 0, 0.1);
	// 1
  analysisManager->CreateH1("vuvlength", "VUV track length [cm]", 100, 0, 140);
	//2
  analysisManager->CreateH1("vislength", "Vis track length [cm]", 200, 0, 800);
  // 3
  analysisManager->CreateH1("absorbed_holder", "fraction absorbed in holder", 10, 0.3, 0.7);
  //4
  analysisManager->CreateH1("WLSphotonspectrum", "energy", 300,300,600);
  //5
  analysisManager->CreateH1("VisStepLength", "Step lenght for vis photons [cm]", 200,0,200);


  // Create all histograms as activated, except photon spectrum
  for (G4int i = 0; i < analysisManager->GetNofH1s(); ++i) {
    analysisManager->SetH1Activation(i, true);
  }
  analysisManager->SetH1Activation(4 ,false);
  
	analysisManager->CreateNtuple("info", "info"); 
	analysisManager->CreateNtupleDColumn("sourcex"); //0
	analysisManager->CreateNtupleDColumn("sourcey"); //1
	analysisManager->CreateNtupleDColumn("sourcez"); //2
	analysisManager->CreateNtupleDColumn("ESRReflectivity"); //2
	analysisManager->CreateNtupleDColumn("LArVUVAbsLength"); //2
	analysisManager->FinishNtuple();
	//analysisManager->SetH1Activation(0, false);

}
