#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h" // Added for IP address assignment
#include "ns3/epc-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-stack-helper.h"

using namespace ns3;


int main (int argc, char *argv[])
{
  CommandLine cmd;
  int macroCells_N = 9;
  int microCells_N = 9;
  int UE_N = 50;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  /*19.2.14. Evolved Packet Core (EPC)*/
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm"); // disable automatic handover

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);


  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

                                      
  /*END EPC*/

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
  NodeContainer allEnbNodes;
  allEnbNodes.Add(macroNodes);
  allEnbNodes.Add(microNodes);

  enbDevices = lteHelper->InstallEnbDevice(allEnbNodes);
  ueDevices = lteHelper->InstallUeDevice(ueNodes);

  /*Handover*/
  
  // Assign IP addresses to UEs
  // InternetStackHelper internet;
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
  lteHelper->AttachToClosestEnb (ueDevices, enbDevices);
  

  Ptr<MobilityModel> pgw_loc = CreateObject<ConstantPositionMobilityModel>();
  pgw_loc->SetPosition(Vector(20.0, 30.0, 0.0));
  pgw->AggregateObject(pgw_loc);	
  
  Ptr<Node> sgw = epcHelper->GetSgwNode ();
  Ptr<MobilityModel> sgw_loc = CreateObject<ConstantPositionMobilityModel>();
  sgw_loc->SetPosition(Vector(50.0, 50.0, 0.0));
  sgw->AggregateObject(sgw_loc);
  
  
  Ptr<MobilityModel> remoteHost_loc = CreateObject<ConstantPositionMobilityModel>();
  remoteHost_loc->SetPosition(Vector(45.0, 20.0, 0.0));
  remoteHost->AggregateObject(remoteHost_loc);

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

  anim.UpdateNodeDescription(pgw,"pgw");
  anim.UpdateNodeColor(pgw,255,0,0);
  anim.UpdateNodeDescription(epcHelper->GetSgwNode(),"sgw");
  anim.UpdateNodeColor(epcHelper->GetSgwNode(),255,0,0);
  anim.UpdateNodeDescription(remoteHost,"remoteHost");
  anim.UpdateNodeColor(remoteHost,255,0,0);


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