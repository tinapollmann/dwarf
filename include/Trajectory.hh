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
/// \file optical//include/Trajectory.hh
/// \brief Definition of the Trajectory class
//
#ifndef Trajectory_h
#define Trajectory_h 1

#include "G4Allocator.hh"
#include "G4Track.hh"
#include "G4Trajectory.hh"

class G4Polyline;
class G4ParticleDefinition;
namespace Dwarf
{
class Trajectory : public G4Trajectory
{
  public:
    Trajectory() = default;
    Trajectory(const G4Track* aTrack);
    Trajectory(Trajectory&);
    ~Trajectory() = default;

    void DrawTrajectory() const override;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void SetDrawTrajectory(G4bool b) { fDrawit = b; }
    void WLS() { fWls = true; }
    void SetForceDrawTrajectory(G4bool b) { fForceDraw = b; }
    void SetForceNoDrawTrajectory(G4bool b) { fForceNoDraw = b; }

  private:
    G4bool fWls = false;
    G4bool fDrawit = false;
    G4bool fForceNoDraw = false;
    G4bool fForceDraw = false;
    G4ParticleDefinition* fParticleDefinition = nullptr;
};

extern G4ThreadLocal G4Allocator<Trajectory>* TrajectoryAllocator;

inline void* Trajectory::operator new(size_t)
{
  if (!TrajectoryAllocator) TrajectoryAllocator = new G4Allocator<Trajectory>;
  return (void*)TrajectoryAllocator->MallocSingle();
}

inline void Trajectory::operator delete(void* aTrajectory)
{
  TrajectoryAllocator->FreeSingle((Trajectory*)aTrajectory);
}
} // end namespace
#endif
