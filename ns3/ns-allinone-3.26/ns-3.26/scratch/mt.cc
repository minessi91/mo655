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
// n5   n6   n7   n0 -------------- n1     
//                   point-to-point  |      
//                                   =
//                              LAN 10.1.2.0

using namespace ns3;
using namespace std;


int myRand(int min, int max){

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min));
  x->SetAttribute ("Max", DoubleValue (max));

  int value = x->GetValue ();

  return value;
}

//################ funcao_flow #################
void funcao_flow(FlowMonitorHelper &flowmon, Ptr<FlowMonitor> &monitor, Ipv4InterfaceContainer &devicesIP, Ipv4InterfaceContainer &p2pdeviceIP,  uint32_t nWifi, uint32_t porcentagem, int tipo_trafego, size_t nodenear, size_t nodefar){

//variaveis iniciando com zero
  double taxaTransferencia = 0;
  double delay = 0;
  double somaTransferencia = 0.0;
  double mediaVazao = 0.0;
  double mediadelay = 0.0;
  double mediaPerdaPacote = 0.0;
  int count = 0;
  FILE *f;

  monitor->CheckForLostPackets();
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());

  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin (); i != stats.end (); ++i, count++){
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

    if (t.destinationAddress == p2pdeviceIP.GetAddress(0)){
      taxaTransferencia = (i->second.txBytes * 8) / ((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds());


        cout << "Flowid                         = " << i->first << endl;
        cout << "Endereço de partida            = " << t.sourceAddress << endl;
        cout << "Endereço de destino            = " << t.destinationAddress << endl;
        /*cout << "Primeiro pacote Tx             = " << i->second.timeFirstTxPacket.GetSeconds() << endl;
        cout << "Primeiro pacote Rx             = " << i->second.timeFirstRxPacket.GetSeconds() << endl;
        cout << "Ultimo pacote Tx               = " << i->second.timeLastTxPacket.GetSeconds() << endl;
        cout << "Ultimo pacote Rx               = " << i->second.timeLastRxPacket.GetSeconds() << endl;
        cout << "Pacotes  Tx                    = " << i->second.txPackets << endl;
        cout << "Pacotes  RX                    = " << i->second.rxPackets << endl;
        cout << "Pacotes perdidos               = " << i->second.lostPackets << endl;
        cout << "Tx Bytes                       = " << i->second.txBytes << endl;
        cout << "RX bytes                       = " << i->second.rxBytes << endl;
        cout << "Atraso da soma                 = " << i->second.delaySum.GetSeconds() << endl;
        cout << "Média de atraso do pacote      = " << i->second.delaySum.GetSeconds()/i->second.rxPackets << endl;
        cout << "Taxa de transferencia recebida = " << taxaTransferencia << " bps" << " " << taxaTransferencia/1024 << " kbps" << endl << endl;*/
            
      taxaTransferencia = ((taxaTransferencia > 0) ? taxaTransferencia : 0);
      somaTransferencia += taxaTransferencia;
      delay = i->second.delaySum.GetSeconds()/i->second.rxPackets;
      delay = ((delay > 0) ? delay : 0);
      mediadelay += delay;
      mediaPerdaPacote += i->second.lostPackets/((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds());
        cout << "sa " << t.sourceAddress << " n " << devicesIP.GetAddress(nodenear) << " f " << devicesIP.GetAddress(nodefar) << endl;
        if(devicesIP.GetAddress(nodenear) == t.sourceAddress){
            std::stringstream ss;
            ss <<nWifi<<"_"<<"mais_proximo.csv";
            f = fopen(ss.str().c_str(), "a");
            fprintf(f, "%d;%.2f;%.2f;%d\n", nWifi, taxaTransferencia/1024, delay, i->second.lostPackets);
            fclose(f);
        }
         else if (devicesIP.GetAddress(nodefar) == t.sourceAddress){
            std::stringstream ss;
            ss <<nWifi<<"_"<<"mais_long.csv";
            f = fopen(ss.str().c_str(), "a");
            fprintf(f, "%d;%.2f;%.2f;%d\n", nWifi, taxaTransferencia/1024, delay, i->second.lostPackets);
            fclose(f);
        }
      }
    }
  
 

  mediaVazao  = somaTransferencia / porcentagem; 
  mediadelay /= porcentagem;
  mediaPerdaPacote /= porcentagem;

  cout << "Valor de Vazão    : " << mediaVazao/1024 << " kbps"<< endl;
  cout << "Valor de Atraso   : " << mediadelay << endl;
  cout << "Valor de Perda    : " << mediaPerdaPacote << endl;



  // Caso seja escolhido as opções 0 será tcp, opção 1 será 50/50 e opção 2 será udp

        std::stringstream ss;
        switch(tipo_trafego){
                case 0:
                      ss <<nWifi<<"_"<<"tcp.csv";
                break;
                case 1:
                     ss <<nWifi<<"_"<<"5050.csv";
                break;
                case 2:
                     ss <<nWifi<<"_"<<"udp.csv";
                break;
                default:
                return;
        }
        f = fopen(ss.str().c_str(), "a");
        fprintf(f, "%d;%.2f;%.10f;%.2f;%.2f\n", nWifi, mediaVazao/1024, mediadelay, mediaPerdaPacote, somaTransferencia);


  fclose(f);
}

//################### TCP ###################
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

//################### UDP ###################
void udp(NodeContainer &wifiApNode, float time, Ipv4InterfaceContainer &csmaInterfaces, uint32_t porcentagem, uint32_t nWifi, NodeContainer &wifiStaNodes){

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiApNode.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (time)); //pega o tempo da variavel TIME

UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (0), 9);
echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
echoClient.SetAttribute ("DataRate", StringValue ("512kbps"));
echoClient.SetAttribute ("PacketSize", UintegerValue (484));

  uint32_t nnode;

  for ( uint32_t x=1 ; x <= porcentagem ; x++) {
	  nnode = (nWifi - x);
	  printf("%d\n", nnode);
	  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nnode));
	  clientApps.Start (Seconds (2.0));
	  clientApps.Stop (Seconds (time));
  }

}

