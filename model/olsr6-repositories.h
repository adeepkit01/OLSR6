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


///
/// \brief Here are defined all data structures needed by an OLSR6 node.
///

#ifndef OLSR6_REPOSITORIES_H
#define OLSR6_REPOSITORIES_H

#include <set>
#include <vector>

#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"

namespace ns3 {
namespace olsr6 {


/// \ingroup olsr6
/// An Interface Association Tuple.
struct IfaceAssocTuple
{
  /// Interface address of a node.
  Ipv6Address ifaceAddr;
  /// Main address of the node.
  Ipv6Address mainAddr;
  /// Time at which this tuple expires and must be removed.
  Time time;
};

static inline bool
operator == (const IfaceAssocTuple &a, const IfaceAssocTuple &b)
{
  return (a.ifaceAddr == b.ifaceAddr
          && a.mainAddr == b.mainAddr);
}

static inline std::ostream&
operator << (std::ostream &os, const IfaceAssocTuple &tuple)
{
  os << "IfaceAssocTuple(ifaceAddr=" << tuple.ifaceAddr
  << ", mainAddr=" << tuple.mainAddr
  << ", time=" << tuple.time << ")";
  return os;
}

/// \ingroup olsr6
/// A Link Tuple.
struct LinkTuple
{
  /// Interface address of the local node.
  Ipv6Address localIfaceAddr;
  /// Interface address of the neighbor node.
  Ipv6Address neighborIfaceAddr;
  /// The link is considered bidirectional until this time.
  Time symTime;
  /// The link is considered unidirectional until this time.
  Time asymTime;
  /// Time at which this tuple expires and must be removed.
  Time time;
};

static inline bool
operator == (const LinkTuple &a, const LinkTuple &b)
{
  return (a.localIfaceAddr == b.localIfaceAddr
          && a.neighborIfaceAddr == b.neighborIfaceAddr);
}

static inline std::ostream&
operator << (std::ostream &os, const LinkTuple &tuple)
{
  os << "LinkTuple(localIfaceAddr=" << tuple.localIfaceAddr
  << ", neighborIfaceAddr=" << tuple.neighborIfaceAddr
  << ", symTime=" << tuple.symTime
  << ", asymTime=" << tuple.asymTime
  << ", expTime=" << tuple.time
  << ")";
  return os;
}

/// \ingroup olsr6
/// A Neighbor Tuple.
struct NeighborTuple
{
  /// Main address of a neighbor node.
  Ipv6Address neighborMainAddr;
  /// Status of the link (Symmetric or not Symmetric).
  enum Status
  {
    STATUS_NOT_SYM = 0, // "not symmetric"
    STATUS_SYM = 1, // "symmetric"
  } status; //!< Status of the link.
  /// A value between 0 and 7 specifying the node's willingness to carry traffic on behalf of other nodes.
  uint8_t willingness;
};

static inline bool
operator == (const NeighborTuple &a, const NeighborTuple &b)
{
  return (a.neighborMainAddr == b.neighborMainAddr
          && a.status == b.status
          && a.willingness == b.willingness);
}

static inline std::ostream&
operator << (std::ostream &os, const NeighborTuple &tuple)
{
  os << "NeighborTuple(neighborMainAddr=" << tuple.neighborMainAddr
  << ", status=" << (tuple.status == NeighborTuple::STATUS_SYM ? "SYM" : "NOT_SYM")
  << ", willingness=" << (int) tuple.willingness << ")";
  return os;
}

/// \ingroup olsr6
/// A 2-hop Tuple.
struct TwoHopNeighborTuple
{
  /// Main address of a neighbor.
  Ipv6Address neighborMainAddr;
  /// Main address of a 2-hop neighbor with a symmetric link to nb_main_addr.
  Ipv6Address twoHopNeighborAddr;
  /// Time at which this tuple expires and must be removed.
  Time expirationTime; // previously called 'time_'
};

static inline std::ostream&
operator << (std::ostream &os, const TwoHopNeighborTuple &tuple)
{
  os << "TwoHopNeighborTuple(neighborMainAddr=" << tuple.neighborMainAddr
  << ", twoHopNeighborAddr=" << tuple.twoHopNeighborAddr
  << ", expirationTime=" << tuple.expirationTime
  << ")";
  return os;
}

static inline bool
operator == (const TwoHopNeighborTuple &a, const TwoHopNeighborTuple &b)
{
  return (a.neighborMainAddr == b.neighborMainAddr
          && a.twoHopNeighborAddr == b.twoHopNeighborAddr);
}

/// \ingroup olsr6
/// An MPR-Selector Tuple.
struct MprSelectorTuple
{
  /// Main address of a node which have selected this node as a MPR.
  Ipv6Address mainAddr;
  /// Time at which this tuple expires and must be removed.
  Time expirationTime; // previously called 'time_'
};

static inline bool
operator == (const MprSelectorTuple &a, const MprSelectorTuple &b)
{
  return (a.mainAddr == b.mainAddr);
}


// The type "list of interface addresses"
//typedef std::vector<nsaddr_t> addr_list_t;

/// \ingroup olsr6
/// A Duplicate Tuple
struct DuplicateTuple
{
  /// Originator address of the message.
  Ipv6Address address;
  /// Message sequence number.
  uint16_t sequenceNumber;
  /// Indicates whether the message has been retransmitted or not.
  bool retransmitted;
  /// List of interfaces which the message has been received on.
  std::vector<Ipv6Address> ifaceList;
  /// Time at which this tuple expires and must be removed.
  Time expirationTime;
};

static inline bool
operator == (const DuplicateTuple &a, const DuplicateTuple &b)
{
  return (a.address == b.address
          && a.sequenceNumber == b.sequenceNumber);
}

/// \ingroup olsr6
/// A Topology Tuple
struct TopologyTuple
{
  /// Main address of the destination.
  Ipv6Address destAddr;
  /// Main address of a node which is a neighbor of the destination.
  Ipv6Address lastAddr;
  /// Sequence number.
  uint16_t sequenceNumber;
  /// Time at which this tuple expires and must be removed.
  Time expirationTime;
};

static inline bool
operator == (const TopologyTuple &a, const TopologyTuple &b)
{
  return (a.destAddr == b.destAddr
          && a.lastAddr == b.lastAddr
          && a.sequenceNumber == b.sequenceNumber);
}

static inline std::ostream&
operator << (std::ostream &os, const TopologyTuple &tuple)
{
  os << "TopologyTuple(destAddr=" << tuple.destAddr
  << ", lastAddr=" << tuple.lastAddr
  << ", sequenceNumber=" << (int) tuple.sequenceNumber
  << ", expirationTime=" << tuple.expirationTime
  << ")";
  return os;
}

/// \ingroup olsr6
/// Association
struct Association
{
  Ipv6Address networkAddr; //!< IPv6 Network address.
  Ipv6Prefix netmask;        //!< IPv6 Network prefix.
};

static inline bool
operator == (const Association &a, const Association &b)
{
  return (a.networkAddr == b.networkAddr
          && a.netmask == b.netmask);
}

static inline std::ostream&
operator << (std::ostream &os, const Association &tuple)
{
  os << "Association(networkAddr=" << tuple.networkAddr
  << ", netmask=" << tuple.netmask
  << ")";
  return os;
}

/// \ingroup olsr6
/// An Association Tuple
struct AssociationTuple
{
  /// Main address of the gateway.
  Ipv6Address gatewayAddr;
  /// Network Address of network reachable through gatewayAddr
  Ipv6Address networkAddr;
  /// Netmask of network reachable through gatewayAddr
  Ipv6Prefix netmask;
  /// Time at which this tuple expires and must be removed
  Time expirationTime;
};

static inline bool
operator == (const AssociationTuple &a, const AssociationTuple &b)
{
  return (a.gatewayAddr == b.gatewayAddr
          && a.networkAddr == b.networkAddr
          && a.netmask == b.netmask);
}

static inline std::ostream&
operator << (std::ostream &os, const AssociationTuple &tuple)
{
  os << "AssociationTuple(gatewayAddr=" << tuple.gatewayAddr
  << ", networkAddr=" << tuple.networkAddr
  << ", netmask=" << tuple.netmask
  << ", expirationTime=" << tuple.expirationTime
  << ")";
  return os;
}


typedef std::set<Ipv6Address>                   MprSet; //!< MPR Set type.
typedef std::vector<MprSelectorTuple>           MprSelectorSet; //!< MPR Selector Set type.
typedef std::vector<LinkTuple>                  LinkSet; //!< Link Set type.
typedef std::vector<NeighborTuple>              NeighborSet; //!< Neighbor Set type.
typedef std::vector<TwoHopNeighborTuple>        TwoHopNeighborSet; //!< 2-hop Neighbor Set type.
typedef std::vector<TopologyTuple>              TopologySet; //!< Topology Set type.
typedef std::vector<DuplicateTuple>             DuplicateSet; //!< Duplicate Set type.
typedef std::vector<IfaceAssocTuple>            IfaceAssocSet; //!< Interface Association Set type.
typedef std::vector<AssociationTuple>           AssociationSet; //!< Association Set type.
typedef std::vector<Association>                Associations; //!< Association Set type.


}
}  // namespace ns3, olsr6

#endif /* OLSR6_REPOSITORIES_H */
