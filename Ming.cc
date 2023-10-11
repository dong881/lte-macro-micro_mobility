#include <ns3/core-module.h>

#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // 9 macro cells
  NodeContainer macroNodes;
  macroNodes.Create(9);
  
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add (Vector(5000, 5000, 0));
  positionAlloc->Add (Vector(0, 5000, 0));
  positionAlloc->Add (Vector(-5000, 5000, 0));
  positionAlloc->Add (Vector(5000, 0, 0));
  positionAlloc->Add (Vector(0, 0, 0));
  positionAlloc->Add (Vector(-5000, 0, 0));
  positionAlloc->Add (Vector(5000, -5000, 0));
  positionAlloc->Add (Vector(0, -5000, 0));
  positionAlloc->Add (Vector(-5000, -5000, 0));

  MobilityHelper macroMobility;
  macroMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  macroMobility.SetPositionAllocator(positionAlloc);
  macroMobility.Install (macroNodes);
  
  // 30 micro cells
  NodeContainer microNodes;
  microNodes.Create(30);
  
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
  x->SetAttribute ("Min", DoubleValue (-5000));
  x->SetAttribute ("Max", DoubleValue (5000));

  Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable>();
  y->SetAttribute ("Min", DoubleValue (-5000));
  y->SetAttribute ("Max", DoubleValue (5000));
  
  Ptr<RandomRectanglePositionAllocator> microPositionAlloc = CreateObject<RandomRectanglePositionAllocator>();
  microPositionAlloc->SetX (x);
  microPositionAlloc->SetY (y);

  MobilityHelper microMobility;
  microMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  microMobility.SetPositionAllocator(microPositionAlloc);
  microMobility.Install (microNodes);

  // 50 UEs
  NodeContainer ueNodes;
  ueNodes.Create (50);
  
  MobilityHelper ueMobility;
  ueMobility.SetPositionAllocator ("ns3::UniformRandomDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "Rho", DoubleValue (5000));
  ueMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Mode", StringValue ("Time"),
                               "Time", StringValue ("2s"),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=20]"),
                               "Bounds", RectangleValue (Rectangle (-5000, 5000, -5000, 5000)));
  ueMobility.Install (ueNodes);

  // Install LTE protocol stack
  NetDeviceContainer enbDevices;
  NetDeviceContainer ueDevices;
  
  enbDevices = lteHelper->InstallEnbDevice (macroNodes);
  ueDevices = lteHelper->InstallUeDevice (ueNodes);

  // Attach UEs to eNBs
  lteHelper->Attach (ueDevices); // for UEs
  lteHelper->Attach (enbDevices.Get(0)); // for eNBs (just first eNB here)
  
  // Activate data radio bearers
  lteHelper->ActivateDataRadioBearer (ueDevices, EpsBearer (EpsBearer::GBR_CONV_VOICE));

  // Stop time
  Simulator::Stop (Seconds (20));

  // Traces
  lteHelper->EnableTraces ();

  // Animation
  AnimationInterface anim ("lte-macro-micro.xml");
  
  anim.SetMaxPktsPerTraceFile (1000000);

  // Run simulation
  Simulator::Run ();

  Simulator::Destroy ();
  
  return 0;
}