void tcp_udp (NodeContainer &wifiApNode, float time, Ipv4InterfaceContainer &csmaInterfaces, uint32_t porcentagem, uint32_t nWifi, NodeContainer &wifiStaNodes)
{

  ApplicationContainer serverApp;
  ApplicationContainer sinkApp;

  std::ostringstream ossOnTime;
  ossOnTime << "ns3::ConstantRandomVariable[Constant=" << 0.001 << "]";
  std::ostringstream ossOffTime;
  ossOffTime << "ns3::ConstantRandomVariable[Constant=" << 0.001 << "]";

  /* TCP/UDP */
  for (uint32_t i = 0; i < porcentagem/2; i++){
  /* Instala TCP no access point */
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

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiApNode.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (time)); //pega o tempo da variavel TIME

UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (0), 9);
echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
echoClient.SetAttribute ("DataRate", StringValue ("512kbps"));
echoClient.SetAttribute ("PacketSize", UintegerValue (484));

  uint32_t nnode;

  for ( uint32_t x=1 ; x <= porcentagem/2 ; x++) {
	  nnode = (nWifi - x);
	  printf("%d\n", nnode);
	  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nnode));
	  clientApps.Start (Seconds (2.0));
	  clientApps.Stop (Seconds (time));
  }
}

double calcDistance (uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
  return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}



