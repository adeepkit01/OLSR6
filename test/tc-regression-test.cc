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


#include "tc-regression-test.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/olsr6-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/abort.h"
#include "ns3/socket-factory.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/olsr6-header.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"

namespace ns3 {
namespace olsr6 {

TcRegressionTest::TcRegressionTest ()
  : TestCase ("Test OLSR6 Topology Control message generation"),
    m_time (Seconds (20)),
    m_countA (0),
    m_countB (0),
    m_countC (0)
{
}

TcRegressionTest::~TcRegressionTest ()
{
}

void
TcRegressionTest::DoRun ()
{
  RngSeedManager::SetSeed (12345);
  RngSeedManager::SetRun (7);
  CreateNodes ();

  Simulator::Stop (m_time);
  Simulator::Run ();

  m_rxSocketA = 0;
  m_rxSocketB = 0;
  m_rxSocketC = 0;
  Simulator::Destroy ();
}

void
TcRegressionTest::CreateNodes ()
{
  // create 3 nodes
  NodeContainer c;
  c.Create (3);

  // install TCP/IP & OLSR6
  Olsr6Helper olsr6;
  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr6);
  internet.Install (c);
  int64_t streamsUsed = olsr6.AssignStreams (c, 0);
  NS_TEST_EXPECT_MSG_EQ (streamsUsed, 3, "Should have assigned 3 streams");

  // create channel & devices
  SimpleNetDeviceHelper simpleNetHelper;
  simpleNetHelper.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  simpleNetHelper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer nd = simpleNetHelper.Install (c);

  // Blacklist some devices (equivalent to Wireless out of range)
  Ptr<SimpleNetDevice> nd0 = DynamicCast<SimpleNetDevice> (nd.Get (0));
  Ptr<SimpleNetDevice> nd2 = DynamicCast<SimpleNetDevice> (nd.Get (2));
  Ptr<SimpleChannel> ch = DynamicCast<SimpleChannel> (nd.Get (0)->GetChannel ());
  ch->BlackList (nd0, nd2);
  ch->BlackList (nd2, nd0);

  // setup IP addresses
  Ipv6AddressHelper ipv6;
  ipv6.SetBase ("2001:1::", Ipv6Prefix (64));
  ipv6.Assign (nd);

  // Create the sockets
  Ptr<SocketFactory> rxSocketFactoryA = c.Get (0)->GetObject<Ipv6RawSocketFactory> ();
  m_rxSocketA = DynamicCast<Ipv6RawSocketImpl> (rxSocketFactoryA->CreateSocket ());
  m_rxSocketA->SetProtocol (UdpL4Protocol::PROT_NUMBER);
  m_rxSocketA->SetRecvCallback (MakeCallback (&TcRegressionTest::ReceivePktProbeA, this));

  Ptr<SocketFactory> rxSocketFactoryB = c.Get (1)->GetObject<Ipv6RawSocketFactory> ();
  m_rxSocketB = DynamicCast<Ipv6RawSocketImpl> (rxSocketFactoryB->CreateSocket ());
  m_rxSocketB->SetProtocol (UdpL4Protocol::PROT_NUMBER);
  m_rxSocketB->SetRecvCallback (MakeCallback (&TcRegressionTest::ReceivePktProbeB, this));

  Ptr<SocketFactory> rxSocketFactoryC = c.Get (2)->GetObject<Ipv6RawSocketFactory> ();
  m_rxSocketC = DynamicCast<Ipv6RawSocketImpl> (rxSocketFactoryC->CreateSocket ());
  m_rxSocketC->SetProtocol (UdpL4Protocol::PROT_NUMBER);
  m_rxSocketC->SetRecvCallback (MakeCallback (&TcRegressionTest::ReceivePktProbeC, this));
}

// Note: this is identical to ReceivePktProbeC, but the packet counter needs to be different.
void
TcRegressionTest::ReceivePktProbeA (Ptr<Socket> socket)
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

