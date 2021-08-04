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
/// \file optical/LXe/src/LXeRun.cc
/// \brief Implementation of the LXeRun class
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "LXeRun.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeRun::LXeRun() : G4Run()
{
  fHitCount                = fHitCount2                = 0;
  fHC1  = fHC2 = 0;
  fhits = 0;

  fPhotonCount_Scint       = fPhotonCount_Scint2       = 0;
  fPhotonCount_Ceren       = fPhotonCount_Ceren2       = 0;
  fLCE = 0.0;
  //fAbsorptionCount         = fAbsorptionCount2         = 0;
  //fBoundaryAbsorptionCount = fBoundaryAbsorptionCount2 = 0;
  //fPMTsAboveThreshold      = fPMTsAboveThreshold2      = 0;

  fTotE = fTotE2 = 0.0;
  fTotEPM = fTotEPM2 = 0.0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeRun::~LXeRun()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeRun::Merge(const G4Run* run)
{
  const LXeRun* localRun = static_cast<const LXeRun*>(run);

  fHitCount                 += localRun->fHitCount;
  fHitCount2                += localRun->fHitCount2;
  fHC1                      += localRun->fHC1;
  fHC2                      += localRun->fHC2;
  fhits                     += localRun->fhits;
  /*fPMTsAboveThreshold       += localRun->fPMTsAboveThreshold;
  fPMTsAboveThreshold2      += localRun->fPMTsAboveThreshold2;*/
  fPhotonCount_Scint        += localRun->fPhotonCount_Scint;
  fPhotonCount_Scint2       += localRun->fPhotonCount_Scint2;
  fPhotonCount_Ceren        += localRun->fPhotonCount_Ceren;
  fPhotonCount_Ceren2       += localRun->fPhotonCount_Ceren2;
  fLCE                      += localRun->fLCE;
  /*fAbsorptionCount          += localRun->fAbsorptionCount;
  fAbsorptionCount2         += localRun->fAbsorptionCount2;
  fBoundaryAbsorptionCount  += localRun->fBoundaryAbsorptionCount;
  fBoundaryAbsorptionCount2 += localRun->fBoundaryAbsorptionCount2;*/
  fTotE                     += localRun->fTotE;
  fTotE2                    += localRun->fTotE2;
  fTotEPM                   += localRun->fTotEPM;
  fTotEPM2                  += localRun->fTotEPM2;


  G4Run::Merge(run);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeRun::EndOfRun()
{
  G4cout << "\n ======================== run summary ======================\n";

  G4int prec = G4cout.precision();
  G4int num_GPS = 1;
  G4int n_evt = numberOfEvent;
  G4double N_event = n_evt*num_GPS;
  G4cout << "The run had " << n_evt*num_GPS << " events." << G4endl;
  G4cout << "The run had " << fhits << " hit events." <<G4endl;
  G4cout << "hits/events:" << 100*fhits/N_event << "%" <<G4endl;
  G4cout<<"No of photons produced: "<<fPhotonCount_Scint+fPhotonCount_Ceren<<G4endl;
  G4cout << "Tot Hits on SiPM: " << fHC1 << G4endl;

  /*G4cout.precision(4);
  G4double hits = G4double(fHitCount)/fhits;
  G4double hits2 = G4double(fHitCount2)/fhits;
  G4double rms_hits = hits2 - hits*hits;
  if (rms_hits > 0.) rms_hits = std::sqrt(rms_hits/fhits);
  else rms_hits = 0.;
  G4cout << "Number of hits per event:\t " << hits << " +- " << rms_hits
         << G4endl;

  G4double hitsAbove = G4double(fPMTsAboveThreshold)/fhits;
  G4double hitsAbove2 = G4double(fPMTsAboveThreshold2)/fhits;
  G4double rms_hitsAbove = hitsAbove2 - hitsAbove*hitsAbove;
  if (rms_hitsAbove > 0.) rms_hitsAbove = std::sqrt(rms_hitsAbove/fhits);
  else rms_hitsAbove = 0.;

  G4cout << "Number of hits per event above threshold:\t " << hitsAbove
         << " +- " << rms_hitsAbove << G4endl;

  G4double scint = G4double(fPhotonCount_Scint)/fhits;
  G4double scint2 = G4double(fPhotonCount_Scint2)/fhits;

  G4double rms_scint = scint2 - scint*scint;

  if (rms_scint > 0.) rms_scint = std::sqrt(rms_scint/fhits);
  else rms_scint = 0.;

  G4cout << "Number of scintillation photons per event :\t " << scint << " +- "
         << rms_scint << G4endl;


  G4double ceren = G4double(fPhotonCount_Ceren)/fhits;
  G4double ceren2 = G4double(fPhotonCount_Ceren2)/fhits;
  G4double rms_ceren = ceren2 - ceren*ceren;
  if (rms_ceren > 0.) rms_ceren = std::sqrt(rms_ceren/fhits);
  else rms_ceren = 0.;

  G4cout << "Number of Cerenkov photons per event:\t " << ceren << " +- "
         << rms_ceren << G4endl;

  G4double absorb = G4double(fAbsorptionCount)/fhits;
  G4double absorb2 = G4double(fAbsorptionCount2)/fhits;
  G4double rms_absorb = absorb2 - absorb*absorb;
  if (rms_absorb > 0.) rms_absorb = std::sqrt(rms_absorb/fhits);
  else rms_absorb = 0.;

  G4cout << "Number of absorbed photons per event :\t " << absorb << " +- "
         << rms_absorb << G4endl;

  G4double bdry = G4double(fBoundaryAbsorptionCount)/fhits;
  G4double bdry2 = G4double(fBoundaryAbsorptionCount2)/fhits;
  G4double rms_bdry = bdry2 - bdry*bdry;
  if (rms_bdry > 0.) rms_bdry = std::sqrt(rms_bdry/fhits);
  else rms_bdry = 0.;

  G4cout << "Number of photons absorbed at boundary per event:\t " << bdry
         << " +- " << rms_bdry << G4endl;
  //G4cout << "Number of unaccounted for photons: " << G4endl;

  G4double en = fTotE/fhits;
  G4double en2 = fTotE2/fhits;
  G4double rms_en = en2 - en*en;
  if (rms_en > 0.) rms_en = std::sqrt(rms_en/fhits);
  else rms_en = 0.;

  G4cout << "Total energy deposition in scintillator per event:\t " << en/keV
         << " +- " << rms_en/keV << " keV." << G4endl;

 G4double enpm = fTotEPM/fhits;
 G4double enpm2 = fTotEPM2/fhits;
 G4double rms_enpm = enpm2 - enpm*enpm;
 if (rms_enpm > 0.) rms_enpm = std::sqrt(rms_enpm/fhits);
 else rms_enpm = 0.;

G4cout << "Total energy deposition in SiPM per event:\t " << enpm/keV
       << " +- " << rms_enpm/keV << " keV." << G4endl;*/


// G4cout << "Tot Hits on back SiPM: " << fHC2 << G4endl;
// G4cout << "Tot Hits: " << fHitCount << G4endl;
// G4cout << "hit/photon: "<<(G4double(fHitCount*100)/(fPhotonCount_Scint+fPhotonCount_Ceren)) <<G4endl;
// G4cout << "Light Collection Efficiency: " << (fLCE)/fhits << G4endl;







  G4cout << G4endl;
  G4cout.precision(prec);
}
