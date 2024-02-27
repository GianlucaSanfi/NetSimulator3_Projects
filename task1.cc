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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Task_1_Team_59");

int main(int argc, char * argv[]){

    int configuration = 0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("configuration", "configuration default = 0", configuration);
    cmd.Parse(argc, argv);
    
    //LOG CMD
    LogComponentEnable("Task_1_Team_59", LOG_LEVEL_INFO);

    /*
    if(argc >= 1){

        char * ch = argv[1];
        configuration = atoi(ch);
    }
    */

/*area 0   |     area 1                |   area 2
csma       |    p2p                    |   csma

n0   n1   n2____________n3____n4______n6__    n7   n8
|____|_____|            |____n5________|  |___|____|

*/

//-----------------START CONTAINERS DEVICES-------------------
    //AREA 0 CSMA
    NodeContainer csmaNodes012;
    csmaNodes012.Create(3);

    //AREA 1 P2P
    NodeContainer p2pNodes23;
    p2pNodes23.Add(csmaNodes012.Get(2));
    p2pNodes23.Create(1); 

    NodeContainer p2pNodes34;
    p2pNodes34.Add(p2pNodes23.Get(1));
    p2pNodes34.Create(1);

    NodeContainer p2pNodes35;
    p2pNodes35.Add(p2pNodes23.Get(1));
    p2pNodes35.Create(1);

    NodeContainer p2pNodes46;
    p2pNodes46.Add(p2pNodes34.Get(1));
    p2pNodes46.Create(1);

    NodeContainer p2pNodes56;
    p2pNodes56.Add(p2pNodes35.Get(1));
    p2pNodes56.Add(p2pNodes46.Get(1));

    //AREA 2 CSMA
    NodeContainer csmaNodes678;
    csmaNodes678.Add(p2pNodes56.Get(1));
    csmaNodes678.Create(2);

//-----------------END CONTAINERS DEVICES-------------------

//-----------------START DEV PARAMS     p2p/csma-------------------
//creo interfacce di rete sui devices
    PointToPointHelper p2p100;
    p2p100.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p100.SetChannelAttribute("Delay", StringValue("15us"));
    
    PointToPointHelper p2p80;
    p2p80.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    p2p100.SetChannelAttribute("Delay", StringValue("5us"));
    
    //interfaces (devices) p2p
    NetDeviceContainer p2p23;
    p2p23 = p2p100.Install(p2pNodes23);

    NetDeviceContainer p2p34;
    p2p34 = p2p80.Install(p2pNodes34);
    
    NetDeviceContainer p2p35;
    p2p35 = p2p80.Install(p2pNodes35);
    
    NetDeviceContainer p2p46;
    p2p46 = p2p80.Install(p2pNodes46);
    
    NetDeviceContainer p2p56;
    p2p56 = p2p80.Install(p2pNodes56);


    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("25Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("10us"));

    //interfaces (devices) csma
    NetDeviceContainer csma0, csma2;
    csma0 = csma.Install(csmaNodes012);
    csma2 = csma.Install(csmaNodes678);

//-----------------END DEV PARAMS-------------------
//------ START - STACK INTERNET IP--------

    InternetStackHelper stack;
    stack.Install(csmaNodes012);        //n0, n1, n2
    stack.Install(csmaNodes678);        //n6, n7, n8
    stack.Install(p2pNodes35);          //n3, n5
    stack.Install(p2pNodes46.Get(0));   //n4

    //address p2p
    Ipv4AddressHelper p2p23Addr;
    p2p23Addr.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer p2p23Interfaces = p2p23Addr.Assign(p2p23);

    Ipv4AddressHelper p2p34Addr;
    p2p34Addr.SetBase("10.0.1.0", "255.255.255.252");
    Ipv4InterfaceContainer p2p34Interfaces = p2p34Addr.Assign(p2p34);

    Ipv4AddressHelper p2p35Addr;
    p2p35Addr.SetBase("10.0.2.0", "255.255.255.252");
    Ipv4InterfaceContainer p2p35Interfaces = p2p35Addr.Assign(p2p35);

    Ipv4AddressHelper p2p46Addr;
    p2p46Addr.SetBase("10.0.4.0", "255.255.255.252");
    Ipv4InterfaceContainer p2p46Interfaces = p2p46Addr.Assign(p2p46);

    Ipv4AddressHelper p2p56Addr;
    p2p56Addr.SetBase("10.0.3.0", "255.255.255.252");
    Ipv4InterfaceContainer p2p56Interfaces = p2p56Addr.Assign(p2p56);

    //address csma
    Ipv4AddressHelper addressCSMA1;
    addressCSMA1.SetBase("192.148.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesCSMA1 = addressCSMA1.Assign(csma0);

    Ipv4AddressHelper addressCSMA2;
    addressCSMA2.SetBase("192.148.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesCSMA2 = addressCSMA2.Assign(csma2);

//------ END - STACK INTERNET IP--------
    //routing ip
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

//---------- APPLICATION Layer CONFIGURATION ----------
    /*
    conf general:
        ASCII tracing solo client, server
        PCAP tracing solo n3, n6

    conf_0:
        TCP sink n0 port 2600
        TCP onOff client n8
            start 3s
            stop 15s
            size 1500 B

    conf_1:
        TCP sink n0 port 2600, n7 port 7777
        TCP onOff client n8 -> n0
            start 5s
            stop 15s
            size 2500 B
        TCP onOff client n1 -> n7
            start 2s
            stop 9s
            size 5000 B

    conf_2:
        UDP echo server n2 port 63
        UDP echo client n8
            5 packet: start 3s, 4s, 7s, 9s
            payload somma matricole in hex
            size 2560 B
        TCP sink n0 port 2600
        TCP onOff client n8 -> n0
            start 3s
            stop 9s
            size 3000 B
        UDP sink n7 port 2500
        UDP onOff client n8 -> n7
            start 5s
            stop 15s
            size 3000 B

    */
    switch(configuration){
        case 0:{

            //TCP sink n0
            uint16_t tcpSinkPort = 2600;
            Address sinkTCPaddr(InetSocketAddress(interfacesCSMA1.GetAddress(0), tcpSinkPort));
            PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory", sinkTCPaddr);
            ApplicationContainer tcpSinkAPP = tcpSinkHelper.Install(csmaNodes012.Get(0));

            tcpSinkAPP.Start(Seconds(0.0));
            tcpSinkAPP.Stop(Seconds(20.0));

            uint32_t onOffPSIZE = 1500;
            //onOff tcp n8 --> n0
            OnOffHelper tcpClient("ns3::TcpSocketFactory", Address());
            tcpClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")) ;
            tcpClient.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            AddressValue serverTcp(InetSocketAddress(interfacesCSMA1.GetAddress(0), tcpSinkPort));
            tcpClient.SetAttribute("Remote", serverTcp) ;
            tcpClient.SetAttribute("PacketSize", UintegerValue(onOffPSIZE));
            ApplicationContainer tcpOnOffApp = tcpClient.Install(csmaNodes678.Get(2)) ;
            tcpOnOffApp.Start (Seconds (3.0));
            tcpOnOffApp.Stop (Seconds (15.0));

            //tracing ascii
            AsciiTraceHelper ascii;
            csma.EnableAscii(ascii.CreateFileStream ("task1-0-n0.tr"), csma0.Get(0));
            csma.EnableAscii(ascii.CreateFileStream ("task1-0-n8.tr"), csma2.Get(2));

            //pcap tracing n3, n6
            p2p100.EnablePcap("task1-0-n3", p2p23.Get(1), true);//nic p2p 2-3 n3
            p2p80.EnablePcap("task1-0-n3", p2p34.Get(0), true);//nic p2p l0 n3
            p2p80.EnablePcap("task1-0-n3", p2p35.Get(0), true);//nic p2p l1 n3

            p2p100.EnablePcap("task1-0-n6", p2p56.Get(1), true);//nic p2p l2 n6
            p2p100.EnablePcap("task1-0-n6", p2p46.Get(1), true);//nic p2p l3 n6
            csma.EnablePcap("task1-0-n6", csma2.Get(0), true); //nic csma n6

        }break;
        case 1:{

            //TCP sink n0
            uint16_t tcpSinkPort = 2600;
            Address sinkTCPaddr(InetSocketAddress(interfacesCSMA1.GetAddress(0), tcpSinkPort));
            PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory", sinkTCPaddr);
            ApplicationContainer tcpSinkAPP = tcpSinkHelper.Install(csmaNodes012.Get(0));

            tcpSinkAPP.Start(Seconds(0.0));
            tcpSinkAPP.Stop(Seconds(20.0));

            //TCP sink n7
            uint16_t tcpSinkPort2 = 7777;
            Address sinkTCPaddr2(InetSocketAddress(interfacesCSMA2.GetAddress(1), tcpSinkPort2));
            PacketSinkHelper tcpSinkHelper2("ns3::TcpSocketFactory", sinkTCPaddr2);
            ApplicationContainer tcpSinkAPP2 = tcpSinkHelper2.Install(csmaNodes678.Get(1));

            tcpSinkAPP2.Start(Seconds(0.0));
            tcpSinkAPP2.Stop(Seconds(20.0));

            uint32_t onOffPSIZE = 2500;
            //onOff tcp n8 --> n0
            OnOffHelper tcpClient("ns3::TcpSocketFactory", Address());
            tcpClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")) ;
            tcpClient.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            AddressValue serverTcp(InetSocketAddress(interfacesCSMA1.GetAddress(0), tcpSinkPort));
            tcpClient.SetAttribute("Remote", serverTcp) ;
            tcpClient.SetAttribute("PacketSize", UintegerValue(onOffPSIZE));
            ApplicationContainer tcpOnOffApp = tcpClient.Install(csmaNodes678.Get(2)) ;
            tcpOnOffApp.Start (Seconds (5.0));
            tcpOnOffApp.Stop (Seconds (15.0));

            uint32_t onOffPSIZE2 = 5000;
            //onOff tcp n1 --> n7
            OnOffHelper tcpClient2("ns3::TcpSocketFactory", Address());
            tcpClient2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")) ;
            tcpClient2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            AddressValue serverTcp2(InetSocketAddress(interfacesCSMA2.GetAddress(1), tcpSinkPort2));
            tcpClient2.SetAttribute("Remote", serverTcp2) ;
            tcpClient2.SetAttribute("PacketSize", UintegerValue(onOffPSIZE2));
            ApplicationContainer tcpOnOffApp2 = tcpClient2.Install(csmaNodes012.Get(1)) ;
            tcpOnOffApp2.Start (Seconds (2.0));
            tcpOnOffApp2.Stop (Seconds (9.0));

            //tracing ascii
            AsciiTraceHelper ascii;
            csma.EnableAscii(ascii.CreateFileStream ("task1-1-n0.tr"), csma0.Get(0));
            csma.EnableAscii(ascii.CreateFileStream ("task1-1-n1.tr"), csma0.Get(1));
            csma.EnableAscii(ascii.CreateFileStream ("task1-1-n7.tr"), csma2.Get(1));
            csma.EnableAscii(ascii.CreateFileStream ("task1-1-n8.tr"), csma2.Get(2));

            //pcap tracing n3, n6
            p2p100.EnablePcap("task1-1-n3", p2p23.Get(1), true);//nic p2p 2-3 n3
            p2p80.EnablePcap("task1-1-n3", p2p34.Get(0), true);//nic p2p l0 n3
            p2p80.EnablePcap("task1-1-n3", p2p35.Get(0), true);//nic p2p l1 n3

            p2p100.EnablePcap("task1-1-n6", p2p56.Get(1), true);//nic p2p l2 n6
            p2p100.EnablePcap("task1-1-n6", p2p46.Get(1), true);//nic p2p l3 n6
            csma.EnablePcap("task1-1-n6", csma2.Get(0), true); //nic csma n6

        }break;
        case 2:{

            //UDP
            NS_LOG_INFO("");
            uint16_t udpEchoPort = 63;
            UdpEchoServerHelper echoServer(udpEchoPort); //n2 port 63
            ApplicationContainer echoApp =  echoServer.Install(csmaNodes012.Get(2));
            echoApp.Start(Seconds(0.0));
            echoApp.Stop(Seconds(20));

            UdpEchoClientHelper echoClient(interfacesCSMA1.GetAddress(2), udpEchoPort);
            echoClient.SetAttribute("MaxPackets", UintegerValue(5));
            echoClient.SetAttribute ("Interval", TimeValue (Seconds(2.0)));
            echoClient.SetAttribute ("PacketSize", UintegerValue(2560));
            //set payload 7708F9
            uint8_t * esa = (uint8_t *) calloc(2560, sizeof(char));
            std::string sum = "7708F9";
            int i = 0;
            while(sum[i] != 1){
                esa[i] = sum[i];
                i++;
            }
            
            //n8 udp echoClient
            uint32_t dataLength = 2560;
             


            echoApp = echoClient.Install(csmaNodes678.Get(2));
            
            echoApp.Start(Seconds(3.0));
            echoApp.Stop(Seconds(13.0));
            echoClient.SetFill(echoApp.Get(0),esa,sizeof(esa),dataLength);       
            free(esa);

            //TCP sink n0
            uint16_t tcpSinkPort = 2600;
            Address sinkTCPaddr(InetSocketAddress(interfacesCSMA1.GetAddress(0), tcpSinkPort));
            PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory", sinkTCPaddr);
            ApplicationContainer tcpSinkAPP = tcpSinkHelper.Install(csmaNodes012.Get(0));

            tcpSinkAPP.Start(Seconds(0.0));
            tcpSinkAPP.Stop(Seconds(20.0));


            //UDP sink n7
            uint16_t udpSinkPort = 2500;
            Address sinkUDPaddr(InetSocketAddress(interfacesCSMA2.GetAddress(1), udpSinkPort));
            PacketSinkHelper udpSinkHelper("ns3::UdpSocketFactory", sinkUDPaddr);
            ApplicationContainer udpSinkAPP = udpSinkHelper.Install(csmaNodes678.Get(1));

            udpSinkAPP.Start(Seconds(0.0));
            udpSinkAPP.Stop(Seconds(20.0));

            uint32_t onOffPSIZE = 3000;
            //onOff tcp n8 --> n0
            OnOffHelper tcpClient("ns3::TcpSocketFactory", Address());
            tcpClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")) ;
            tcpClient.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            AddressValue serverTcp(InetSocketAddress(interfacesCSMA1.GetAddress(0), tcpSinkPort));
            tcpClient.SetAttribute("Remote", serverTcp) ;
            tcpClient.SetAttribute("PacketSize", UintegerValue(onOffPSIZE));
            ApplicationContainer tcpOnOffApp = tcpClient.Install(csmaNodes678.Get(2)) ;
            tcpOnOffApp.Start (Seconds (3.0));
            tcpOnOffApp.Stop (Seconds (9.0));

            //onOff udp n8 --> n7
            OnOffHelper udpClient("ns3::UdpSocketFactory", Address());
            udpClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")) ;
            udpClient.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            AddressValue serverUdp(InetSocketAddress(interfacesCSMA2.GetAddress(1), udpSinkPort));
            udpClient.SetAttribute("Remote", serverUdp) ;
            udpClient.SetAttribute("PacketSize", UintegerValue(onOffPSIZE));
            ApplicationContainer udpOnOffApp = udpClient.Install(csmaNodes678.Get(2)) ;
            udpOnOffApp.Start(Seconds (5.0));
            udpOnOffApp.Stop(Seconds (15.0));

            //ascii tracing
            AsciiTraceHelper ascii;
            csma.EnableAscii(ascii.CreateFileStream ("task1-2-n0.tr"), csma0.Get(0));
            csma.EnableAscii(ascii.CreateFileStream ("task1-2-n2.tr"), csma0.Get(2));
            csma.EnableAscii(ascii.CreateFileStream ("task1-2-n7.tr"), csma2.Get(1));
            csma.EnableAscii(ascii.CreateFileStream ("task1-2-n8.tr"), csma2.Get(2));

            //pcap tracing n3, n6
            p2p100.EnablePcap("task1-2-n3", p2p23.Get(1), true);//nic p2p 2-3 n3
            p2p80.EnablePcap("task1-2-n3", p2p34.Get(0), true);//nic p2p l0 n3
            p2p80.EnablePcap("task1-2-n3", p2p35.Get(0), true);//nic p2p l1 n3

            p2p100.EnablePcap("task1-2-n6", p2p56.Get(1), true);//nic p2p l2 n6
            p2p100.EnablePcap("task1-2-n6", p2p46.Get(1), true);//nic p2p l3 n6
            csma.EnablePcap("task1-2-n6", csma2.Get(0), true); //nic csma n6

        }break;
        default: return EXIT_FAILURE;

    }

//------------------------------------------------   
    Simulator::Run ();
    Simulator::Stop(Seconds(20));
    Simulator::Destroy ();
    return 0;
}