  if (m_countA == 0)
    {
      MessageHeader msgHdr;
      receivedPacketProbe->RemoveHeader (msgHdr);
      const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
      NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 0, "0 - Hello, No messages.");
    }
  else if (m_countA == 1)
    {
      MessageHeader msgHdr;
      receivedPacketProbe->RemoveHeader (msgHdr);
      NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
      const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 2, "1 - Hello, one message.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 1, "1 - Asymmetric Link.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses.size (), 1, "1 - Neighbor.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses[0], Ipv6Address (""), "1 - Neighbor.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].linkCode, 1, "1 - Asymmetric Link.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses.size (), 1, "1 - Neighbor.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses[0], Ipv6Address (""), "1 - Neighbor.");
    }
  else
    {
      if (m_countA == 5 || m_countA == 8)
        {
          MessageHeader msgHdr;
          receivedPacketProbe->RemoveHeader (msgHdr);
          const olsr6::MessageHeader::Tc &tc = msgHdr.GetTc ();
          NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
          NS_TEST_EXPECT_MSG_EQ (tc.neighborAddresses.size (), 2, int(m_countA) << " - TC, one message.");
          NS_TEST_EXPECT_MSG_EQ (tc.neighborAddresses[0], Ipv6Address (""), int(m_countA) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (tc.neighborAddresses[1], Ipv6Address (""), int(m_countA) << " - Neighbor.");
        }
      if (m_countA != 8)
        {
          MessageHeader msgHdr;
          receivedPacketProbe->RemoveHeader (msgHdr);
          NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
          const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 2, int(m_countA) << " - Hello, one message.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 6, int(m_countA) << " - Symmetric Link.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses.size (), 1, int(m_countA) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses[0], Ipv6Address (""), int(m_countA) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].linkCode, 6, int(m_countA) << " - Symmetric Link.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses.size (), 1, int(m_countA) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses[0], Ipv6Address (""), int(m_countA) << " - Neighbor.");
        }
    }
  m_countA++;
}

void
TcRegressionTest::ReceivePktProbeB (Ptr<Socket> socket)
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

  if (m_countB == 0 || m_countB == 2 || m_countB == 5 || m_countB == 6 || m_countB == 8
      || m_countB == 10 || m_countB == 13 || m_countB == 15 || m_countB == 17 || m_countB == 19)
    {
      NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
    }

  if (m_countB == 0 || m_countB == 1)
    {
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 0, int(m_countC) << " - Hello, links announced.");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 1, int(m_countC) << " - Hello, links announced.");
      if (m_countB == 2 || m_countB == 3)
        {
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 1, int(m_countC) << " - Asymmetric Link.");
        }
      else if (m_countB == 4 || m_countB == 5)
        {
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 6, int(m_countC) << " - Symmetric Link.");
        }
      else
        {
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 10, int(m_countC) << " - MPR Link.");
        }

      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses[0], Ipv6Address (""), int(m_countC) << " - Neighbor.");
    }

  m_countB++;
}

// Note: this is identical to ReceivePktProbeA, but the packet counter needs to be different.
void
TcRegressionTest::ReceivePktProbeC (Ptr<Socket> socket)
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

  if (m_countC == 0)
    {
      MessageHeader msgHdr;
      receivedPacketProbe->RemoveHeader (msgHdr);
      const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
      NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 0, "0 - Hello, No messages.");
    }
  else if (m_countC == 1)
    {
      MessageHeader msgHdr;
      receivedPacketProbe->RemoveHeader (msgHdr);
      NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
      const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 2, "1 - Hello, one message.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 1, "1 - Asymmetric Link.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses.size (), 1, "1 - Neighbor.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses[0], Ipv6Address (""), "1 - Neighbor.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].linkCode, 1, "1 - Asymmetric Link.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses.size (), 1, "1 - Neighbor.");
      NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses[0], Ipv6Address (""), "1 - Neighbor.");
    }
  else
    {
      if (m_countC == 5 || m_countC == 8)
        {
          MessageHeader msgHdr;
          receivedPacketProbe->RemoveHeader (msgHdr);
          const olsr6::MessageHeader::Tc &tc = msgHdr.GetTc ();
          NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
          NS_TEST_EXPECT_MSG_EQ (tc.neighborAddresses.size (), 2, int(m_countC) << " - TC, one message.");
          NS_TEST_EXPECT_MSG_EQ (tc.neighborAddresses[0], Ipv6Address (""), int(m_countC) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (tc.neighborAddresses[1], Ipv6Address (""), int(m_countC) << " - Neighbor.");
        }
      if (m_countC != 8)
        {
          MessageHeader msgHdr;
          receivedPacketProbe->RemoveHeader (msgHdr);
          NS_TEST_EXPECT_MSG_EQ (msgHdr.GetOriginatorAddress (), Ipv6Address (""), "Originator address.");
          const olsr6::MessageHeader::Hello &hello = msgHdr.GetHello ();
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages.size (), 2, int(m_countC) << " - Hello, one message.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].linkCode, 6, int(m_countC) << " - Symmetric Link.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses.size (), 1, int(m_countC) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[0].neighborInterfaceAddresses[0], Ipv6Address (""), int(m_countC) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].linkCode, 6, int(m_countC) << " - Symmetric Link.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses.size (), 1, int(m_countC) << " - Neighbor.");
          NS_TEST_EXPECT_MSG_EQ (hello.linkMessages[1].neighborInterfaceAddresses[0], Ipv6Address (""), int(m_countC) << " - Neighbor.");
        }
    }
  m_countC++;
}

}
}