//################### MAIN ###################
int main (int argc, char *argv[]){
  bool verbose = false;
  uint32_t nCsma = 1;
  float time = 60.0;
  uint32_t nWifi = 30;//###### ALTERAR AQUI  ###########
  bool tracing = false;
  int tipo_trafego=0;//0 tcp; 1 50/50; 2 udp ###### ALTERAR AQUI  ###########

size_t nodenear = -1;
size_t nodefar = -1;
double dnodenear;
double dnodefar;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("time", "Number of \"extra\" CSMA nodes/devices", time);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.Parse (argc,argv);
  
  uint32_t porcentagem = (nWifi * 0.2); // CALCULO DA PORCENTAGEM - NESSE CASO ESTOU UTILIZANDO 20% - ###### ALTERAR AQUI para 10 20 30 e 40 ###########
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
  pointToPoint.SetChannelAttribute ("delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer wifiApNode;
  wifiApNode = p2pNodes.Get (1);

 MobilityHelper mobilityh;
  //Lista a posição
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  //Nó ap estático
  uint32_t ApX = 50;
  uint32_t ApY = 50;
  positionAlloc->Add (Vector (ApX, ApY, 0.0));
  mobilityh.SetPositionAllocator (positionAlloc);
  mobilityh.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityh.Install (wifiApNode);
//  csmaNodes.Create (nCsma);

  CsmaHelper csma;
//  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
//  csma.SetChannelAttribute ("delay", TimeValue (NanoSeconds (6560)));

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
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);


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

  switch(tipo_trafego)
  {
        case 0:
                tcp (porcentagem, csmaInterfaces, wifiStaNodes, wifiApNode);// Chama a função TCP
        break;
        case 1:
                tcp_udp(wifiApNode, time, csmaInterfaces, porcentagem, nWifi, wifiStaNodes);//Chama a função TCP e UDP -50%
        break;
        case 2:
                udp(wifiApNode, time, csmaInterfaces, porcentagem, nWifi, wifiStaNodes);//Chama a função UDP
        break;
        default:
        return 0;
   }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();//protocolo IP

  MobilityHelper mobility;

  for (size_t i = 0; i < nWifi; i++) 
  {
     uint32_t NodeX = myRand(0, 100); //O 100 é um tamanho bom!
     uint32_t NodeY = myRand(0, 100);
     double result = calcDistance(ApX, ApY, NodeX, NodeY);

cout << devicesInterfaces.GetAddress(i) << endl;
  switch(tipo_trafego)
  {
        case 0:
            if(i < porcentagem){
             if(i == 0){
                  nodenear = i;
                  nodefar = i;
                  dnodenear = result;
                  dnodefar = result;
             }
             else {
                if(result > dnodefar){
                  nodefar = i;
                  dnodefar = result;
                }
                if(result < dnodenear){
                  nodenear = i;
                  dnodenear = result;
                }
             }
            }
        break;
        case 1:
            if(i < porcentagem/2 || i >= (nWifi - porcentagem/2)){
             if(i == 0){
                  nodenear = i;
                  nodefar = i;
                  dnodenear = result;
                  dnodefar = result;
             }
             else {
                if(result > dnodefar){
                  nodefar = i;
                  dnodefar = result;
                }
                if(result < dnodenear){
                  nodenear = i;
                  dnodenear = result;
                }
             }
            }
        break;
        case 2:
            if(i >= (nWifi - porcentagem)){
             if(i == (nWifi - porcentagem)){
                  nodenear = i;
                  nodefar = i;
                  dnodenear = result;
                  dnodefar = result;
             }
             else {
                if(result > dnodefar){
                  nodefar = i;
                  dnodefar = result;
                }
                if(result < dnodenear){
                  nodenear = i;
                  dnodenear = result;
                }
             }
            }
        break;
        default:
        return 0;
   }
     Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
     //positionAlloc = CreateObject<ListPositionAllocator> ();
     positionAlloc->Add (Vector (NodeX, NodeY, 0.0));
     mobility.SetPositionAllocator (positionAlloc);
     mobility.Install (wifiStaNodes.Get(i));

   }

   Simulator::Stop (Seconds (time)); 

    if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", apDevices.Get (0), true);
    }

//################ FLOW MONITOR #################
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    Simulator::Stop (Seconds(time));
    Simulator::Run ();

  
    funcao_flow(flowHelper, flowMonitor, devicesInterfaces, csmaInterfaces, nWifi, porcentagem, tipo_trafego, nodenear, nodefar);//Chama a função funcao_flow
    Simulator::Destroy ();
    return 0;
}
