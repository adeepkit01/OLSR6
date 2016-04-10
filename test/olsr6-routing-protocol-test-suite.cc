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
#include "ns3/olsr6-routing-protocol.h"
#include "ns3/ipv6-header.h"

/********** Willingness **********/

/// Willingness for forwarding packets from other nodes: never.
#define OLSR6_WILL_NEVER         0
/// Willingness for forwarding packets from other nodes: low.
#define OLSR6_WILL_LOW           1
/// Willingness for forwarding packets from other nodes: medium.
#define OLSR6_WILL_DEFAULT       3
/// Willingness for forwarding packets from other nodes: high.
#define OLSR6_WILL_HIGH          6
/// Willingness for forwarding packets from other nodes: always.
#define OLSR6_WILL_ALWAYS        7

using namespace ns3;
using namespace olsr6;

/// Testcase for MPR computation mechanism
class Olsr6MprTestCase : public TestCase
{
public:
  Olsr6MprTestCase ();
  ~Olsr6MprTestCase ();
  /// \brief Run test case
  virtual void DoRun (void);
};


Olsr6MprTestCase::Olsr6MprTestCase ()
  : TestCase ("Check OLSR6 MPR computing mechanism")
{
}
Olsr6MprTestCase::~Olsr6MprTestCase ()
{
}
void
Olsr6MprTestCase::DoRun ()
{
  Ptr<RoutingProtocol> protocol = CreateObject<RoutingProtocol> ();
  protocol->m_mainAddress = Ipv6Address ("2001:1::1");
  Olsr6State & state = protocol->m_state;

  /*
   *  1 -- 2
   *  |    |
   *  3 -- 4
   *
   * Node 1 must select only one MPR (2 or 3, doesn't matter)
   */
  NeighborTuple neigbor;
  neigbor.status = NeighborTuple::STATUS_SYM;
  neigbor.willingness = OLSR6_WILL_DEFAULT;
  neigbor.neighborMainAddr = Ipv6Address ("2001:1::2");
  protocol->m_state.InsertNeighborTuple (neigbor);
  neigbor.neighborMainAddr = Ipv6Address ("2001:1::3");
  protocol->m_state.InsertNeighborTuple (neigbor);
  TwoHopNeighborTuple tuple;
  tuple.expirationTime = Seconds (3600);
  tuple.neighborMainAddr = Ipv6Address ("2001:1::2");
  tuple.twoHopNeighborAddr = Ipv6Address ("2001:1::4");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);
  tuple.neighborMainAddr = Ipv6Address ("2001:1::3");
  tuple.twoHopNeighborAddr = Ipv6Address ("2001:1::4");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  NS_TEST_EXPECT_MSG_EQ (state.GetMprSet ().size (), 1, "An only address must be chosen.");
  /*
   *  1 -- 2 -- 5
   *  |    |
   *  3 -- 4
   *
   * Node 1 must select node 2 as MPR.
   */
  tuple.neighborMainAddr = Ipv6Address ("2001:1::2");
  tuple.twoHopNeighborAddr = Ipv6Address ("2001:1::5");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  MprSet mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 1, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("2001:1::2") != mpr.end ()), true, "Node 1 must select node 2 as MPR");
  /*
   *  1 -- 2 -- 5
   *  |    |
   *  3 -- 4
   *  |
   *  6
   *
   * Node 1 must select nodes 2 and 3 as MPRs.
   */
  tuple.neighborMainAddr = Ipv6Address ("2001:1::3");
  tuple.twoHopNeighborAddr = Ipv6Address ("2001:1::6");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 2, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("2001:1::2") != mpr.end ()), true, "Node 1 must select node 2 as MPR");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("2001:1::3") != mpr.end ()), true, "Node 1 must select node 3 as MPR");
  /*
   *  7 (OLSR6_WILL_ALWAYS)
   *  |
   *  1 -- 2 -- 5
   *  |    |
   *  3 -- 4
   *  |
   *  6
   *
   * Node 1 must select nodes 2, 3 and 7 (since it is WILL_ALWAYS) as MPRs.
   */
  neigbor.willingness = OLSR6_WILL_ALWAYS;
  neigbor.neighborMainAddr = Ipv6Address ("2001:1::7");
  protocol->m_state.InsertNeighborTuple (neigbor);

  protocol->MprComputation ();
  mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 3, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("2001:1::7") != mpr.end ()), true, "Node 1 must select node 7 as MPR");
  /*
   *                7 <- WILL_ALWAYS
   *                |
   *      9 -- 8 -- 1 -- 2 -- 5
   *                |    |
   *           ^    3 -- 4
   *           |    |
   *   WILL_NEVER   6
   *
   * Node 1 must select nodes 2, 3 and 7 (since it is WILL_ALWAYS) as MPRs.
   * Node 1 must NOT select node 8 as MPR since it is WILL_NEVER
   */
  neigbor.willingness = OLSR6_WILL_NEVER;
  neigbor.neighborMainAddr = Ipv6Address ("2001:1::8");
  protocol->m_state.InsertNeighborTuple (neigbor);
  tuple.neighborMainAddr = Ipv6Address ("2001:1::8");
  tuple.twoHopNeighborAddr = Ipv6Address ("2001:1::9");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 3, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("2001:1::9") == mpr.end ()), true, "Node 1 must NOT select node 8 as MPR");
}

static class Olsr6ProtocolTestSuite : public TestSuite
{
public:
  Olsr6ProtocolTestSuite ();
} g_olsr6ProtocolTestSuite;

Olsr6ProtocolTestSuite::Olsr6ProtocolTestSuite ()
  : TestSuite ("routing-olsr6", UNIT)
{
  AddTestCase (new Olsr6MprTestCase (), TestCase::QUICK);
}
