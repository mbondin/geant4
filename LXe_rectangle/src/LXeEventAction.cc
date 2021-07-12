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
/// \file optical/LXe/src/LXeEventAction.cc
/// \brief Implementation of the LXeEventAction class
//
//
#include "LXeEventAction.hh"
#include "LXeScintHit.hh"
#include "LXePMTHit.hh"
#include "LXeTrajectory.hh"
#include "LXeRun.hh"
//#include "LXeHistoManager.hh"
#include "LXeAnalysis.hh"
#include "LXeDetectorConstruction.hh"

#include "G4EventManager.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4VVisManager.hh"
#include "G4ios.hh"
#include "G4UImanager.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeEventAction::LXeEventAction(const LXeDetectorConstruction* det)
  : fScintCollID(-1),fPMTCollID(-1),fVerbose(0),
   fPMTThreshold(1),fForcedrawphotons(false),fForcenophotons(false)
{
  fEventMessenger = new LXeEventMessenger(this);

  fHitCount = 0;
  fHC1=0;
  fHC2=0;
  fhits=0;

  fPhotonCount_Scint = 0;
  fPhotonCount_Ceren = 0;
  fLCE = 0.0;
  //fAbsorptionCount = 0;
  //fBoundaryAbsorptionCount = 0;
  fTotE = 0.0;
  fTotEPM = 0.0;

  fConvPosSet = false;
  //fEdepMax = 0.0;

  //fPMTsAboveThreshold = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeEventAction::~LXeEventAction(){}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeEventAction::BeginOfEventAction(const G4Event*) {

  fHitCount = 0;
  fHC1=0;
  fHC2=0;
  fhits=0;

  fPhotonCount_Scint = 0;
  fPhotonCount_Ceren = 0;
  fLCE = 0.0;
  //fAbsorptionCount = 0;
  //fBoundaryAbsorptionCount = 0;
  fTotE = 0.0;
  fTotEPM = 0.0;


  fConvPosSet = false;
  //fEdepMax = 0.0;

  //fPMTsAboveThreshold = 0;

  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  if(fScintCollID<0)
    fScintCollID=SDman->GetCollectionID("scintCollection");
  if(fPMTCollID<0)
    fPMTCollID=SDman->GetCollectionID("pmtHitCollection");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeEventAction::EndOfEventAction(const G4Event* anEvent){

  /*G4TrajectoryContainer* trajectoryContainer=anEvent->GetTrajectoryContainer();

  G4int n_trajectories = 0;
  if (trajectoryContainer) n_trajectories = trajectoryContainer->entries();

  // extract the trajectories and draw them
  if (G4VVisManager::GetConcreteInstance()){
    for (G4int i=0; i<n_trajectories; i++){
      LXeTrajectory* trj = (LXeTrajectory*)
        ((*(anEvent->GetTrajectoryContainer()))[i]);
      if(trj->GetParticleName()=="opticalphoton"){
        trj->SetForceDrawTrajectory(fForcedrawphotons);
        trj->SetForceNoDrawTrajectory(fForcenophotons);
      }
      trj->DrawTrajectory();
    }
  }*/

  LXeScintHitsCollection* scintHC = nullptr;
  LXePMTHitsCollection* pmtHC = nullptr;
  G4HCofThisEvent* hitsCE = anEvent->GetHCofThisEvent();

  //Get the hit collections
  if(hitsCE){
    if(fScintCollID>=0) {
      scintHC = (LXeScintHitsCollection*)(hitsCE->GetHC(fScintCollID));
    }
    if(fPMTCollID>=0) {
      pmtHC = (LXePMTHitsCollection*)(hitsCE->GetHC(fPMTCollID));
    }
  }

  //Hits in scintillator
  if(scintHC){
    int n_hit = scintHC->entries();
    //G4cout<<"N_ScintHit: "<<n_hit<<G4endl;
    //G4ThreeVector  eWeightPos(0.);
    G4double edep;
    //G4double edepMax=0;


    for(int i=0;i<n_hit;i++){ //gather info on hits in scintillator
      //G4cout<<"GETEDEP: "<<(*scintHC)[i]->GetEdep()<<G4endl;
      edep=(*scintHC)[i]->GetEdep();
      fTotE += edep;

      /*eWeightPos += (*scintHC)[i]->GetPos()*edep;//calculate energy weighted pos
      if(edep>edepMax){
        edepMax=edep;//store max energy deposit
        G4ThreeVector posMax=(*scintHC)[i]->GetPos();
        fPosMax = posMax;
        fEdepMax = edep;
      }*/
    }

    if(n_hit>0){
      //G4cout<<"Tot Energy Scint: "<<fTotE <<G4endl;

    auto analysisManager = G4AnalysisManager::Instance();
    //int id= analysisManager->GetH1Id("EScinct");
    analysisManager->FillH1(0,fTotE);
  }

    /*if(fTotE == 0.){
      if(fVerbose>0)G4cout<<"No hits in the scintillator this event."<<G4endl;
    }
    else{
      //Finish calculation of energy weighted position
      eWeightPos /= fTotE;
      fEWeightPos = eWeightPos;
      if(fVerbose>0){
        G4cout << "\tEnergy weighted position of hits in LXe : "
               << eWeightPos/mm << G4endl;
      }
    }
    if(fVerbose>0){
    G4cout << "\tTotal energy deposition in scintillator : "
           << fTotE / keV << " (keV)" << G4endl;
    }*/
  }

  if(pmtHC){
    //G4ThreeVector reconPos(0.,0.,0.);
    G4int pmts=pmtHC->entries();
    //G4cout<<"Num of Hits per event: " << pmts<<G4endl;
    if(pmts>0){fhits+=1;}
    G4double edepPM;
    //Gather info from all PMTs
    for(G4int i=0;i<pmts;i++){
      edepPM=(*pmtHC)[i]->GetEdepPM();
      G4int num_pmt = (*pmtHC)[i]->GetPMTNumber();
      //G4cout<<"PMT number: "<<num_pmt<<G4endl;
      if (num_pmt==0){
        fHC1 += (*pmtHC)[i]->GetPhotonCount();
      }
      else{
        fHC2 += (*pmtHC)[i]->GetPhotonCount();
      }
      fTotEPM += edepPM;
      fHitCount += (*pmtHC)[i]->GetPhotonCount();
      //reconPos+=(*pmtHC)[i]->GetPMTPos()*(*pmtHC)[i]->GetPhotonCount();
      //G4cout<<"Photon Count per hit: " << (*pmtHC)[i]->GetPhotonCount() << G4endl;
      /*if((*pmtHC)[i]->GetPhotonCount()>=fPMTThreshold){
        fPMTsAboveThreshold++;
      }
      else{//wasnt above the threshold, turn it back off

        (*pmtHC)[i]->SetDrawit(false);
      }*/
    }
    //G4cout<<"Edep: "<<fTotEPM<<G4endl;
    //G4cout<<"PMT threshold: "<<fPMTThreshold <<G4endl;

    if (pmts>0){
    auto analysisManager = G4AnalysisManager::Instance();
    fLCE = (G4double(fHitCount*10000)/(fPhotonCount_Scint + fPhotonCount_Ceren));
    //analysisManager->FillH1(1, fHC1);
    analysisManager->FillH1(1, fLCE);
    analysisManager->FillH1(2, fHitCount);
    //G4cout<<"Hit COunt: "<<fHitCount<<G4endl;
    //G4AnalysisManager::Instance()->FillH1(2, fPMTsAboveThreshold);
  }

    /*if(fHitCount > 0) {//dont bother unless there were hits
      reconPos/=fHitCount;
      if(fVerbose>0){
        G4cout << "\tReconstructed position of hits in LXe : "
               << reconPos/mm << G4endl;
      }
      fReconPos = reconPos;
    }
    pmtHC->DrawAllHits();*/
  }

  //G4AnalysisManager::Instance()->FillH1(3, fPhotonCount_Scint);
  //G4AnalysisManager::Instance()->FillH1(4, fPhotonCount_Ceren);
  //G4AnalysisManager::Instance()->FillH1(5, fAbsorptionCount);
  //G4AnalysisManager::Instance()->FillH1(6, fBoundaryAbsorptionCount);

  /*if(fVerbose>0){
    //End of event output. later to be controlled by a verbose level
    G4cout << "\tNumber of photons that hit PMTs in this event : "
           << fHitCount << G4endl;
    G4cout << "\tNumber of PMTs above threshold("<<fPMTThreshold<<") : "
           << fPMTsAboveThreshold << G4endl;
    G4cout << "\tNumber of photons produced by scintillation in this event : "
           << fPhotonCount_Scint << G4endl;
    G4cout << "\tNumber of photons produced by cerenkov in this event : "
           << fPhotonCount_Ceren << G4endl;
    G4cout << "\tNumber of photons absorbed (OpAbsorption) in this event : "
           << fAbsorptionCount << G4endl;
    G4cout << "\tNumber of photons absorbed at boundaries (OpBoundary) in "
           << "this event : " << fBoundaryAbsorptionCount << G4endl;
    G4cout << "Unaccounted for photons in this event : "
           << (fPhotonCount_Scint + fPhotonCount_Ceren -
                fAbsorptionCount - fHitCount - fBoundaryAbsorptionCount)
           << G4endl;
  }*/

  // update the run statistics
  LXeRun* run = static_cast<LXeRun*>(
    G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  run->IncHitCount(fHitCount);
  run->IncHC1(fHC1);
  run->IncHC2(fHC2);
  run->Inchits(fhits);

  run->IncPhotonCount_Scint(fPhotonCount_Scint);
  run->IncPhotonCount_Ceren(fPhotonCount_Ceren);
  run->IncLCE(fLCE);
  run->IncEDep(fTotE);
  //run->IncAbsorption(fAbsorptionCount);
  //run->IncBoundaryAbsorption(fBoundaryAbsorptionCount);
  //run->IncHitsAboveThreshold(fPMTsAboveThreshold);
  run->IncEDepPM(fTotEPM);

  //If we have set the flag to save 'special' events, save here
  /*if(fPhotonCount_Scint + fPhotonCount_Ceren <= fDetector->GetSaveThreshold())
  {
    G4RunManager::GetRunManager()->rndmSaveThisEvent();
  }*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
