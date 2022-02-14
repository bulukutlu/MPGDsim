#include <cstdlib>
#include <iostream>
#include <fstream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>

#include "Garfield/ComponentComsol.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/Random.hh"

using namespace Garfield;

int main(int argc, char * argv[]) {

  TApplication app("app", &argc, argv);
  if( argc == 3 ) {
      printf("Will simulate %s for E_drift = %s V/cm\n", argv[1], argv[2]);
  }
  else { printf("Wrond number of arguments provided"); exit(0);}

  //std::string edrift = "0";
  std::string mmgv = "440"; 
  std::string MMG = argv[1]; 
  int edrift = atoi(argv[2]);

  //Initialize the output text file
  std::ofstream outfile;
  outfile.open("../Results/"+MMG+"_DriftScan.txt", std::ios_base::app); // append instead of overwrite
  outfile << "\nMMG, Edrift, MMGdeltaV, nEvents, OutOfBounds, collectedEvents, eColl, Avalanche" << std::endl;

  // Geometry constants
  //constexpr double xyrange = 0.01588;
  constexpr double xyrange = 0.06;
  constexpr double zlow = -0.01605;
  constexpr double zhigh = 0.06;

  double collectedEvents = 0;
  double outofboundsEvents = 0;
  double eColl = 0;
  double mean_avalanche = 0;

  // Sim constants
  constexpr unsigned int nEvents = 1000;
  constexpr double source_size = 0.01;
   
  // Plotting options 
  constexpr bool plotField = false;
  constexpr bool plotDrift = false;
  constexpr bool plotMesh = true;

  // Initialize
  ComponentComsol fm;
  //fm.Initialise("mesh.mphtxt", "dielectrics.dat", "field_"+MMG+"_Eind"+edrift+"_MMG"+mmgv+".txt", "mm");
  fm.Initialise(MMG+"_mesh.mphtxt", "dielectrics.dat", MMG+"_outfile.txt", "mm");
  fm.EnableMirrorPeriodicityX();
  fm.EnableMirrorPeriodicityY();
  fm.PrintRange();

  ViewField fieldView;
  if (plotField) {
    TCanvas* cf = new TCanvas("cf", "", 700, 1000);
    fieldView.SetComponent(&fm);
    // Set the normal vector of the viewing plane (xz plane).
    fieldView.SetPlane(0, -1, 0, 0, 0.014, 0);
    // Set the plot limits in the current viewing plane.
    fieldView.SetArea(0., zlow, xyrange, zhigh);
    fieldView.SetVoltageRange(-600., 0.);
    
    cf->SetLeftMargin(0.16);
    fieldView.SetCanvas(cf);
    fieldView.PlotContour();
    cf->SaveAs("../Results/"+MMG+"/plot_field_"+MMG+"_Edrift"+edrift+"_MMG"+mmgv+".png");
  }

  // Setup the gas.
  MediumMagboltz gas;
  gas.SetComposition("ar", 90., "co2", 10.);
  gas.SetTemperature(293.15);
  gas.SetPressure(760.);
  gas.Initialise(true);  
  // Set the Penning transfer efficiency.
  constexpr double rPenning = 0.51;
  constexpr double lambdaPenning = 0.;
  gas.EnablePenningTransfer(rPenning, lambdaPenning, "ar");
  // Load the ion mobilities.
  const std::string path = std::getenv("GARFIELD_INSTALL");
  gas.LoadIonMobility(path + "/share/Garfield/Data/IonMobility_Ar+_Ar.txt");
  // Associate the gas with the corresponding field map material. 
  const unsigned int nMaterials = fm.GetNumberOfMaterials();
  for (unsigned int i = 0; i < nMaterials; ++i) {
    const double eps = fm.GetPermittivity(i);
    if (eps == 1.) fm.SetMedium(i, &gas);
  }
  fm.PrintMaterials();
 
  // Create the sensor.
  Sensor sensor;
  sensor.AddComponent(&fm);
  constexpr double edge = 0.02;
  sensor.SetArea(0-edge, 0-edge, zlow-edge,
                  xyrange+edge, xyrange+edge, zhigh+edge);

  AvalancheMicroscopic aval;
  aval.SetSensor(&sensor);

  AvalancheMC drift;
  drift.SetSensor(&sensor);
  drift.SetDistanceSteps(2.e-4);

  ViewDrift driftView;
  if (plotDrift) {
    aval.EnablePlotting(&driftView);
    drift.EnablePlotting(&driftView);
  }

  for (unsigned int i = 0; i < nEvents; ++i) { 
    std::cout << i << "/" << nEvents << "\n";
    // Randomize the initial position. 
    const double x0 = xyrange/2 + (RndmUniform()-0.5) * source_size;
    const double y0 = xyrange/2 + (RndmUniform()-0.5) * source_size;
    const double z0 = 0.05; 
    const double t0 = 0.;
    const double e0 = 0.1;
    aval.AvalancheElectron(x0, y0, z0, t0, e0, 0., 0., 0.);
    int ne = 0, ni = 0;
    aval.GetAvalancheSize(ne, ni);
    const unsigned int np = aval.GetNumberOfElectronEndpoints();
    

    std::cout << "Number of electron endpoints: " << np << "\n";
    std::cout << "Avalanche electron: " << ne << "\n";
    std::cout << "Avalanche ion: " << ni << "\n";
    if (ni) collectedEvents++;
    mean_avalanche+=ne;

    double xe1, ye1, ze1, te1, e1;
    double xe2, ye2, ze2, te2, e2;
    double xi1, yi1, zi1, ti1;
    double xi2, yi2, zi2, ti2;
    int status;
    for (unsigned int j = 0; j < np; ++j) {
      aval.GetElectronEndpoint(j, xe1, ye1, ze1, te1, e1, 
                                  xe2, ye2, ze2, te2, e2, status);
      drift.DriftIon(xe1, ye1, ze1, te1);
      drift.GetIonEndpoint(0, xi1, yi1, zi1, ti1, 
                              xi2, yi2, zi2, ti2, status);
    }
    if(!ni && ze2 > 0.005) outofboundsEvents++;
  }

  mean_avalanche = mean_avalanche / nEvents;
  eColl = collectedEvents / (nEvents-outofboundsEvents);
  outfile << MMG << ", "<< edrift << ", "<< mmgv << ", "<< nEvents << ", " << outofboundsEvents << ", " << collectedEvents << ", " << eColl << ", " << mean_avalanche << std::endl;

  if (plotDrift) {
    TCanvas* cd = new TCanvas("cd", "", 700, 1100);
    if (plotMesh) {
      ViewFEMesh* meshView = new ViewFEMesh();
      meshView->SetArea(0, 0, zlow, 
                         xyrange, xyrange, zhigh);
      meshView->SetCanvas(cd);
      meshView->SetComponent(&fm);
      // x-z projection.
      meshView->SetPlane(0, -1, 0, 0, 0.01, 0);
      meshView->SetFillMeshWithBorders();
      // Set the color of the kapton and the metal.
      meshView->SetColor(1, kWhite);
      meshView->SetColor(2, kWhite);
      meshView->SetColor(3, kWhite);
      
      meshView->SetFillColor(1, kWhite);
      meshView->SetFillColor(2, kWhite);
      meshView->SetFillColor(3, kWhite);

      meshView->EnableAxes();
      meshView->SetViewDrift(&driftView);
      meshView->Plot();
    } else {
      driftView.SetPlane(0, -1, 0, 0, 0.01, 0);
      driftView.SetArea(0, zlow, xyrange, zhigh);
      driftView.SetColourElectrons(2);
      driftView.SetColourIons(3);
      driftView.SetCanvas(cd);
      constexpr bool twod = true;
      driftView.Plot(twod);
    }
    cd->SaveAs("../Results/"+MMG+"/plot_drift_"+MMG+"_Edrift"+edrift+"_MMG"+mmgv+".png");
  }
  app.Run(kTRUE);
}
