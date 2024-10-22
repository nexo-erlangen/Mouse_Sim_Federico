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
/// \file B3DetectorConstruction.cc
/// \brief Implementation of the B3DetectorConstruction class

#include "B3DetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSDoseDeposit.hh"
#include "G4VisAttributes.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B3DetectorConstruction::B3DetectorConstruction()
: G4VUserDetectorConstruction(),
  fCheckOverlaps(true)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B3DetectorConstruction::~B3DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


G4VPhysicalVolume* B3DetectorConstruction::Construct()
{
  // Gamma detector Parameters
  //
  G4double cryst_dX = 3.5*mm, cryst_dY = 3.5*mm, cryst_dZ = 1.*mm;
  G4int nb_cryst = 45;
  G4int nb_rings = 35;
  //
  G4double dPhi = twopi/nb_cryst, half_dPhi = 0.5*dPhi;
  G4double cosdPhi = std::cos(half_dPhi);
  G4double tandPhi = std::tan(half_dPhi);
  //
  G4double ring_R1 = 0.5*cryst_dY/tandPhi;
  G4double ring_R2 = (ring_R1+cryst_dZ)/cosdPhi;
  //
  G4double detector_dZ = nb_rings*cryst_dX;
  //
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* default_mat = nist->FindOrBuildMaterial("G4_AIR");//("G4_AIR");G4_Galactic
  G4Material* cryst_mat   = nist->FindOrBuildMaterial("G4_CADMIUM_TELLURIDE");//("G4_Si");G4_CADMIUM_TELLURIDE //G4_GALLIUM_ARSENIDE //G4_Si

  G4cout << "\nNb of crystals: " << nb_cryst
         << "\nNb of rings: " << nb_rings
         << "\nInternal radius: " << ring_R1/cm << " cm"
         << "\nDetector lenth: " << detector_dZ/cm << " cm" << G4endl;

  //
  // World
  //
  G4double world_sizeXY = 2.4*ring_R2;
  G4double world_sizeZ  = 1.2*detector_dZ;

  G4Box* solidWorld =
    new G4Box("World",                       //its name
       0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ); //its size

  G4LogicalVolume* logicWorld =
    new G4LogicalVolume(solidWorld,          //its solid
                        default_mat,         //its material
                        "World");            //its name

  G4VPhysicalVolume* physWorld =
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      fCheckOverlaps);       // checking overlaps

  //
  // ring
  //
  G4Tubs* solidRing =
    new G4Tubs("Ring", ring_R1, ring_R2, 0.5*cryst_dX, 0., twopi);

  G4LogicalVolume* logicRing =
    new G4LogicalVolume(solidRing,           //its solid
                        default_mat,         //its material
                        "Ring");             //its name

  G4LogicalVolume* logicRing_0 =
    new G4LogicalVolume(solidRing,           //its solid
                        default_mat,         //its material
                        "Ring_0");             //its name

  //
  // define crystal
  //
  G4double gap = 0.3*mm;        //a gap for wrapping
  G4double dX = cryst_dX - gap, dY = cryst_dY - gap;
  G4Box* solidCryst = new G4Box("crystal", dX/2, dY/2, cryst_dZ/2);

  G4LogicalVolume* logicCryst =
    new G4LogicalVolume(solidCryst,          //its solid
                        cryst_mat,           //its material
                        "CrystalLV");        //its name

  // place crystals within a ring
  //
  for (G4int icrys = 0; icrys < nb_cryst ; icrys++) {
    G4double phi = icrys*dPhi;
    G4RotationMatrix rotm  = G4RotationMatrix();
    rotm.rotateY(90*deg);
    rotm.rotateZ(phi);
    G4ThreeVector uz = G4ThreeVector(std::cos(phi),  std::sin(phi),0.);
    G4ThreeVector position = (ring_R1+0.5*cryst_dZ)*uz;
    G4Transform3D transform = G4Transform3D(rotm,position);
    if (icrys != 0)
    {
    //fCrystalPV =
    new G4PVPlacement(transform,             //rotation,position
                      logicCryst,            //its logical volume
                      "crystal_0",             //its name
                      logicRing_0,             //its mother  volume
                      false,                 //no boolean operation
                      icrys,                 //copy number
                      fCheckOverlaps);       // checking overlaps
    }
    new G4PVPlacement(transform,             //rotation,position
                      logicCryst,            //its logical volume
                      "crystal",             //its name
                      logicRing,             //its mother  volume
                      false,                 //no boolean operation
                      icrys,                 //copy number
                      fCheckOverlaps);       // checking overlaps
  }


  //
  // full detector
  //
  G4Tubs* solidDetector =
    new G4Tubs("Detector", ring_R1, ring_R2, 0.5*detector_dZ, 0., twopi);

  G4LogicalVolume* logicDetector =
    new G4LogicalVolume(solidDetector,       //its solid
                        default_mat,         //its material
                        "Detector");         //its name

  //
  // place rings within detector
  //
  G4double OG = -0.5*(detector_dZ + cryst_dX);
  for (G4int iring = 0; iring < nb_rings ; iring++) {
    OG += cryst_dX;
    if (iring != 17)  // 35 rings/2 = 17.5
    {
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(0,0,OG), //position
                      logicRing,             //its logical volume
                      "ring",                //its name
                      logicDetector,         //its mother  volume
                      false,                 //no boolean operation
                      iring,                 //copy number
                      fCheckOverlaps);       // checking overlaps
    }
    else
    {
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(0,0,OG), //position
                      logicRing_0,             //its logical volume
                      "ring",                //its name
                      logicDetector,         //its mother  volume
                      false,                 //no boolean operation
                      iring,                 //copy number
                      fCheckOverlaps);       // checking overlaps
    }
  }

  //
  // place detector in world
  //

  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicDetector,           //its logical volume
                    "Detector",              //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    fCheckOverlaps);         // checking overlaps


  //
  // Mouse
  //
  G4double patient_radius = 2.0*cm; // 2*cm
  G4double patient_dZ = 10.0*cm; //10*cm
  G4Material* patient_mat = nist->FindOrBuildMaterial("G4_A-150_TISSUE");              //G4_A-150_TISSUE
                                                                            //G4_BRAIN_ICRP G4_A-150_TISSUE

  G4Tubs* solidPatient =
    new G4Tubs("Patient", 0., patient_radius, 0.5*patient_dZ, 0., twopi);

  G4LogicalVolume* logicPatient =
    new G4LogicalVolume(solidPatient,        //its solid
                        patient_mat,         //its material
                        "PatientLV");        //its name

  //
  // place patient in world
  //
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicPatient,            //its logical volume
                    "Patient",               //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    fCheckOverlaps);         // checking overlaps

  auto Patient_color = new G4VisAttributes(G4Colour(0.,0.,1.0));
  Patient_color->SetVisibility(true);
  logicPatient->SetVisAttributes(Patient_color);


  //
  // Mo Solution
  //
  G4Material* Mo_mat = nist->FindOrBuildMaterial("G4_Mo");
  G4double density_Mo = Mo_mat->GetDensity();
  G4double mass_Mo = 0.1*mg;//e-01*mg;  // 1.e-03mg per mm3 minimum, 1.e-02mg per cm3
  G4double vol_Mo = mass_Mo / density_Mo;

  G4Material* Water_mat = nist->FindOrBuildMaterial("G4_A-150_TISSUE"); //G4_A-150_TISSUE //G4_WATER
  G4double density_water = Water_mat->GetDensity();

  G4double vol_sol = 1.*mm3;

  G4double vol_water = vol_sol - vol_Mo;
  G4double mass_water = density_water * vol_water;


  G4double mass_sol = mass_Mo + mass_water;
  G4double density_sol = mass_sol / vol_sol;

  G4double w_Mo = mass_Mo / (mass_Mo + mass_water); // Mo %
  G4Material* Mo_Solution_mat =
  new G4Material("Mo_Solution", // name
                 density_sol,   // density
                 2);            // nb of components
  Mo_Solution_mat->AddMaterial(Mo_mat, // material
                               w_Mo);  // fraction mass
  Mo_Solution_mat->AddMaterial(Water_mat, // material
                               1.-w_Mo);  // fraction mass

  G4double sol_dl = pow(vol_sol, (1./3.)); // Cube Volume
