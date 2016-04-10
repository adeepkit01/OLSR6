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


#ifndef TC_REGRESSION_TEST_H
#define TC_REGRESSION_TEST_H

#include "ns3/test.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/ipv6-raw-socket-impl.h"
#include "ns3/node-container.h"

namespace ns3 {
namespace olsr6 {
/**
 * \ingroup olsr6
 * \brief Less trivial test of OLSR6 Topology Control message generation
 *
 * This test simulates 3 Wi-Fi stations with chain topology and runs OLSR6 without any extra traffic.
 * It is expected that only second station will send TC messages.
 *
 */
class TcRegressionTest : public TestCase
{
public:
  TcRegressionTest ();
  ~TcRegressionTest ();
private:
  /// Total simulation time
  const Time m_time;
  /// Create & configure test network
  void CreateNodes ();
  /// Go
  void DoRun ();

  /// Receive raw data on node A
  void ReceivePktProbeA (Ptr<Socket> socket);
  /// Packet counter on node A
  uint8_t m_countA;
  /// Receiving socket on node A
  Ptr<Ipv6RawSocketImpl> m_rxSocketA;

  /// Receive raw data on node B
  void ReceivePktProbeB (Ptr<Socket> socket);
  /// Packet counter on node B
  uint8_t m_countB;
  /// Receiving socket on node B
  Ptr<Ipv6RawSocketImpl> m_rxSocketB;

  /// Receive raw data on node C
  void ReceivePktProbeC (Ptr<Socket> socket);
  /// Packet counter on node C
  uint8_t m_countC;
  /// Receiving socket on node C
  Ptr<Ipv6RawSocketImpl> m_rxSocketC;

};

}
}

#endif /* TC_REGRESSION_TEST_H */
