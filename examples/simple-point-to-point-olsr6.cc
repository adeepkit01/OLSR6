/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
 *
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
 * Authors: Ankit Deepak <adadeepak8@gmail.com>
 *          Shravya Ks <shravya.ks0@gmail.com>,
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

/*
 * PORT NOTE: This code was ported from ns-3 IPv4 implementation  (src/olsr).  Almost all
 * comments have also been ported from the same
 */


//
// Simple example of OLSR6 routing over some point-to-point links
//
// Network topology
//
//   n0
//     \ 5 Mb/s, 2ms
//      \          1.5Mb/s, 10ms
//       n2 -------------------------n3---------n4
//      /
//     / 5 Mb/s, 2ms
//   n1
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n4, and from n3 to n1
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues
// - Tracing of queues and packet receptions to file "simple-point-to-point-olsr6.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr6-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-list-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimplePointToPointOlsr6Example");

int
main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if 0
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

  // Set up some default values for the simulation.  Use the

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (5);
  NodeContainer n02 = NodeContainer (c.Get (0), c.Get (2));
  NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n32 = NodeContainer (c.Get (3), c.Get (2));
  NodeContainer n34 = NodeContainer (c.Get (3), c.Get (4));

  // Enable OLSR6
  NS_LOG_INFO ("Enabling OLSR6 Routing.");
  Olsr6Helper olsr6;

  Ipv6StaticRoutingHelper staticRouting;

  Ipv6ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr6, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer nd02 = p2p.Install (n02);
  NetDeviceContainer nd12 = p2p.Install (n12);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1500kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer nd32 = p2p.Install (n32);
  NetDeviceContainer nd34 = p2p.Install (n34);

  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv6AddressHelper ipv6;
  ipv6.SetBase ("2001:1::", Ipv6Prefix (64));
  Ipv6InterfaceContainer i02 = ipv6.Assign (nd02);

  ipv6.SetBase ("2001:2::", Ipv6Prefix (64));
  Ipv6InterfaceContainer i12 = ipv6.Assign (nd12);

  ipv6.SetBase ("2001:3::", Ipv6Prefix (64));
  Ipv6InterfaceContainer i32 = ipv6.Assign (nd32);

  ipv6.SetBase ("2001:4::", Ipv6Prefix (64));
  Ipv6InterfaceContainer i34 = ipv6.Assign (nd34);

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s from n0 to n4
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)

  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     Inet6SocketAddress (i34.GetAddress (1,0), port));
  onoff.SetConstantRate (DataRate ("448kb/s"));

  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Inet6SocketAddress (Ipv6Address::GetAny (), port));

  apps = sink.Install (c.Get (3));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  // Create a similar flow from n3 to n1, starting at time 1.1 seconds
  onoff.SetAttribute ("Remote",
                      AddressValue (Inet6SocketAddress (i12.GetAddress (0,0), port)));
  apps = onoff.Install (c.Get (3));
  apps.Start (Seconds (1.1));
  apps.Stop (Seconds (10.0));

  // Create a packet sink to receive these packets
  apps = sink.Install (c.Get (1));
  apps.Start (Seconds (1.1));
  apps.Stop (Seconds (10.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-point-to-point-olsr6.tr"));
  p2p.EnablePcapAll ("simple-point-to-point-olsr6");

  Simulator::Stop (Seconds (30));

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
