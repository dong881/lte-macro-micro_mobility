#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h" // Added for IP address assignment

using namespace ns3;


int main (int argc, char *argv[])
{
  CommandLine cmd;
  int macroCells_N = 9;
  int microCells_N = 9;
  int UE_N = 50;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  lteHelper->SetSchedulerType("ns3::FdMtFfMacScheduler");    // FD-MT scheduler 

  // macro cells
  NodeContainer macroNodes;
  macroNodes.Create(macroCells_N);

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

  // micro cells
  NodeContainer microNodes;
  microNodes.Create(microCells_N);

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

  // UEs
  NodeContainer ueNodes;
  ueNodes.Create (UE_N);

  MobilityHelper ueMobility;
  ueMobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
  "X", DoubleValue (0.0),
  "Y", DoubleValue (0.0),
  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=5000.0]"));
  ueMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  "Mode", StringValue ("Time"),
  "Time", StringValue ("2s"),
  "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1000]"),
  "Bounds", RectangleValue (Rectangle (-5000, 5000, -5000, 5000)));
  ueMobility.Install (ueNodes);

  // Install LTE protocol stack
  NetDeviceContainer enbDevices;
  NetDeviceContainer ueDevices;
  
  enbDevices = lteHelper->InstallEnbDevice(macroNodes);
  ueDevices = lteHelper->InstallUeDevice(ueNodes);

  /*Handover*/
  
  // Assign IP addresses to UEs
  InternetStackHelper internet;
  internet.Install (ueNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ueIpIface = ipv4.Assign (ueDevices);

  /*Handover END*/

  //Attach the UEs to an eNB. This will configure each UE according to the eNB configuration, and create an RRC connection between them:
  lteHelper->Attach (ueDevices, enbDevices.Get (0));

  //Activate a data radio bearer between each UE and the eNB it is attached to:
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevices, bearer);
  
  //Set the stop time
  Simulator::Stop (Seconds (20));

  /*Simulation Output*/
  ///This is needed otherwise the simulation will last forever, because (among others) the start-of-subframe event is scheduled repeatedly, and the ns-3 simulator scheduler will hence never run out of events.
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  
  // Enable animation
  AnimationInterface anim("lte-macro-micro.xml");
  anim.EnablePacketMetadata(true);

  // Set color for each node
  for (int i = 0; i < macroCells_N; ++i) {
    anim.UpdateNodeColor(macroNodes.Get(i), 0, 0, 0);  // RGB values
    anim.UpdateNodeSize(macroNodes.Get(i), 180, 180);  // Set node size
  }
  for (int i = 0; i < microCells_N; ++i) {
    anim.UpdateNodeColor(microNodes.Get(i), 255, 0, 0);  // RGB values
    anim.UpdateNodeSize(microNodes.Get(i), 130,130);  // Set node size
  }
  for (int i = 0; i < UE_N; ++i) {
    anim.UpdateNodeColor(ueNodes.Get(i), 0, 255, 0);  // RGB values
    anim.UpdateNodeSize(ueNodes.Get(i), 80,80);  // Set node size
  }

  // Display IP addresses
  for (uint32_t i = 0; i < ueNodes.GetN (); ++i) {
    Ptr<Ipv4> ipv4 = ueNodes.Get (i)->GetObject<Ipv4> ();
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();
    std::cout << "UE " << i + 1 << " IP address: " << addr << std::endl;
  }

  anim.SetMobilityPollInterval(Seconds(0.10));
  anim.SetMaxPktsPerTraceFile (100000000000);
  // anim.EnablePacketMetadata(true);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}