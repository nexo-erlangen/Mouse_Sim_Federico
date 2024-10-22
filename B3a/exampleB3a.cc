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
/// \file exampleB3a.cc
/// \brief Main program of the B3a example

#include "G4Types.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4TScoreNtupleWriter.hh"

#include "Randomize.hh"

#include "B3DetectorConstruction.hh"

#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"
#include "G4LossTableManager.hh"
#include "G4UAtomicDeexcitation.hh"

#include "B3aActionInitialization.hh"
#include "B3Analysis.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = 0;
  if ( argc == 1 ) {
    ui = new G4UIExecutive(argc, argv);
  }

  // Optionally: choose a different Random engine...
  //
  // G4Random::setTheEngine(new CLHEP::MTwistEngine);

  // Construct the default run manager
  //
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager;
#else
  G4RunManager* runManager = new G4RunManager;
#endif

  // Set mandatory initialization classes
  //
  runManager->SetUserInitialization(new B3DetectorConstruction);
  //
    // Basti
  G4PhysListFactory physListFactory;
  G4VModularPhysicsList* physicsList = physListFactory.GetReferencePhysList("FTFP_BERT_EMY");
  physicsList->SetCutValue(0.001*CLHEP::mm, "gamma");
  runManager->SetUserInitialization(physicsList);


  G4VAtomDeexcitation* de = new G4UAtomicDeexcitation();
  de->SetFluo(true);
  de->SetAuger(true);
  de->SetPIXE(true);
  de->InitialiseAtomicDeexcitation();
  G4LossTableManager::Instance()->SetAtomDeexcitation(de);

  // Set user action initialization
  //
  runManager->SetUserInitialization(new B3aActionInitialization());

  // Initialize visualization
  //
  G4VisManager* visManager = new G4VisExecutive;

  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  UImanager->ApplyCommand("/process/em/fluo true");
  UImanager->ApplyCommand("/process/em/auger true");
  UImanager->ApplyCommand("/process/em/augerCascade true");
  UImanager->ApplyCommand("/process/em/pixe true");
  UImanager->ApplyCommand("/run/setCutForAGivenParticle gamma 0.0001 mm");
  //UImanager->ApplyCommand("/random/resetEngineFrom currentRun.rndm");
  UImanager->ApplyCommand("/random/setSeed 1");

  // Activate score ntuple writer
  // The Root output type (Root) is selected in B3Analysis.hh.
  // The verbose level can be also set via UI commands
  // /score/ntuple/writerVerbose level
  G4TScoreNtupleWriter<G4AnalysisManager> scoreNtupleWriter;
  scoreNtupleWriter.SetVerboseLevel(1);

  // Initialize G4 kernel
  runManager->Initialize();

  // start a run
  int numberOfEvent = 1e+06;
  runManager->BeamOn(numberOfEvent);

//  // Process macro or start UI session
//  //
//  if ( ! ui ) {
//    // batch mode
//    G4String command = "/control/execute ";
//    G4String fileName = argv[1];
//    UImanager->ApplyCommand(command+fileName);
//  }
//  else {
//    // interactive mode
//    UImanager->ApplyCommand("/control/execute init_vis.mac");
//    ui->SessionStart();
//    delete ui;
//  }


  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !

  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
