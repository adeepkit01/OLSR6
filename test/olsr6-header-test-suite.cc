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


#include "ns3/test.h"
#include "ns3/olsr6-header.h"
#include "ns3/packet.h"

using namespace ns3;

class Olsr6EmfTestCase : public TestCase
{
public:
  Olsr6EmfTestCase ();
  virtual void DoRun (void);
};

Olsr6EmfTestCase::Olsr6EmfTestCase ()
  : TestCase ("Check Emf olsr6 time conversion")
{
}
void
Olsr6EmfTestCase::DoRun (void)
{
  for (int time = 1; time <= 30; time++)
    {
      uint8_t emf = olsr6::SecondsToEmf (time);
      double seconds = olsr6::EmfToSeconds (emf);
      NS_TEST_ASSERT_MSG_EQ ((seconds < 0 || std::fabs (seconds - time) > 0.1), false,
                             "100");
    }
}


class Olsr6MidTestCase : public TestCase
{
public:
  Olsr6MidTestCase ();
  virtual void DoRun (void);
};

Olsr6MidTestCase::Olsr6MidTestCase ()
  : TestCase ("Check Mid olsr6 messages")
{
}
void
Olsr6MidTestCase::DoRun (void)
{
  Packet packet;

  {
    olsr6::PacketHeader hdr;
    olsr6::MessageHeader msg1;
    olsr6::MessageHeader::Mid &mid1 = msg1.GetMid ();
    olsr6::MessageHeader msg2;
    olsr6::MessageHeader::Mid &mid2 = msg2.GetMid ();

    // MID message #1
    {
      std::vector<Ipv6Address> &addresses = mid1.interfaceAddresses;
      addresses.clear ();
      addresses.push_back (Ipv6Address ("2001:1::4"));
      addresses.push_back (Ipv6Address ("2001:1::5"));
    }

    msg1.SetTimeToLive (255);
    msg1.SetOriginatorAddress (Ipv6Address ("2001:2::44"));
    msg1.SetVTime (Seconds (9));
    msg1.SetMessageSequenceNumber (7);

    // MID message #2
    {
      std::vector<Ipv6Address> &addresses = mid2.interfaceAddresses;
      addresses.clear ();
      addresses.push_back (Ipv6Address ("2001:3::4"));
      addresses.push_back (Ipv6Address ("2001:3::5"));
    }

    msg2.SetTimeToLive (254);
    msg2.SetOriginatorAddress (Ipv6Address ("2001:4::44"));
    msg2.SetVTime (Seconds (10));
    msg2.SetMessageType (olsr6::MessageHeader::MID_MESSAGE);
    msg2.SetMessageSequenceNumber (7);

    // Build an OLSR6 packet header
    hdr.SetPacketLength (hdr.GetSerializedSize () + msg1.GetSerializedSize () + msg2.GetSerializedSize ());
    hdr.SetPacketSequenceNumber (123);


    // Now add all the headers in the correct order
    packet.AddHeader (msg2);
    packet.AddHeader (msg1);
    packet.AddHeader (hdr);
  }

  {
    olsr6::PacketHeader hdr;
    packet.RemoveHeader (hdr);
    NS_TEST_ASSERT_MSG_EQ (hdr.GetPacketSequenceNumber (), 123, "200");
    uint32_t sizeLeft = hdr.GetPacketLength () - hdr.GetSerializedSize ();
    {
      olsr6::MessageHeader msg1;

      packet.RemoveHeader (msg1);

      NS_TEST_ASSERT_MSG_EQ (msg1.GetTimeToLive (),  255, "201");
      NS_TEST_ASSERT_MSG_EQ (msg1.GetOriginatorAddress (), Ipv6Address ("2001:2::44"), "202");
      NS_TEST_ASSERT_MSG_EQ (msg1.GetVTime (), Seconds (9), "203");
      NS_TEST_ASSERT_MSG_EQ (msg1.GetMessageType (), olsr6::MessageHeader::MID_MESSAGE, "204");
      NS_TEST_ASSERT_MSG_EQ (msg1.GetMessageSequenceNumber (), 7, "205");

      olsr6::MessageHeader::Mid &mid1 = msg1.GetMid ();
      NS_TEST_ASSERT_MSG_EQ (mid1.interfaceAddresses.size (), 2, "206");
      NS_TEST_ASSERT_MSG_EQ (*mid1.interfaceAddresses.begin (), Ipv6Address ("2001:1::4"), "207");

      sizeLeft -= msg1.GetSerializedSize ();
      NS_TEST_ASSERT_MSG_EQ ((sizeLeft > 0), true, "208");
    }
    {
      // now read the second message
      olsr6::MessageHeader msg2;

      packet.RemoveHeader (msg2);

      NS_TEST_ASSERT_MSG_EQ (msg2.GetTimeToLive (),  254, "209");
      NS_TEST_ASSERT_MSG_EQ (msg2.GetOriginatorAddress (), Ipv6Address ("2001:4::44"), "210");
      NS_TEST_ASSERT_MSG_EQ (msg2.GetVTime (), Seconds (10), "211");
      NS_TEST_ASSERT_MSG_EQ (msg2.GetMessageType (), olsr6::MessageHeader::MID_MESSAGE, "212");
      NS_TEST_ASSERT_MSG_EQ (msg2.GetMessageSequenceNumber (), 7, "213");

      olsr6::MessageHeader::Mid mid2 = msg2.GetMid ();
      NS_TEST_ASSERT_MSG_EQ (mid2.interfaceAddresses.size (), 2, "214");
      NS_TEST_ASSERT_MSG_EQ (*mid2.interfaceAddresses.begin (), Ipv6Address ("2001:3::4"), "215");

      sizeLeft -= msg2.GetSerializedSize ();
      NS_TEST_ASSERT_MSG_EQ (sizeLeft, 0, "216");
    }
  }
}


