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
#include <iostream>
#include <sstream>
#include <math.h>

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
using namespace std;

//NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int myRand(int min, int max){

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min));
  x->SetAttribute ("Max", DoubleValue (max));

  int value = x->GetValue ();

  return value;
}

void funcao_flow(FlowMonitorHelper &flowmon, Ptr<FlowMonitor> &monitor, Ipv4InterfaceContainer &devicesIP, Ipv4InterfaceContainer &p2pdeviceIP,  uint32_t nWifi, uint32_t porcentagem){

  double throughput = 0;
  double delay = 0;
  double sumThroughput = 0.0;
  double meanThroughput = 0.0;
  double meanDelayPackets = 0.0;
  double meanLostPackets = 0.0;
  int count = 0;
  FILE *f;

  monitor->CheckForLostPackets();
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());

  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin (); i != stats.end (); ++i, count++){
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    if (t.destinationAddress == p2pdeviceIP.GetAddress(0)){
      throughput = (i->second.txBytes * 8) / ((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds());



//if(printLog){
        cout << "Flowid              = " << i->first << endl;
        cout << "Source Address      = " << t.sourceAddress << endl;
        cout << "Destination Address = " << t.destinationAddress << endl;
        cout << "First Tx Packet     = " << i->second.timeFirstTxPacket.GetSeconds() << endl;
        cout << "First Rx Packet     = " << i->second.timeFirstRxPacket.GetSeconds() << endl;
        cout << "Last Tx Packet      = " << i->second.timeLastTxPacket.GetSeconds() << endl;
        cout << "Last Rx Packet      = " << i->second.timeLastRxPacket.GetSeconds() << endl;
        cout << "Tx Packets          = " << i->second.txPackets << endl;
        cout << "RX Packets          = " << i->second.rxPackets << endl;
        cout << "Lost Packets        = " << i->second.lostPackets << endl;
        cout << "Tx Bytes            = " << i->second.txBytes << endl;
        cout << "RX bytes            = " << i->second.rxBytes << endl;
        cout << "Delay Sum           = " << i->second.delaySum.GetSeconds() << endl;
        cout << "Delay/Packet (mean) = " << i->second.delaySum.GetSeconds()/i->second.rxPackets << endl;
        cout << "Received Throughput = " << throughput << " bps" << " " << throughput/1024 << " kbps" << endl << endl;
        cout << "Lost packets        = " << i->second.lostPackets/((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds()) << endl;
      
        throughput = ((throughput > 0) ? throughput : 0);
      sumThroughput += throughput;
      delay = i->second.delaySum.GetSeconds()/i->second.rxPackets;
      delay = ((delay > 0) ? delay : 0);
      meanDelayPackets += delay;
      meanLostPackets += i->second.lostPackets/((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds());

     /* if(!mobility){
        if(devicesIP.GetAddress(nearestNode) == t.sourceAddress){
          if(traffic){
            std::stringstream ss;
            ss <<prefix<<"_"<<"Nearest_node_cbr.csv";
            f = fopen(ss.str().c_str(), "a");
            fprintf(f, "%d;%.2f;%.2f;%.2f;%d\n", nNodes, dnearestNode, throughput/1024, delay, i->second.lostPackets);
          }
          else{
            std::stringstream ss;
            ss <<prefix<<"_"<<"Nearest_node_pulse.csv";
            f = fopen(ss.str().c_str(), "a");
            fprintf(f, "%d;%.2f;%.2f;%.2f;%d\n", nNodes, dnearestNode, throughput/1024, delay, i->second.lostPackets);
          }
          fclose(f);
        }
 else if (devicesIP.GetAddress(farthestNode) == t.sourceAddress){
          if(traffic){
            std::stringstream ss;
            ss <<prefix<<"_"<<"Farthest_node_cbr.csv";
            f = fopen(ss.str().c_str(), "a");
            fprintf(f, "%d;%.2f;%.2f;%.2f;%d\n", nNodes, dfarthestNode, throughput/1024, delay, i->second.lostPackets);
          }
          else{
            std::stringstream ss;
            ss <<prefix<<"_"<<"Farthest_node_pulse.csv";
            f = fopen(ss.str().c_str(), "a");
            fprintf(f, "%d;%.2f;%.2f;%.2f;%d\n", nNodes, dfarthestNode, throughput/1024, delay, i->second.lostPackets);
          }
          fclose(f);
        }
      }
    }*/
}
  }
  
 cout << "#######Valor de meanThroughput ####### " << meanThroughput << endl;
 cout << "#######Valor de sumThroughput  ####### " << sumThroughput << endl;
 cout << "#######Valor de nWifi e porcentagem  ####### " << nWifi << " " << porcentagem << endl;
 

  meanThroughput  = sumThroughput / porcentagem; 
  meanDelayPackets /= porcentagem;
  meanLostPackets /= porcentagem;

  cout << "Throughput (mean)   : " << meanThroughput/1024 << " kbps"<< endl;
  cout << "Delay Packets (mean): " << meanDelayPackets << endl;
  cout << "Lost Packets (mean) : " << meanLostPackets << endl;

  // if udp/cbr
  /*if(traffic){
    if(mobility){
      std::stringstream ss;
      ss <<prefix<<"_"<<"RandomWalk_cbr.csv";
      f = fopen(ss.str().c_str(), "a");
      fprintf(f, "%d;%.2f;%.2f;%.2f;%.2f\n", nNodes, meanThroughput/1024, meanDelayPackets, meanLostPackets, sumThroughput);
    }
    else{
      std::stringstream ss;
      ss <<prefix<<"_"<<"ConstantPosition_cbr.csv";
      f = fopen(ss.str().c_str(), "a");
      fprintf(f, "%d;%.2f;%.2f;%.2f;%.2f\n", nNodes, meanThroughput/1024, meanDelayPackets, meanLostPackets, sumThroughput);
    }
  }
  else{
    if(mobility){
      std::stringstream ss;
      ss <<prefix<<"_"<<"RandomWalk_pulse.csv";
      f = fopen(ss.str().c_str(), "a");
      fprintf(f, "%d;%.2f;%.2f;%.2f;%.2f\n", nNodes, meanThroughput/1024, meanDelayPackets, meanLostPackets, sumThroughput);
    }
    else{*/
      std::stringstream ss;
      ss <<0<<"_"<<"ConstantPosition_udp_cbr.csv";
      f = fopen(ss.str().c_str(), "a");
      fprintf(f, "%d;%.2f;%.2f;%.2f;%.2f\n", nWifi, meanThroughput/1024, meanDelayPackets, meanLostPackets, sumThroughput);
    /*}
  }*/
  fclose(f);
}

void tcp (uint32_t porcentagem, Ipv4InterfaceContainer &csmaInterfaces, NodeContainer &wifiStaNodes, NodeContainer &wifiApNode)
{
  ApplicationContainer serverApp;
  ApplicationContainer sinkApp;

  std::ostringstream ossOnTime;
  ossOnTime << "ns3::ConstantRandomVariable[Constant=" << 0.001 << "]";
  std::ostringstream ossOffTime;
  ossOffTime << "ns3::ConstantRandomVariable[Constant=" << 0.001 << "]";

  /* Install TCP/UDP Transmitter on the station */
  for (uint32_t i = 0; i < porcentagem; i++){
  /* Install TCP Receiver on the access point */
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (csmaInterfaces.GetAddress(0), i+10000));
    sinkApp = sinkHelper.Install (wifiStaNodes.Get(i));
    sinkApp.Add (sinkHelper.Install (wifiApNode.Get(0)));
    sinkApp.Start (Seconds (0.0));
    OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (csmaInterfaces.GetAddress(0), i+10000)));
    server.SetAttribute ("PacketSize", UintegerValue (1484));
    server.SetAttribute ("OnTime", StringValue (ossOnTime.str()));
    server.SetAttribute ("OffTime", StringValue (ossOffTime.str()));
    server.SetAttribute ("DataRate", StringValue ("512kbps"));
    serverApp = server.Install (wifiStaNodes.Get(i));
    serverApp.Start (Seconds (1));
  }
    serverApp.Stop(Seconds(60+1));
}

