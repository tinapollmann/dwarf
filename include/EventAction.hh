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
/// \file Dwarf/include/EventAction.hh
/// \brief Definition of the Dwarf::EventAction class

#ifndef DwarfEventAction_h
#define DwarfEventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include "G4AnalysisManager.hh"

namespace Dwarf
{

class RunAction;

/// Event action class

class EventAction : public G4UserEventAction
{
  public:
    EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    void IncPhotonCount_Scint() { fNScint++; }
    void IncPhotonCount_Vis() {fNVis++; }
    void IncAbsorption() {fNAbs++; }
    void IncBoundaryAbsorption() {fNAbsBound++; }
    void AddPMTHit() { fTotalHits++; }
    void AddVUVAbsorbed() { fNVUVAbsorbed++; }
    void IncBoundaryVUVAbsorption() {fNVUVBoundaryAbsorbed++; }
    G4int GetPMTHits() { return fTotalHits; }

  private:
    RunAction* fRunAction = nullptr;
    G4int   fNScint = 0.;
    G4int	fNVis = 0;
    G4int	fNAbs = 0;
    G4int	fNAbsBound = 0;  
    G4int	fNVUVAbsorbed = 0; // number of VUV photons absorbed in holder
    G4int	fNVUVBoundaryAbsorbed = 0;
    G4int fTotalHits;
    
    G4AnalysisManager* man = nullptr;
    int fMissingCode = -99;
    G4int	trackingVerboseLevel;
};

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif


