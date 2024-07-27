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
/// \file Dwarf/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the Dwarf::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include <cmath>

namespace Dwarf
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
	G4int n_particle = 1;
	fParticleGun  = new G4ParticleGun(n_particle);

	// default particle kinematic
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	G4String particleName;

	G4ParticleDefinition* particle
		= particleTable->FindParticle(particleName="alpha");
		fParticleGun->SetParticleDefinition(particle);
	
	for (int ipos = 0; ipos < 10; ipos++) {
		fPositions.push_back(G4ThreeVector(0.*cm, 0.*cm, 45.*cm - (ipos*10)*cm));
	}

    		
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	//this function is called at the begining of each event
	//
	const auto detConstruction = static_cast<const DetectorConstruction*>(
		  G4RunManager::GetRunManager()->GetUserDetectorConstruction());		

	G4double holderZHalfLength = detConstruction->GetSourceHolderHalfZHeight();
	G4ThreeVector holderpos = detConstruction->GetSourceHolderPosition();
	
	G4ThreeVector sourcepos;
	// we have a special position here - if the holder position is at x=-100 (outside the
	// world volume), we simulate events uniformly across z
	// 
	// this is for the muon simulation, but we shoot alphas for simplicity, since
	// we really just need to produce VUV photons, but want to re-use the infrastructiure
	// already created for the alphas
	
	if (holderpos.z() < -48.9*cm) {
		G4cout << "********************" << G4endl;
		G4cout << "********************" << G4endl;
		G4cout << "********* PrimaryActionGenerator ***********" << G4endl;
		G4cout << "**** In muon track mode ******" << G4endl;
		G4cout << "********************" << G4endl;
		G4cout << "********************" << G4endl;										
		sourcepos = fPositions[anEvent->GetEventID()%10];
	}
	else {
	sourcepos = G4ThreeVector(holderpos.x(),holderpos.y(),holderpos.z()+holderZHalfLength);
	}

	fParticleGun->SetParticlePosition(sourcepos);

	fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
	fParticleGun->SetParticleEnergy(1.*MeV);
	fParticleGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

} // close Dwarf namespace
