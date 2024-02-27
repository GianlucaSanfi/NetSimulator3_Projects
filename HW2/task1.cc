#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <fstream>
#include <string.h>

#include <stdio.h>

#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/wifi-radio-energy-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HW2_Task1_Team_59");
/* WLAN
    5 nodi AD HOC
*/
int main(int argc, char * argv[]){

    bool useRtsCts = false;
    bool verbose = false;
    bool useNetAnim = false;

    CommandLine cmd (__FILE__);
    cmd.AddValue ("useRtsCts", "for Rts/Cts set true", useRtsCts);
    cmd.AddValue ("verbose", "to enable logs set true", verbose);
    cmd.AddValue ("useNetAnim", "for animation set true", useNetAnim);

    cmd.Parse (argc,argv);

    if (verbose){
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    //config rts/cts
    UintegerValue ctsThreshold = (useRtsCts? UintegerValue(100) : UintegerValue(2346));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThreshold);

    //------- NODES WIFI -------
    NodeContainer wifiStaNodes;

    wifiStaNodes.Create(5);

    //------- CHANNEL WIFI -------
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g); //if not working use WIFI_PHY_STANDARD_80211g
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    //------- NET DEVICES -------
    WifiMacHelper mac;
 
    mac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer staDevices = wifi.Install(phy, mac, wifiStaNodes);
  
    //------- MOBILITY -------
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-90, 90, -90, 90)));
    mobility.Install(wifiStaNodes);
/*
    //------- POWER MANAGEMENT -------
    Ptr<BasicEnergySource> energySource = CreateObject<BasicEnergySource>();
    Ptr<WifiRadioEnergyModel> energyModel = CreateObject<WifiRadioEnergyModel>();

    energySource->SetInitialEnergy (300);
    energyModel->SetEnergySource (energySource);
    energySource->AppendDeviceEnergyModel (energyModel);

    wifiApNodes.Get(0)->AggregateObject(energySource);
*/
    //------- NETWORK SETUP -------
    InternetStackHelper stack;
    stack.Install (wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");

    Ipv4InterfaceContainer wifiStaInterfaces;
    wifiStaInterfaces = address.Assign(staDevices);

    //------- APPLICATIONS -------
    /*
    udp echo server n0 port=20
        udp echo client n4 -> n0 2 pkts time: 1s, 2s
        udp echo client n3 -> n0 2 pkts time: 2s, 4s
        pkts sie 512 B
    */
   //server n0
    UdpEchoServerHelper echoServer(20);
    ApplicationContainer serverApps = echoServer.Install(wifiStaNodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(7.0));

    //client n3
    UdpEchoClientHelper echoClient1(wifiStaInterfaces.GetAddress(0), 20);
    echoClient1.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(2.)));
    echoClient1.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps1 = echoClient1.Install(wifiStaNodes.Get(3));
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(5.0));

    //client n4
    UdpEchoClientHelper echoClient2(wifiStaInterfaces.GetAddress(0), 20);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps2 = echoClient2.Install(wifiStaNodes.Get(4));
    clientApps2.Start(Seconds(1.0));
    clientApps2.Stop(Seconds(5.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Simulator::Stop(Seconds(10.0));

    //------- NET ANIMATION -------
    if(useNetAnim){

        /*if(useRtsCts){
                IF BELOW STATEMENT DOESN'T WORK
        }*/
        AnimationInterface anim((useRtsCts? "wireless-task1-rts-on.xml" : "wireless-task1-rts-off.xml"));
        //n0
        anim.UpdateNodeDescription (wifiStaNodes.Get(0), "SRV-0");
        anim.UpdateNodeColor (wifiStaNodes.Get(0), 255, 0, 0);
        //n1
        anim.UpdateNodeDescription (wifiStaNodes.Get(1), "HOC-1");
        anim.UpdateNodeColor (wifiStaNodes.Get(1), 0, 0, 255);
        //n2
        anim.UpdateNodeDescription (wifiStaNodes.Get(2), "HOC-2");
        anim.UpdateNodeColor (wifiStaNodes.Get(2), 0, 0, 255);
        //n3
        anim.UpdateNodeDescription (wifiStaNodes.Get(3), "CLI-3");
        anim.UpdateNodeColor (wifiStaNodes.Get(3), 0, 255, 0);
        //n4
        anim.UpdateNodeDescription (wifiStaNodes.Get(4), "CLI-4");
        anim.UpdateNodeColor (wifiStaNodes.Get(4), 0, 255, 0);
            
        anim.EnablePacketMetadata ();
        anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25));
        anim.EnableWifiMacCounters (Seconds (0), Seconds (10));
        anim.EnableWifiPhyCounters (Seconds (0), Seconds (10));
  
        //------- TRACING -------
        //tracing pcap: n2
        phy.EnablePcap((useRtsCts? "task1-on-2.pcap" : "task1-off-2.pcap"), staDevices.Get(2), true, true);


        Simulator::Run();
        Simulator::Destroy();

    } else {

        //------- TRACING -------
        //tracing pcap: n2
        phy.EnablePcap((useRtsCts? "task1-on-2.pcap" : "task1-off-2.pcap"), staDevices.Get(2), true, true);


        Simulator::Run();
        Simulator::Destroy();

    }
           

    return 0;
}