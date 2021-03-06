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
 *
 * Author: Nelson Powell (Adapted from wireless-animation.cc)
 */


#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/waypoint-mobility-model.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/batmand-routing-protocol.h"
#include "ns3/batmand-helper.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/olsr-helper.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/aggregator-helper.h"
//#include "ns3/aggregator.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThreeRadio");

void PktValue( Ptr<Packet const> initpkt ) {

  //std::ostream os = &std::cout;
  //uint8_t buffer[6];
  //Mac48Address addr;

  //  printf(" pkt id %lu has size %u at Time %f\n", initpkt->GetUid(), initpkt->GetSize(), (Simulator::Now().GetDouble() / 1000000000.0) );
}

int
main (int argc, char *argv[])
{
  uint32_t nWifi = 2;
  int  nRun = 0;
  int  nSeed = 5;
  int  nCount = 1;
  double   maxSimulatedTime = 45.0;
  
  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("nTime", "Maximum simulation time", maxSimulatedTime );
  cmd.AddValue ("nRun", "Simulation run number", nRun);
  cmd.AddValue ("nSeed", "Seed value that differs from the default", nSeed);
  cmd.AddValue ("nCount", "Number of iterations to run this scenario with differing PRNG", nCount);


  cmd.Parse (argc,argv);

  RngSeedManager::SetSeed(nSeed);
  RngSeedManager::SetRun(nRun);

  for ( int j = 0; j < nCount; ++j )
  {
    for ( uint8_t itr = 0; itr < 2; ++itr )
    {
  
      /*
      * Create NS-3 Nodes to accumulate net devices
      */
      NodeContainer allNodes;
      NodeContainer wifiStaNodes;
      NodeContainer wifiSrvrNode;
      NodeContainer manetNodes;
      NodeContainer clientNodes;

      // Create the server first so it is MAC 00:00:00:00:00:01
      wifiSrvrNode.Create (1);
      allNodes.Add (wifiSrvrNode);
      // Now create the mobile stations that want to send data to the server
      wifiStaNodes.Create (nWifi);
      allNodes.Add (wifiStaNodes);
      manetNodes.Add (wifiStaNodes);
      manetNodes.Add (wifiSrvrNode);
      clientNodes.Add( wifiStaNodes.Get(1) );

      /*
      * Setup the WiFi Channel, MAC, and MAC Helper for AD HOC Mode
      */
      YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
      YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
      phy.SetChannel (channel.Create ());
      phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      WifiHelper wifi;
      wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
      WifiMacHelper mac;
      mac.SetType ("ns3::AdhocWifiMac");
      NetDeviceContainer srvrDevice;
      srvrDevice = wifi.Install (phy, mac, wifiSrvrNode);
      NetDeviceContainer staDevices;
      staDevices = wifi.Install (phy, mac, wifiStaNodes);

      /*
      * Setup the mobility model for the nodes
      */
      MobilityHelper mobility;
      MobilityHelper waymobility;
      Ptr<WaypointMobilityModel> mob;
      //mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
      //                               "MinX", DoubleValue (10.0),
      //                               "MinY", DoubleValue (10.0),
      //                               "DeltaX", DoubleValue (70.0),
      //                               "DeltaY", DoubleValue (70.0),
      //                               "GridWidth", UintegerValue (50),
      //                               "LayoutType", StringValue ("RowFirst"));
      //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
      //                           "Bounds", RectangleValue (Rectangle (-150, 150, -150, 150)));
      //mobility.Install (allNodes);
      //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

      // The first mobility model is a constant position for the UDP Echo Server - i.e. sink
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      positionAlloc->Add (Vector (0,50,0));
      //positionAlloc->Add (Vector (75,75,0));
      //positionAlloc->Add (Vector (150,50,0));
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (wifiSrvrNode.Get(0));
      //mobility.Install (wifiStaNodes.Get(0));
      //mobility.Install (wifiStaNodes.Get(1));


      // Now we move the UDP Echo Clients using waypoints - just for now
      waymobility.SetMobilityModel("ns3::WaypointMobilityModel");
      waymobility.Install(wifiStaNodes.Get(0));
      // First we do the center station
      mob = wifiStaNodes.Get(0)->GetObject<WaypointMobilityModel>();
      Waypoint waySta1_p0 = Waypoint(Seconds(0),Vector(75,75,0));
      mob->AddWaypoint(waySta1_p0);
      Waypoint waySta1_p1 = Waypoint(Seconds(2),Vector(75,75,0));
      mob->AddWaypoint(waySta1_p1);
      Waypoint waySta1_p2 = Waypoint(Seconds(8),Vector(50,75,0));
      mob->AddWaypoint(waySta1_p2);
      Waypoint waySta1_p3 = Waypoint(Seconds(20),Vector(50,75,0));
      mob->AddWaypoint(waySta1_p3);
      Waypoint waySta1_p4 = Waypoint(Seconds(22),Vector(75,75,0));
      mob->AddWaypoint(waySta1_p4);
      // Then we do the far right station
      waymobility.Install(wifiStaNodes.Get(1));
      mob = wifiStaNodes.Get(1)->GetObject<WaypointMobilityModel>();
      Waypoint waySta2_p0 = Waypoint(Seconds(0),Vector(150,50,0));
      mob->AddWaypoint(waySta2_p0);
      Waypoint waySta2_p1 = Waypoint(Seconds(2),Vector(150,50,0));
      mob->AddWaypoint(waySta2_p1);
      Waypoint waySta2_p2 = Waypoint(Seconds(12),Vector(90,50,0));
      mob->AddWaypoint(waySta2_p2);
      Waypoint waySta2_p3 = Waypoint(Seconds(20),Vector(90,50,0));
      mob->AddWaypoint(waySta2_p3);
      Waypoint waySta2_p4 = Waypoint(Seconds(24),Vector(150,50,0));
      mob->AddWaypoint(waySta2_p4);


      /*
       * Setup the WiFi energy model
       */
      Ptr<BasicEnergySource> energySource = CreateObject<BasicEnergySource>();
      Ptr<SimpleDeviceEnergyModel> energyModel = CreateObject<SimpleDeviceEnergyModel>();
      energySource->SetInitialEnergy (300);
      energyModel->SetEnergySource (energySource);
      energySource->AppendDeviceEnergyModel (energyModel);
      energyModel->SetCurrentA (20);

      /*
       * Configure the BATMAN or OLSR engine
       */
      BatmandHelper batman;
      OlsrHelper olsr;
      Ipv4StaticRoutingHelper staticRouting;
      Ipv4ListRoutingHelper list;
      InternetStackHelper internet_helper;
      list.Add (staticRouting, 0);

      if ( itr == 0 ) {
        NS_LOG_DEBUG("Executing BATMAN test");
        list.Add (batman, 10);
      }
      else {
        NS_LOG_DEBUG("Executing OLSRv1 test");
        list.Add (olsr, 10);
      }
      
      internet_helper.SetRoutingHelper (list); // has effect on the next Install ()
      internet_helper.Install (manetNodes);

      /*
      * Create the internet stack
      */
      // Install Ipv4 addresses
      Ipv4AddressHelper address;
      address.SetBase ("10.1.1.0", "255.255.255.0");
      Ipv4InterfaceContainer srvrInterface;
      srvrInterface = address.Assign (srvrDevice);
      Ipv4InterfaceContainer staInterfaces;
      staInterfaces = address.Assign (staDevices);

      // Install applications
      UdpEchoServerHelper echoServer (9);
      ApplicationContainer serverApps = echoServer.Install (wifiSrvrNode.Get (0));
      serverApps.Start (Seconds (1.0));
      serverApps.Stop (Seconds ( maxSimulatedTime ));
      UdpEchoClientHelper echoClient (srvrInterface.GetAddress (0), 9);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
      echoClient.SetAttribute ("PacketSize", UintegerValue (1500));
      ApplicationContainer clientApps = echoClient.Install (clientNodes);
      clientApps.Start (Seconds (3.0));
      clientApps.Stop (Seconds ( maxSimulatedTime ));

    //  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      Simulator::Stop (Seconds ( maxSimulatedTime ));

      
      std::string animation_filename;
     
      if ( itr == 0 )
        animation_filename = "Hnode/batman-animation.xml";
      else
        animation_filename = "Hnode/olsr-animation.xml";
      
      AnimationInterface anim (animation_filename); // Mandatory
      for (uint32_t i = 0; i < wifiStaNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (wifiStaNodes.Get (i), "STA"); // Optional
        anim.UpdateNodeColor (wifiStaNodes.Get (i), 255, 0, 0); // Optional
      }
      for (uint32_t i = 0; i < wifiSrvrNode.GetN (); ++i)
      {
        anim.UpdateNodeDescription (wifiSrvrNode.Get (i), "SRVR"); // Optional
        anim.UpdateNodeColor (wifiSrvrNode.Get (i), 0, 255, 0); // Optional
      }

      anim.EnablePacketMetadata (); // Optional
      
      if ( itr == 0 )
        anim.EnableIpv4RouteTracking ("Hnode/batman-routingtable-wireless.xml", Seconds (0), Seconds (maxSimulatedTime), Seconds (0.5)); //Optional
      else
        anim.EnableIpv4RouteTracking ("Hnode/olsr-routingtable-wireless.xml", Seconds (0), Seconds (maxSimulatedTime), Seconds (0.5)); //Optional
              
      anim.EnableWifiMacCounters (Seconds (0), Seconds (maxSimulatedTime)); //Optional
      anim.EnableWifiPhyCounters (Seconds (0), Seconds (maxSimulatedTime)); //Optional
      anim.SetMaxPktsPerTraceFile(100000000);

      // Setup tracing in PCAP
      if ( itr == 0 )
      {
//        phy.EnablePcap ("batman-3radio", staDevices, true);
//        phy.EnablePcap ("batman-3radio", srvrDevice, true);
      }
      else
      {
//        phy.EnablePcap ("olsr-3radio", staDevices, true);
//        phy.EnablePcap ("olsr-3radio", srvrDevice, true);
      }

      
      AggregatorHelper aggHelper;
      Ptr<Aggregator>  aggregator = aggHelper.GetAggregator();
      
      // Add all nodes to the packet aggregator
      aggHelper.Install( allNodes );
      // Now add the Udp Echo client server applications
      aggHelper.InstallUdpClient( clientApps );
      aggHelper.InstallUdpServer( serverApps );
      
      
    /*  
      PacketAggregator aggregator;
      NdCbList  perNodeCbList;
      

      // Callbacks for NODE 0  
      Ptr<Node> ndptr = wifiSrvrNode.Get(0);
      NodeCallback node0( ndptr->GetId(), &aggregator );
      perNodeCbList.push_back( node0 );

      printf("srvr node has %u devices\n", ndptr->GetNDevices() );

      Ptr<WifiNetDevice> wfnd = DynamicCast<WifiNetDevice> ( ndptr->GetDevice(0) );
      Ptr<WifiMac> themac = wfnd->GetMac();
      Ptr<WifiPhy> thephy = themac->GetWifiPhy();

      printf(" \n Config CallBack\n\n");
      thephy->TraceConnectWithoutContext( "PhyRxBegin" , MakeCallback( &NodeCallback::FrameRxStartEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyRxEnd" , MakeCallback( &NodeCallback::FrameRxStopEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyTxBegin" , MakeCallback( &NodeCallback::FrameTxStartEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyTxEnd" , MakeCallback( &NodeCallback::FrameTxStopEvent , &( perNodeCbList.back() ) ) );

      Ptr<UdpEchoServer> serverptr = DynamicCast<UdpEchoServer> ( serverApps.Get( 0 ) );
      serverptr->TraceConnectWithoutContext( "Tx" , MakeCallback( &NodeCallback::PktTxEvent , &( perNodeCbList.back() ) ) );
      serverptr->TraceConnectWithoutContext( "Rx" , MakeCallback( &NodeCallback::PktRxEvent , &( perNodeCbList.back() ) ) );

      // Just for debug
        serverptr->TraceConnectWithoutContext( "Rx" , MakeCallback( PktValue ) ) ;

      Ptr<batmand::RoutingProtocol> bat0 = ndptr->GetObject<ns3::batmand::RoutingProtocol>();
      bat0->TraceConnectWithoutContext( "TxPkt" , MakeCallback( &NodeCallback::RouterPacket , &( perNodeCbList.back() ) ) );
      
      // Callbacks for NODE 1
      ndptr = wifiStaNodes.Get(0);
      NodeCallback node1( ndptr->GetId(), &aggregator );
      perNodeCbList.push_back( node1 );
      
      wfnd = DynamicCast<WifiNetDevice> ( ndptr->GetDevice(0) );
      themac = wfnd->GetMac();
      thephy = themac->GetWifiPhy();
      
      thephy->TraceConnectWithoutContext( "PhyRxBegin" , MakeCallback( &NodeCallback::FrameRxStartEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyRxEnd" , MakeCallback( &NodeCallback::FrameRxStopEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyTxBegin" , MakeCallback( &NodeCallback::FrameTxStartEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyTxEnd" , MakeCallback( &NodeCallback::FrameTxStopEvent , &( perNodeCbList.back() ) ) );

      Ptr<batmand::RoutingProtocol> bat1 = ndptr->GetObject<ns3::batmand::RoutingProtocol>();
      bat1->TraceConnectWithoutContext( "TxPkt" , MakeCallback( &NodeCallback::RouterPacket , &( perNodeCbList.back() ) ) );
      

      // Callbacks for NODE 2
      ndptr = wifiStaNodes.Get(1);
      NodeCallback node2( ndptr->GetId(), &aggregator );
      perNodeCbList.push_back( node2 );
      
      wfnd = DynamicCast<WifiNetDevice> ( ndptr->GetDevice(0) );
      themac = wfnd->GetMac();
      thephy = themac->GetWifiPhy();
      
      thephy->TraceConnectWithoutContext( "PhyRxBegin" , MakeCallback( &NodeCallback::FrameRxStartEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyRxEnd" , MakeCallback( &NodeCallback::FrameRxStopEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyTxBegin" , MakeCallback( &NodeCallback::FrameTxStartEvent , &( perNodeCbList.back() ) ) );
      thephy->TraceConnectWithoutContext( "PhyTxEnd" , MakeCallback( &NodeCallback::FrameTxStopEvent , &( perNodeCbList.back() ) ) );

      Ptr<UdpEchoClient> clientptr = DynamicCast<UdpEchoClient> ( clientApps.Get( 0 ) );
      clientptr->TraceConnectWithoutContext( "Tx" , MakeCallback( &NodeCallback::PktTxEvent , &( perNodeCbList.back() ) ) );
      clientptr->TraceConnectWithoutContext( "Rx" , MakeCallback( &NodeCallback::PktRxEvent , &( perNodeCbList.back() ) ) );

      // Just for debug
        clientptr->TraceConnectWithoutContext( "Rx" , MakeCallback( &PktValue ) );

      Ptr<batmand::RoutingProtocol> bat2 = ndptr->GetObject<ns3::batmand::RoutingProtocol>();
      bat2->TraceConnectWithoutContext( "TxPkt" , MakeCallback( &NodeCallback::RouterPacket , &( perNodeCbList.back() ) ) );

    */

      // Just for debug
      Ptr<UdpEchoServer> serverptr = DynamicCast<UdpEchoServer> ( serverApps.Get( 0 ) );
      Ptr<UdpEchoClient> clientptr = DynamicCast<UdpEchoClient> ( clientApps.Get( 0 ) );
      serverptr->TraceConnectWithoutContext( "Rx" , MakeCallback( &PktValue ) ) ;
      clientptr->TraceConnectWithoutContext( "Rx" , MakeCallback( &PktValue ) );

      Simulator::Run ();
      Simulator::Destroy ();
      
      // Now generate the output CSV file for plotting
      // NOTE: Adjust the third argument for scalling factor - timeline ticks in seconds
      char filename[200];
      if ( itr == 0 )
      {
        sprintf( filename, "Hnode/batman-3node-%02d.csv", j );
        aggregator->PlotData( filename, maxSimulatedTime, 1.0 );
      }
      else
      {
        sprintf( filename, "Hnode/olsr-3node-%02d.csv", j );
        aggregator->PlotData( filename, maxSimulatedTime, 1.0 );
      }
      
    }
    
    nSeed *= 13;
    nRun = ( (nRun * nSeed) / 7 ) + 5;
    RngSeedManager::SetSeed(nSeed);
    RngSeedManager::SetRun(nRun);

  }
  
  return 0;
}
