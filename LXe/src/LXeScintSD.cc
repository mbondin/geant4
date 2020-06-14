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
/// \file optical/LXe/src/LXeScintSD.cc
/// \brief Implementation of the LXeScintSD class
//
//
#include "LXeScintSD.hh"
#include "LXeScintHit.hh"
#include "LXeSteppingMessenger.hh"
#include "LXeSteppingAction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "G4ios.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeScintSD::LXeScintSD(G4String name)
  : G4VSensitiveDetector(name)
{
  fScintCollection = nullptr;
  collectionName.insert("scintCollection");
  G4int k=0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeScintSD::~LXeScintSD() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeScintSD::Initialize(G4HCofThisEvent* hitsCE){
  fScintCollection = new LXeScintHitsCollection
                      (SensitiveDetectorName,collectionName[0]);
  //A way to keep all the hits of this event in one place if needed
  static G4int hitsCID = -1;
  G4int k=0;
  if(hitsCID<0){
    hitsCID = GetCollectionID(0);
  }
  hitsCE->AddHitsCollection( hitsCID, fScintCollection );
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void LXeScintSD::Store(const G4Step* theStep,G4TouchableHistory* ){
  G4StepPoint* thePrePoint = theStep->GetPreStepPoint();
  G4TouchableHistory* theTouchable =
    (G4TouchableHistory*)(theStep->GetPreStepPoint()->GetTouchable());
  G4VPhysicalVolume* thePrePV = theTouchable->GetVolume();
  G4StepPoint* thePostPoint = theStep->GetPostStepPoint();
  G4double edep=0.0;

  LXeScintHit* scintHit = new LXeScintHit(thePrePV);
  if(theStep->GetTrack()->GetDefinition()==G4OpticalPhoton::OpticalPhotonDefinition()){
    edep= theStep->GetTotalEnergyDeposit();
  }
  else{
    G4double ebef=thePrePoint->GetKineticEnergy();
    G4double eaft=thePostPoint->GetKineticEnergy();
    edep=ebef-eaft;
  }

  scintHit->SetEdep(edep);
  G4ThreeVector pos = theStep->GetPreStepPoint()->GetPosition() + theStep->GetPostStepPoint()->GetPosition();
  pos/=2.0;
  scintHit->SetPos(pos);
  fScintCollection->insert(scintHit);
}


G4bool LXeScintSD::ProcessHits(G4Step* aStep,G4TouchableHistory* ){
  G4double edep = aStep->GetTotalEnergyDeposit();

  if(edep==0.) return false; //No edep so dont count as hit


  G4StepPoint* thePrePoint = aStep->GetPreStepPoint();
  G4TouchableHistory* theTouchable =
    (G4TouchableHistory*)(aStep->GetPreStepPoint()->GetTouchable());
  G4VPhysicalVolume* thePrePV = theTouchable->GetVolume();

  G4StepPoint* thePostPoint = aStep->GetPostStepPoint();
  /*if(aStep->GetTrack()->GetDefinition()!=G4OpticalPhoton::OpticalPhotonDefinition()){
  G4cout<<"..................."<<G4endl;
  G4cout<<"Process Name: "<<G4String(thePostPoint->GetProcessDefinedStep()->GetProcessName())<<G4endl;*/
  /*G4cout<<"Pre Ene: "<<G4double(thePrePoint->GetTotalEnergy())<<G4endl;
  G4cout<<"Post Ene: "<<G4double(thePostPoint->GetTotalEnergy())<<G4endl;

  G4cout << "Energy: " << edep << G4endl;
  G4double edep1 = G4double(thePrePoint->GetTotalEnergy())-G4double(thePostPoint->GetTotalEnergy());
  G4cout<<"...........SCINT.............."<<G4endl;
  if(edep1>0.){G4cout<<"EDEP SCINT: "<<edep1<<G4endl;}
  G4cout<<"..................."<<G4endl;
  if(aStep->GetTrack()->GetParentID()==0){G4cout<<"Primary"<<G4endl;}*/
//}

  //Get the average position of the hit
  G4ThreeVector pos = thePrePoint->GetPosition() + thePostPoint->GetPosition();
  pos/=2.;




  //if(aStep->GetTrack()->GetDefinition()!=G4OpticalPhoton::OpticalPhotonDefinition()){
    /*if(aStep->GetTrack()->GetParentID()==0 && k==0){
      G4cout<<"PRIMARY!"<<G4endl;
      k=1;
    }


  G4cout<<"...........Scint.............."<<G4endl;
  G4cout<<"Process: "<<G4String(thePostPoint->GetProcessDefinedStep()->GetProcessName())<<G4endl;
  //if(G4String(thePostPoint->GetProcessDefinedStep()->GetProcessName())=="hIoni" ){//}&& aStep->GetTrack()->GetParentID()==0){

    G4cout<<"EBEF: "<<G4double(thePrePoint->GetKineticEnergy())<<G4endl;
    G4cout<<"EAFT: "<<G4double(thePostPoint->GetTotalEnergy())<<G4endl;
    G4cout<<"EDEP: "<<edep<<G4endl;
    G4cout<<"E Diff: "<<G4double(thePrePoint->GetTotalEnergy())-G4double(thePostPoint->GetTotalEnergy())<<G4endl;
    G4cout<<"........................."<<G4endl;
    scintHit->Setitr(1);*/

  //}

  //}


  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeScintSD::EndOfEvent(G4HCofThisEvent* ) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeScintSD::clear() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeScintSD::DrawAll() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeScintSD::PrintAll() {}
