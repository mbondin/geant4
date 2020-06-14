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
/// \file optical/LXe/src/LXeSteppingAction.cc
/// \brief Implementation of the LXeSteppingAction class
//
//
#include "LXeSteppingAction.hh"
#include "LXeEventAction.hh"
#include "LXeTrackingAction.hh"
#include "LXeTrajectory.hh"
#include "LXePMTSD.hh"
#include "LXeUserTrackInformation.hh"
#include "LXeSteppingMessenger.hh"
#include "LXeScintSD.hh"

#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4EventManager.hh"
#include "G4ProcessManager.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeSteppingAction::LXeSteppingAction(LXeEventAction* ea)
  : fOneStepPrimaries(false),
    fEventAction(ea)
{
  fSteppingMessenger = new LXeSteppingMessenger(this);
  rate=0;
  fExpectedNextStatus = Undefined;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeSteppingAction::~LXeSteppingAction() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeSteppingAction::UserSteppingAction(const G4Step * theStep){

  G4Track* theTrack = theStep->GetTrack();
  G4double edep = 0.0;
  G4double ebef=0.0;
  G4double eaft=0.0;

  if ( theTrack->GetCurrentStepNumber() == 1 ) fExpectedNextStatus = Undefined;

  LXeUserTrackInformation* trackInformation
    =(LXeUserTrackInformation*)theTrack->GetUserInformation();

  G4StepPoint* thePrePoint = theStep->GetPreStepPoint();
  G4VPhysicalVolume* thePrePV = thePrePoint->GetPhysicalVolume();

  G4StepPoint* thePostPoint = theStep->GetPostStepPoint();
  G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();


  G4OpBoundaryProcessStatus boundaryStatus=Undefined;
  static G4ThreadLocal G4OpBoundaryProcess* boundary = nullptr;

  /*if(theTrack->GetParentID()==0){G4cout<<"Par Def: "<<theTrack->GetDefinition()<<G4endl;}
  else if(theTrack->GetDefinition()==G4OpticalPhoton::OpticalPhotonDefinition()){}
  else if(theTrack->GetDefinition()==G4Electron::ElectronDefinition()){G4cout<<"Electron "<<G4endl;}
  else if(theTrack->GetDefinition()==G4Proton::ProtonDefinition()){G4cout<<"Proton "<<G4endl;}
  else if(theTrack->GetDefinition()==G4Neutron::NeutronDefinition()){G4cout<<"Neutron "<<G4endl;}
  else{G4cout<<"Some other!"<<G4endl;}*/
  //find the boundary process only once
  if(!boundary){
    G4ProcessManager* pm
      = theStep->GetTrack()->GetDefinition()->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    //G4cout<<"N of Processes: "<<nprocesses<<G4endl;
    G4ProcessVector* pv = pm->GetProcessList();
    G4int i;
    for( i=0;i<nprocesses;i++){
      //G4cout<<"Process Name: "<<(*pv)[i]->GetProcessName()<<G4endl;
      if((*pv)[i]->GetProcessName()=="OpBoundary"){
        boundary = (G4OpBoundaryProcess*)(*pv)[i];
        break;
      }
    }
  }

  if(theTrack->GetParentID()==0){
    //This is a primary track

    if(edep>0.0){
    /*LXeScintHit* scintHit = new LXeScintHit((G4VPhysicalVolume*)(theStep->GetPreStepPoint()->GetTouchable()->GetVolume()));

    scintHit->SetEdep(edep);
    G4ThreeVector pos = theStep->GetPreStepPoint()->GetPosition() + theStep->GetPostStepPoint()->GetPosition();
    pos/=2.0;
    scintHit->SetPos(pos);*/

    G4TrackVector* fSecondary=fpSteppingManager->GetfSecondary();
    G4int tN2ndariesTot = fpSteppingManager->GetfN2ndariesAtRestDoIt()
      + fpSteppingManager->GetfN2ndariesAlongStepDoIt()
      + fpSteppingManager->GetfN2ndariesPostStepDoIt();

    //If we havent already found the conversion position and there were
    //secondaries generated, then search for it
    //G4cout << "Prim: Num of 2: " << tN2ndariesTot << G4endl;
    //G4cout << "ITER of 2: " << G4int((*fSecondary).size()-tN2ndariesTot) << G4endl;

    /*G4cout<<"PRIM EBef : "<<ebef<<G4endl;
    G4cout<<"PRIM EAft : "<<eaft<<G4endl;
    G4cout<<"PRIM Edep : "<<edep<<G4endl;*/
    if(!fEventAction->IsConvPosSet() && tN2ndariesTot>0 ){

      for(size_t lp1=(*fSecondary).size()-tN2ndariesTot;
          lp1<(*fSecondary).size(); lp1++){
        const G4VProcess* creator=(*fSecondary)[lp1]->GetCreatorProcess();
        if(creator && (*fSecondary)[lp1]->GetDefinition()!=G4OpticalPhoton::OpticalPhotonDefinition()){
          G4String creatorName=creator->GetProcessName();
          //G4cout<<"Creator Name: "<<creatorName<<G4endl;

          if(creatorName=="phot"||creatorName=="compt"||creatorName=="conv"||creatorName=="hadElastic"){
            //since this is happening before the secondary is being tracked
            //the Vertex position has not been set yet(set in initial step)
            fEventAction->SetConvPos((*fSecondary)[lp1]->GetPosition());
          }
        }
      }
    }

    if(fOneStepPrimaries&&thePrePV->GetName()=="scintillator")
      theTrack->SetTrackStatus(fStopAndKill);
  }
}
  else if(theTrack->GetDefinition()==G4OpticalPhoton::OpticalPhotonDefinition()){
    //G4cout<<"Par Def1: "<<theTrack->GetDefinition()<<G4endl;
    edep = theStep->GetTotalEnergyDeposit();
    if(edep>0.0){
      //G4cout<<"E Photon: "<<edep<<G4endl;
      G4SDManager* SDman = G4SDManager::GetSDMpointer();
      G4String sdName="/LXeDet/scintSD";
      LXeScintSD* scintSD = (LXeScintSD*)SDman->FindSensitiveDetector(sdName);
      if(scintSD){scintSD->Store(theStep, nullptr);}
    }
  }
  else{
    //G4TrackVector* fSecondary=fpSteppingManager->GetfSecondary();
    G4int tN2ndariesTot = fpSteppingManager->GetfN2ndariesAtRestDoIt()
      + fpSteppingManager->GetfN2ndariesAlongStepDoIt()
      + fpSteppingManager->GetfN2ndariesPostStepDoIt();

    //G4cout<<"Par Def1: "<<theTrack->GetDefinition()<<G4endl;
    ebef = theStep->GetPreStepPoint()->GetKineticEnergy();
    eaft = theStep->GetPostStepPoint()->GetKineticEnergy();
    edep = ebef-eaft;
    /*G4cout<<"EBef : "<<ebef<<G4endl;
    G4cout<<"EAft : "<<eaft<<G4endl;
    G4cout<<"Edep : "<<edep<<G4endl;*/
    if(tN2ndariesTot>0 && edep>0.0){
      /*const G4VProcess* creator=(*fSecondary)[0]->GetCreatorProcess();
      G4String creatorName=creator->GetProcessName();*/

      /*G4cout<<"...........STEP.............."<<G4endl;
      G4cout<<"Process Name: "<<creatorName<<G4endl;
      G4cout<<"N 2nd: "<<tN2ndariesTot<<G4endl;*/

      G4SDManager* SDman = G4SDManager::GetSDMpointer();
      G4String sdName="/LXeDet/scintSD";
      LXeScintSD* scintSD = (LXeScintSD*)SDman->FindSensitiveDetector(sdName);
      if(scintSD){scintSD->Store(theStep, nullptr);}
    }
    /*else if(edep>0.0){

      G4SDManager* SDman = G4SDManager::GetSDMpointer();
      G4String sdName="/LXeDet/scintSD";
      LXeScintSD* scintSD = (LXeScintSD*)SDman->FindSensitiveDetector(sdName);
      if(scintSD){scintSD->Store(theStep, nullptr);}
    }*/
    else{}
  }



  if(!thePostPV){//out of world
    fExpectedNextStatus=Undefined;
    return;
  }

  G4ParticleDefinition* particleType = theTrack->GetDefinition();
  if(particleType==G4OpticalPhoton::OpticalPhotonDefinition()){
    //Optical photon only
    //G4cout<<"Post PV: "<<thePostPV->GetName()<<G4endl;

    if(thePrePV->GetName()=="Slab")
      //force drawing of photons in WLS slab
      trackInformation->SetForceDrawTrajectory(true);
    else if(thePostPV->GetName()=="expHall")
      //Kill photons entering expHall from something other than Slab
      theTrack->SetTrackStatus(fStopAndKill);

    //Was the photon absorbed by the absorption process
    if(thePostPoint->GetProcessDefinedStep()->GetProcessName()
       =="OpAbsorption"){
      //fEventAction->IncAbsorption();
      trackInformation->AddTrackStatusFlag(absorbed);
    }

    boundaryStatus=boundary->GetStatus();

    //Check to see if the partcile was actually at a boundary
    //Otherwise the boundary status may not be valid
    //Prior to Geant4.6.0-p1 this would not have been enough to check
    if(thePostPoint->GetStepStatus()==fGeomBoundary){
      if(fExpectedNextStatus==StepTooSmall){
        if(boundaryStatus!=StepTooSmall){
          G4ExceptionDescription ed;
          ed << "LXeSteppingAction::UserSteppingAction(): "
                << "No reallocation step after reflection!"
                << G4endl;
          G4Exception("LXeSteppingAction::UserSteppingAction()", "LXeExpl01",
          FatalException,ed,
          "Something is wrong with the surface normal or geometry");
        }
      }
      fExpectedNextStatus=Undefined;
      switch(boundaryStatus){
      case Absorption:
        trackInformation->AddTrackStatusFlag(boundaryAbsorbed);
        //fEventAction->IncBoundaryAbsorption();
        break;
      case Detection: //Note, this assumes that the volume causing detection
                      //is the photocathode because it is the only one with
                      //non-zero efficiency
        {
        //Triger sensitive detector manually since photon is
        //absorbed but status was Detection
        G4SDManager* SDman = G4SDManager::GetSDMpointer();
        G4String sdName="/LXeDet/pmtSD";
        LXePMTSD* pmtSD = (LXePMTSD*)SDman->FindSensitiveDetector(sdName);
        if(pmtSD)pmtSD->ProcessHits_constStep(theStep, nullptr);
        theTrack->SetTrackStatus(fStopAndKill);
        trackInformation->AddTrackStatusFlag(hitPMT);

        break;
        }
      case FresnelReflection:
      case TotalInternalReflection:
      case LambertianReflection:
      case LobeReflection:
      case SpikeReflection:
      case BackScattering:
        trackInformation->IncReflections();
        fExpectedNextStatus=StepTooSmall;
        break;
      default:
        break;
      }
      /*if(thePostPV->GetName()=="sphere")
        trackInformation->AddTrackStatusFlag(hitSphere);*/
    }
  }
}
