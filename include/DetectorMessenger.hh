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
/// \file optical//include/DetectorMessenger.hh
/// \brief Definition of the DetectorMessenger class
//
//
#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"


class G4UIcmdWithABool;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAnInteger;
class G4UIcmdWith3VectorAndUnit;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWith3Vector;


namespace Dwarf {
class DetectorConstruction;


class DetectorMessenger : public G4UImessenger
{
  public:
    DetectorMessenger(DetectorConstruction*);
    ~DetectorMessenger() override;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:
    DetectorConstruction* fDetector = nullptr;
    G4UIdirectory* fDetectorDir = nullptr;
    G4UIdirectory* fVolumesDir = nullptr;
    G4UIcmdWithADouble* fESRReflectivityCmd = nullptr;
    G4UIcmdWith3VectorAndUnit* fSourcePosCmd = nullptr;
    G4UIcmdWithADouble* fMainScintYieldCmd = nullptr;
    G4UIcmdWithADouble* fLArAbsLengthCmd = nullptr;
    G4UIcmdWith3Vector* fSourcePosCmd2 = nullptr;


};
}
#endif