//  G4double xpos = G4UniformRand()*18.;
//  G4double xpos = 0.;
  G4ThreeVector sol_pos = G4ThreeVector();

  G4cout << "Mo mass: " << mass_Mo/mg << " mg" << G4endl
         << "Mo vol: " << vol_Mo/mm3 << " mm3" << G4endl
         << "Medium mass: " << mass_water/mg << " mg" << G4endl
         << "Medium vol: " << vol_water/mm3 << " mm3" << G4endl
         << "Solution mass: " << mass_sol/mg << " mg" << G4endl
         << "Solution vol: " << vol_sol/mm3 << " mm3" << G4endl
         << "Cube side: " << sol_dl/mm << " mm" << G4endl;

  G4Box* solidSol =
    new G4Box("SolutionS", sol_dl, sol_dl, sol_dl);

  G4LogicalVolume* logicSol =
    new G4LogicalVolume(solidSol,            //its solid
                        Mo_Solution_mat,      //its material
                        "SolutionLV");    //its name

  //
  // place Solution in world
  //
//  MolybdenumPV =
  new G4PVPlacement(0,                       //no rotation
                    sol_pos,         //at (0,0,0)
                    logicSol,                //its logical volume
                    "Solution",             //its name
                    logicPatient,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    fCheckOverlaps);         // checking overlaps

  auto sol_color = new G4VisAttributes(G4Colour(1.0,0.8,0.8));
  sol_color->SetVisibility(true);
  logicSol->SetVisAttributes(sol_color);



  // Visualization attributes
  //
  logicRing->SetVisAttributes (G4VisAttributes::GetInvisible());
  logicDetector->SetVisAttributes (G4VisAttributes::GetInvisible());
  logicWorld->SetVisAttributes (G4VisAttributes::GetInvisible());


  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  //always return the physical World
  //
  return physWorld;
}








//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B3DetectorConstruction::ConstructSDandField()
{
  G4SDManager::GetSDMpointer()->SetVerboseLevel(1);
  
  // declare crystal as a MultiFunctionalDetector scorer
  //  
  G4MultiFunctionalDetector* cryst = new G4MultiFunctionalDetector("crystal");
  G4SDManager::GetSDMpointer()->AddNewDetector(cryst);
  G4VPrimitiveScorer* primitiv1 = new G4PSEnergyDeposit("edep");
  cryst->RegisterPrimitive(primitiv1);
  SetSensitiveDetector("CrystalLV",cryst);
  
  // declare patient as a MultiFunctionalDetector scorer
  //  
  G4MultiFunctionalDetector* patient = new G4MultiFunctionalDetector("patient");
  G4SDManager::GetSDMpointer()->AddNewDetector(patient);
  G4VPrimitiveScorer* primitiv2 = new G4PSDoseDeposit("dose");
  patient->RegisterPrimitive(primitiv2);
  SetSensitiveDetector("PatientLV",patient);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
