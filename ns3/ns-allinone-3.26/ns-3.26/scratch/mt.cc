/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/propagation-module.h"
#include "ns3/flow-monitor-module.h"

// Default Network Topology
//
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   
//                   point-to-point  |    |    
//                                   ======
//                                LAN 10.1.2.0

using namespace ns3;

//NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int myRand(int min, int max){

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min));
  x->SetAttribute ("Max", DoubleValue (max));

  int value = x->GetValue ();

  return value;
}

int main (int argc, char *argv[]){
  bool verbose = false;
  uint32_t nCsma = 1;
  float time = 10.0;
  uint32_t nWifi = 10;
  bool tracing = false;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("time", "Number of \"extra\" CSMA nodes/devices", time);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.Parse (argc,argv);
  
  uint32_t porcentagem = (nWifi * 0.2); // CALCULO DA PORCENTAGEM - NESSE CASO ESTOU UTILIZANDO 20%
  printf("Minha porcentagem: %d\n", porcentagem);//Printf apenas para ver se está com parametro correto na porcentagem

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  for (size_t i = 0; i < nWifi; i++) {
     uint32_t NodeX = myRand(0, 100); //O 100 é um tamanho bom!
     uint32_t NodeY = myRand(0, 100);
     Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
     //positionAlloc = CreateObject<ListPositionAllocator> ();
     positionAlloc->Add (Vector (NodeX, NodeY, 0.0));
     mobility.SetPositionAllocator (positionAlloc);
     mobility.Install (wifiStaNodes.Get(i));
  }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (time)); //pega o tempo da variavel TIME
//UDP
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("DataRate", StringValue("1500kb/s"));;
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));

  uint32_t nnode;

  for ( uint32_t x=1 ; x <= porcentagem ; x++) {
	  nnode = (nWifi - x);
	  printf("%d\n", nnode);
	  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nnode));
	  clientApps.Start (Seconds (2.0));
	  clientApps.Stop (Seconds (time));
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();//protocolo IP

  Simulator::Stop (Seconds (time));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }
//###### flow monitor ###########
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds(time));
  Simulator::Run ();

  flowMonitor->SerializeToXmlFile("MariaNS3.xml", true, true); //GERA um xml com o nome MariaNS3 na pasta ns-3.26

  Simulator::Destroy ();
  return 0;
}