class Olsr6HelloTestCase : public TestCase
{
public:
  Olsr6HelloTestCase ();
  virtual void DoRun (void);
};

Olsr6HelloTestCase::Olsr6HelloTestCase ()
  : TestCase ("Check Hello olsr6 messages")
{
}
void
Olsr6HelloTestCase::DoRun (void)
{
  Packet packet;
  olsr6::MessageHeader msgIn;
  olsr6::MessageHeader::Hello &helloIn = msgIn.GetHello ();

  helloIn.SetHTime (Seconds (7));
  helloIn.willingness = 66;

  {
    olsr6::MessageHeader::Hello::LinkMessage lm1;
    lm1.linkCode = 2;
    lm1.neighborInterfaceAddresses.push_back (Ipv6Address ("2001:1::4"));
    lm1.neighborInterfaceAddresses.push_back (Ipv6Address ("2001:1::5"));
    helloIn.linkMessages.push_back (lm1);

    olsr6::MessageHeader::Hello::LinkMessage lm2;
    lm2.linkCode = 3;
    lm2.neighborInterfaceAddresses.push_back (Ipv6Address ("2001:3::4"));
    lm2.neighborInterfaceAddresses.push_back (Ipv6Address ("2001:3::5"));
    helloIn.linkMessages.push_back (lm2);
  }

  packet.AddHeader (msgIn);

  olsr6::MessageHeader msgOut;
  packet.RemoveHeader (msgOut);
  olsr6::MessageHeader::Hello &helloOut = msgOut.GetHello ();

  NS_TEST_ASSERT_MSG_EQ (helloOut.GetHTime (), Seconds (7), "300");
  NS_TEST_ASSERT_MSG_EQ (helloOut.willingness, 66, "301");
  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages.size (), 2, "302");

  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages[0].linkCode, 2, "303");
  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages[0].neighborInterfaceAddresses[0],
                         Ipv6Address ("2001:1::4"), "304");
  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages[0].neighborInterfaceAddresses[1],
                         Ipv6Address ("2001:1::5"), "305");

  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages[1].linkCode, 3, "306");
  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages[1].neighborInterfaceAddresses[0],
                         Ipv6Address ("2001:3::4"), "307");
  NS_TEST_ASSERT_MSG_EQ (helloOut.linkMessages[1].neighborInterfaceAddresses[1],
                         Ipv6Address ("2001:3::5"), "308");

  NS_TEST_ASSERT_MSG_EQ (packet.GetSize (), 0, "All bytes in packet were not read");

}

