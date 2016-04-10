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


#include <vector>
#include "hello-regression-test.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/olsr6-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/abort.h"
#include "ns3/socket-factory.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/olsr6-header.h"

namespace ns3 {
namespace olsr6 {

HelloRegressionTest::HelloRegressionTest ()
  : TestCase ("Test OLSR6 Hello messages generation"),
    m_time (Seconds (5)),
    m_countA (0),
    m_countB (0)
{
}

HelloRegressionTest::~HelloRegressionTest ()
{
}

void
HelloRegressionTest::DoRun ()
{
  RngSeedManager::SetSeed (12345);
  RngSeedManager::SetRun (7);
  CreateNodes ();

  Simulator::Stop (m_time);
  Simulator::Run ();

  m_rxSocketA = 0;
  m_rxSocketB = 0;
  Simulator::Destroy ();
}

void
HelloRegressionTest::CreateNodes ()
{
  // create 2 nodes
  NodeContainer c;
  c.Create (2);
  // install TCP/IP & OLSR6
  Olsr6Helper olsr6;
  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr6);
  internet.Install (c);
  // Assign OLSR6 RVs to specific streams
  int64_t streamsUsed = olsr6.AssignStreams (c, 0);
  NS_TEST_ASSERT_MSG_EQ (streamsUsed, 2, "Should have assigned 2 streams");
  // create channel & devices
  SimpleNetDeviceHelper simpleNetHelper;
  simpleNetHelper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  simpleNetHelper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer nd = simpleNetHelper.Install (c);
  // setup IP addresses
  Ipv6AddressHelper ipv6;
  ipv6.SetBase ("2001:1::", Ipv6Prefix (64));
  ipv6.Assign (nd);

  // Create the sockets
  Ptr<SocketFactory> rxSocketFactoryA = c.Get (0)->GetObject<Ipv6RawSocketFactory> ();
  m_rxSocketA = DynamicCast<Ipv6RawSocketImpl> (rxSocketFactoryA->CreateSocket ());
  m_rxSocketA->SetProtocol (UdpL4Protocol::PROT_NUMBER);
  m_rxSocketA->SetRecvCallback (MakeCallback (&HelloRegressionTest::ReceivePktProbeA, this));

  Ptr<SocketFactory> rxSocketFactoryB = c.Get (1)->GetObject<Ipv6RawSocketFactory> ();
  m_rxSocketB = DynamicCast<Ipv6RawSocketImpl> (rxSocketFactoryB->CreateSocket ());
  m_rxSocketB->SetProtocol (UdpL4Protocol::PROT_NUMBER);
  m_rxSocketB->SetRecvCallback (MakeCallback (&HelloRegressionTest::ReceivePktProbeB, this));
}

void
HelloRegressionTest::ReceivePktProbeA (Ptr<Socket> socket)
{
  uint32_t availableData;
  availableData = socket->GetRxAvailable ();
  Ptr<Packet> receivedPacketProbe = socket->Recv (std::numeric_limits<uint32_t>::max (), 0);
  NS_ASSERT (availableData == receivedPacketProbe->GetSize ());

  Ipv6Header ipHdr;
  receivedPacketProbe->RemoveHeader (ipHdr);
  UdpHeader udpHdr;
  receivedPacketProbe->RemoveHeader (udpHdr);
  PacketHeader pktHdr;
  receivedPacketProbe->RemoveHeader (pktHdr);
  MessageHeader msgHdr;
  receivedPacketProbe->RemoveHeader (msgHdr);

  const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
  NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address ("::"), "Originator address.");

  if (m_countA == 0)
    {
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 0, "No Link messages on the first Hello.");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 1, "One Link message on the second and third Hello.");
    }

  std::vector<olsr6::MessageHeader::Hello::LinkMessage>::const_iterator iter;
  for (iter = hello.linkMessages.begin (); iter != hello.linkMessages.end (); iter++)
    {
      if (m_countA == 1)
        {
          NS_TEST_EXPECT_MSG_EQ (iter->linkCode, 1, "Asymmetric link on second Hello.");
        }
      else
        {
          NS_TEST_EXPECT_MSG_EQ (iter->linkCode, 6, "Symmetric link on second Hello.");
        }

      NS_TEST_EXPECT_MSG_EQ (iter->neighborInterfaceAddresses.size (), 1, "Only one neighbor.");
      NS_TEST_EXPECT_MSG_EQ (iter->neighborInterfaceAddresses[0], Ipv6Address ("::"), "Only one neighbor.");
    }

  m_countA++;
}

void
HelloRegressionTest::ReceivePktProbeB (Ptr<Socket> socket)
{
  uint32_t availableData;
  availableData = socket->GetRxAvailable ();
  Ptr<Packet> receivedPacketProbe = socket->Recv (std::numeric_limits<uint32_t>::max (), 0);
  NS_ASSERT (availableData == receivedPacketProbe->GetSize ());

  Ipv6Header ipHdr;
  receivedPacketProbe->RemoveHeader (ipHdr);
  UdpHeader udpHdr;
  receivedPacketProbe->RemoveHeader (udpHdr);
  PacketHeader pktHdr;
  receivedPacketProbe->RemoveHeader (pktHdr);
  MessageHeader msgHdr;
  receivedPacketProbe->RemoveHeader (msgHdr);

  const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
  NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address ("::"), "Originator address.");

  if (m_countA == 0)
    {
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 0, "No Link messages on the first Hello.");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 1, "One Link message on the second and third Hello.");
    }

  std::vector<olsr6::MessageHeader::Hello::LinkMessage>::const_iterator iter;
  for (iter = hello.linkMessages.begin (); iter != hello.linkMessages.end (); iter++)
    {
      if (m_countA == 1)
        {
          NS_TEST_EXPECT_MSG_EQ (iter->linkCode, 1, "Asymmetric link on second Hello.");
        }
      else
        {
          NS_TEST_EXPECT_MSG_EQ (iter->linkCode, 6, "Symmetric link on second Hello.");
        }

      NS_TEST_EXPECT_MSG_EQ (iter->neighborInterfaceAddresses.size (), 1, "Only one neighbor.");
      NS_TEST_EXPECT_MSG_EQ (iter->neighborInterfaceAddresses[0], Ipv6Address ("::"), "Only one neighbor.");
    }

  m_countB++;
}

}
}