void udp(NodeContainer &wifiApNode, float time, Ipv4InterfaceContainer &csmaInterfaces, uint32_t porcentagem, uint32_t nWifi, NodeContainer &wifiStaNodes){

UdpEchoServerHelper echoServer (9);
//codigo a se observar/////////////////////
  ApplicationContainer serverApps = echoServer.Install (wifiApNode.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (time)); //pega o tempo da variavel TIME
//TCP


//UDP
UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (0), 9);
//cout << "linha 235" << endl;
echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
echoClient.SetAttribute ("DataRate", StringValue ("512kbps"));
echoClient.SetAttribute ("PacketSize", UintegerValue (484));
//codigo a se observar////////////////////
  uint32_t nnode;

  for ( uint32_t x=1 ; x <= porcentagem ; x++) {
	  nnode = (nWifi - x);
	  printf("%d\n", nnode);
	  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nnode));
	  clientApps.Start (Seconds (2.0));
	  clientApps.Stop (Seconds (time));
  }

}

int main (int argc, char *argv[]){
  bool verbose = false;
  uint32_t nCsma = 1;
  float time = 60.0;
  uint32_t nWifi = 10;//alterar valores
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

  NodeContainer wifiApNode;
  wifiApNode = p2pNodes.Get (1);
//  csmaNodes.Create (nCsma);

  CsmaHelper csma;
//  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
//  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

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
  stack.Install (p2pNodes.Get(0));
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (apDevices);
  Ipv4InterfaceContainer devicesInterfaces;
  devicesInterfaces = address.Assign (staDevices);

  //udp(wifiApNode, time, csmaInterfaces, porcentagem, nWifi, wifiStaNodes);
  tcp (porcentagem, csmaInterfaces, wifiStaNodes, wifiApNode);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();//protocolo IP

  Simulator::Stop (Seconds (time));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", apDevices.Get (0), true);
    }

//###### flow monitor ###########
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds(time));
  Simulator::Run ();

  flowMonitor->SerializeToXmlFile("MariaNS3.xml", true, true); //GERA um xml com o nome MariaNS3 na pasta ns-3.26
  
  funcao_flow(flowHelper, flowMonitor, p2pInterfaces, csmaInterfaces, nWifi, porcentagem);
  Simulator::Destroy ();
  return 0;
}