class Olsr6TcTestCase : public TestCase
{
public:
  Olsr6TcTestCase ();
  virtual void DoRun (void);
};

Olsr6TcTestCase::Olsr6TcTestCase ()
  : TestCase ("Check Tc olsr6 messages")
{
}
void
Olsr6TcTestCase::DoRun (void)
{
  Packet packet;
  olsr6::MessageHeader msgIn;
  olsr6::MessageHeader::Tc &tcIn = msgIn.GetTc ();

  tcIn.ansn = 0x1234;
  tcIn.neighborAddresses.push_back (Ipv6Address ("2001:1::4"));
  tcIn.neighborAddresses.push_back (Ipv6Address ("2001:1::5"));
  packet.AddHeader (msgIn);

  olsr6::MessageHeader msgOut;
  packet.RemoveHeader (msgOut);
  olsr6::MessageHeader::Tc &tcOut = msgOut.GetTc ();

  NS_TEST_ASSERT_MSG_EQ (tcOut.ansn, 0x1234, "400");
  NS_TEST_ASSERT_MSG_EQ (tcOut.neighborAddresses.size (), 2, "401");

  NS_TEST_ASSERT_MSG_EQ (tcOut.neighborAddresses[0],
                         Ipv6Address ("2001:1::4"), "402");
  NS_TEST_ASSERT_MSG_EQ (tcOut.neighborAddresses[1],
                         Ipv6Address ("2001:1::5"), "403");

  NS_TEST_ASSERT_MSG_EQ (packet.GetSize (), 0, "404");

}

class Olsr6HnaTestCase : public TestCase
{
public:
  Olsr6HnaTestCase ();
  virtual void DoRun (void);
};

Olsr6HnaTestCase::Olsr6HnaTestCase ()
  : TestCase ("Check Hna olsr6 messages")
{
}

void
Olsr6HnaTestCase::DoRun (void)
{
  Packet packet;
  olsr6::MessageHeader msgIn;
  olsr6::MessageHeader::Hna &hnaIn = msgIn.GetHna ();

  hnaIn.associations.push_back ((olsr6::MessageHeader::Hna::Association)
                                { Ipv6Address ("2001:1::4"), Ipv6Prefix (64)});
  hnaIn.associations.push_back ((olsr6::MessageHeader::Hna::Association)
                                { Ipv6Address ("2001:1::5"), Ipv6Prefix (64)});
  packet.AddHeader (msgIn);

  olsr6::MessageHeader msgOut;
  packet.RemoveHeader (msgOut);
  olsr6::MessageHeader::Hna &hnaOut = msgOut.GetHna ();

  NS_TEST_ASSERT_MSG_EQ (hnaOut.associations.size (), 2, "500");

  NS_TEST_ASSERT_MSG_EQ (hnaOut.associations[0].address,
                         Ipv6Address ("2001:1::4"), "501");
  NS_TEST_ASSERT_MSG_EQ (hnaOut.associations[0].mask,
                         Ipv6Prefix (64), "502");

  NS_TEST_ASSERT_MSG_EQ (hnaOut.associations[1].address,
                         Ipv6Address ("2001:1::5"), "503");
  NS_TEST_ASSERT_MSG_EQ (hnaOut.associations[1].mask,
                         Ipv6Prefix (64), "504");

  NS_TEST_ASSERT_MSG_EQ (packet.GetSize (), 0, "All bytes in packet were not read");

}


static class Olsr6TestSuite : public TestSuite
{
public:
  Olsr6TestSuite ();
} g_olsr6TestSuite;

Olsr6TestSuite::Olsr6TestSuite ()
  : TestSuite ("routing-olsr6-header", UNIT)
{
  AddTestCase (new Olsr6HnaTestCase (), TestCase::QUICK);
  AddTestCase (new Olsr6TcTestCase (), TestCase::QUICK);
  AddTestCase (new Olsr6HelloTestCase (), TestCase::QUICK);
  AddTestCase (new Olsr6MidTestCase (), TestCase::QUICK);
  AddTestCase (new Olsr6EmfTestCase (), TestCase::QUICK);
}
