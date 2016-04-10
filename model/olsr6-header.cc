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

#include <cmath>

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/address-utils.h"
#include "olsr6-header.h"

#define IPV6_ADDRESS_SIZE 16
#define OLSR6_MSG_HEADER_SIZE 24
#define OLSR6_PKT_HEADER_SIZE 4

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Olsr6Header");

namespace olsr6 {


/// Scaling factor used in RFC 3626.
#define OLSR6_C 0.0625

///
/// \brief Converts a decimal number of seconds to the mantissa/exponent format.
///
/// \param seconds decimal number of seconds we want to convert.
/// \return the number of seconds in mantissa/exponent format.
///
uint8_t
SecondsToEmf (double seconds)
{
  int a, b = 0;

  // find the largest integer 'b' such that: T/C >= 2^b
  for (b = 0; (seconds / OLSR6_C) >= (1 << b); ++b)
    {
    }
  NS_ASSERT ((seconds / OLSR6_C) < (1 << b));
  b--;
  NS_ASSERT ((seconds / OLSR6_C) >= (1 << b));

  // compute the expression 16*(T/(C*(2^b))-1), which may not be a integer
  double tmp = 16 * (seconds / (OLSR6_C * (1 << b)) - 1);

  // round it up.  This results in the value for 'a'
  a = (int) std::ceil (tmp - 0.5);

  // if 'a' is equal to 16: increment 'b' by one, and set 'a' to 0
  if (a == 16)
    {
      b += 1;
      a = 0;
    }

  // now, 'a' and 'b' should be integers between 0 and 15,
  NS_ASSERT (a >= 0 && a < 16);
  NS_ASSERT (b >= 0 && b < 16);

  // the field will be a byte holding the value a*16+b
  return (uint8_t)((a << 4) | b);
}

///
/// \brief Converts a number of seconds in the mantissa/exponent format to a decimal number.
///
/// \param olsr6_format number of seconds in mantissa/exponent format.
/// \return the decimal number of seconds.
///
double
EmfToSeconds (uint8_t olsr6Format)
{
  int a = (olsr6Format >> 4);
  int b = (olsr6Format & 0xf);
  // value = C*(1+a/16)*2^b [in seconds]
  return OLSR6_C * (1 + a / 16.0) * (1 << b);
}



// ---------------- OLSR6 Packet -------------------------------

NS_OBJECT_ENSURE_REGISTERED (PacketHeader);

PacketHeader::PacketHeader ()
{
}

PacketHeader::~PacketHeader ()
{
}

TypeId
PacketHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::olsr6::PacketHeader")
    .SetParent<Header> ()
    .SetGroupName ("Olsr6")
    .AddConstructor<PacketHeader> ()
  ;
  return tid;
}
TypeId
PacketHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
PacketHeader::GetSerializedSize (void) const
{
  return OLSR6_PKT_HEADER_SIZE;
}

void
PacketHeader::Print (std::ostream &os) const
{
  /// \todo
}

void
PacketHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU16 (m_packetLength);
  i.WriteHtonU16 (m_packetSequenceNumber);
}

uint32_t
PacketHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_packetLength  = i.ReadNtohU16 ();
  m_packetSequenceNumber = i.ReadNtohU16 ();
  return GetSerializedSize ();
}


// ---------------- OLSR6 Message -------------------------------

NS_OBJECT_ENSURE_REGISTERED (MessageHeader);

MessageHeader::MessageHeader ()
  : m_messageType (MessageHeader::MessageType (0))
{
}

MessageHeader::~MessageHeader ()
{
}

TypeId
MessageHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::olsr6::MessageHeader")
    .SetParent<Header> ()
    .SetGroupName ("Olsr6")
    .AddConstructor<MessageHeader> ()
  ;
  return tid;
}
TypeId
MessageHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MessageHeader::GetSerializedSize (void) const
{
  uint32_t size = OLSR6_MSG_HEADER_SIZE;
  switch (m_messageType)
    {
    case MID_MESSAGE:
      size += m_message.mid.GetSerializedSize ();
      break;
    case HELLO_MESSAGE:
      NS_LOG_DEBUG ("Hello Message Size: " << size << " + " << m_message.hello.GetSerializedSize ());
      size += m_message.hello.GetSerializedSize ();
      break;
    case TC_MESSAGE:
      size += m_message.tc.GetSerializedSize ();
      break;
    case HNA_MESSAGE:
      size += m_message.hna.GetSerializedSize ();
      break;
    default:
      NS_ASSERT (false);
    }
  return size;
}

void
MessageHeader::Print (std::ostream &os) const
{
  /// \todo
}

void
MessageHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteU8 (m_vTime);
  i.WriteHtonU16 (GetSerializedSize ());
  WriteTo (i, m_originatorAddress);
  i.WriteU8 (m_timeToLive);
  i.WriteU8 (m_hopCount);
  i.WriteHtonU16 (m_messageSequenceNumber);

  switch (m_messageType)
    {
    case MID_MESSAGE:
      m_message.mid.Serialize (i);
      break;
    case HELLO_MESSAGE:
      m_message.hello.Serialize (i);
      break;
    case TC_MESSAGE:
      m_message.tc.Serialize (i);
      break;
    case HNA_MESSAGE:
      m_message.hna.Serialize (i);
      break;
    default:
      NS_ASSERT (false);
    }

}

uint32_t
MessageHeader::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType  = (MessageType) i.ReadU8 ();
  NS_ASSERT (m_messageType >= HELLO_MESSAGE && m_messageType <= HNA_MESSAGE);
  m_vTime  = i.ReadU8 ();
  m_messageSize  = i.ReadNtohU16 ();
  ReadFrom (i,m_originatorAddress);
  m_timeToLive  = i.ReadU8 ();
  m_hopCount  = i.ReadU8 ();
  m_messageSequenceNumber = i.ReadNtohU16 ();
  size = OLSR6_MSG_HEADER_SIZE;
  switch (m_messageType)
    {
    case MID_MESSAGE:
      size += m_message.mid.Deserialize (i, m_messageSize - OLSR6_MSG_HEADER_SIZE);
      break;
    case HELLO_MESSAGE:
      size += m_message.hello.Deserialize (i, m_messageSize - OLSR6_MSG_HEADER_SIZE);
      break;
    case TC_MESSAGE:
      size += m_message.tc.Deserialize (i, m_messageSize - OLSR6_MSG_HEADER_SIZE);
      break;
    case HNA_MESSAGE:
      size += m_message.hna.Deserialize (i, m_messageSize - OLSR6_MSG_HEADER_SIZE);
      break;
    default:
      NS_ASSERT (false);
    }
  return size;
}


// ---------------- OLSR6 MID Message -------------------------------

uint32_t
MessageHeader::Mid::GetSerializedSize (void) const
{
  return this->interfaceAddresses.size () * IPV6_ADDRESS_SIZE;
}

void
MessageHeader::Mid::Print (std::ostream &os) const
{
  /// \todo
}

void
MessageHeader::Mid::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  for (std::vector<Ipv6Address>::const_iterator iter = this->interfaceAddresses.begin ();
       iter != this->interfaceAddresses.end (); iter++)
    {
      WriteTo (i,*iter);
    }
}

uint32_t
MessageHeader::Mid::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  Buffer::Iterator i = start;

  this->interfaceAddresses.clear ();
  NS_ASSERT (messageSize % IPV6_ADDRESS_SIZE == 0);

  int numAddresses = messageSize / IPV6_ADDRESS_SIZE;
  this->interfaceAddresses.erase (this->interfaceAddresses.begin (),
                                  this->interfaceAddresses.end ());
  Ipv6Address next_add;
  for (int n = 0; n < numAddresses; ++n)
    {
      ReadFrom (i,next_add);
      this->interfaceAddresses.push_back (next_add);
    }
  return GetSerializedSize ();
}



// ---------------- OLSR6 HELLO Message -------------------------------

uint32_t
MessageHeader::Hello::GetSerializedSize (void) const
{
  uint32_t size = 4;
  for (std::vector<LinkMessage>::const_iterator iter = this->linkMessages.begin ();
       iter != this->linkMessages.end (); iter++)
    {
      const LinkMessage &lm = *iter;
      size += 4;
      size += IPV6_ADDRESS_SIZE * lm.neighborInterfaceAddresses.size ();
    }
  return size;
}

void
MessageHeader::Hello::Print (std::ostream &os) const
{
  /// \todo
}

void
MessageHeader::Hello::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU16 (0); // Reserved
  i.WriteU8 (this->hTime);
  i.WriteU8 (this->willingness);

  for (std::vector<LinkMessage>::const_iterator iter = this->linkMessages.begin ();
       iter != this->linkMessages.end (); iter++)
    {
      const LinkMessage &lm = *iter;

      i.WriteU8 (lm.linkCode);
      i.WriteU8 (0); // Reserved

      // The size of the link message, counted in bytes and measured
      // from the beginning of the "Link Code" field and until the
      // next "Link Code" field (or - if there are no more link types
      // - the end of the message).
      i.WriteHtonU16 (4 + lm.neighborInterfaceAddresses.size () * IPV6_ADDRESS_SIZE);

      for (std::vector<Ipv6Address>::const_iterator neigh_iter = lm.neighborInterfaceAddresses.begin ();
           neigh_iter != lm.neighborInterfaceAddresses.end (); neigh_iter++)
        {
          WriteTo (i,*neigh_iter);
        }
    }
}

uint32_t
MessageHeader::Hello::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  Buffer::Iterator i = start;

  NS_ASSERT (messageSize >= 4);

  this->linkMessages.clear ();

  uint16_t helloSizeLeft = messageSize;

  i.ReadNtohU16 (); // Reserved
  this->hTime = i.ReadU8 ();
  this->willingness = i.ReadU8 ();

  helloSizeLeft -= 4;

  while (helloSizeLeft)
    {
      LinkMessage lm;
      NS_ASSERT (helloSizeLeft >= 4);
      lm.linkCode = i.ReadU8 ();
      i.ReadU8 (); // Reserved
      uint16_t lmSize = i.ReadNtohU16 ();
      NS_ASSERT ((lmSize - 4) % IPV6_ADDRESS_SIZE == 0);
      Ipv6Address next_add;
      for (int n = (lmSize - 4) / IPV6_ADDRESS_SIZE; n; --n)
        {
          ReadFrom (i,next_add);
          lm.neighborInterfaceAddresses.push_back (next_add);
        }
      helloSizeLeft -= lmSize;
      this->linkMessages.push_back (lm);
    }

  return messageSize;
}



// ---------------- OLSR6 TC Message -------------------------------

uint32_t
MessageHeader::Tc::GetSerializedSize (void) const
{
  return 4 + this->neighborAddresses.size () * IPV6_ADDRESS_SIZE;
}

void
MessageHeader::Tc::Print (std::ostream &os) const
{
  /// \todo
}

void
MessageHeader::Tc::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (this->ansn);
  i.WriteHtonU16 (0); // Reserved

  for (std::vector<Ipv6Address>::const_iterator iter = this->neighborAddresses.begin ();
       iter != this->neighborAddresses.end (); iter++)
    {
      WriteTo (i,*iter);
    }
}

uint32_t
MessageHeader::Tc::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  Buffer::Iterator i = start;

  this->neighborAddresses.clear ();
  NS_ASSERT (messageSize >= 4);

  this->ansn = i.ReadNtohU16 ();
  i.ReadNtohU16 (); // Reserved

  NS_ASSERT ((messageSize - 4) % IPV6_ADDRESS_SIZE == 0);
  int numAddresses = (messageSize - 4) / IPV6_ADDRESS_SIZE;
  this->neighborAddresses.clear ();
  Ipv6Address next_add;
  for (int n = 0; n < numAddresses; ++n)
    {
      ReadFrom (i,next_add);
      this->neighborAddresses.push_back (next_add);
    }

  return messageSize;
}


// ---------------- OLSR6 HNA Message -------------------------------

uint32_t
MessageHeader::Hna::GetSerializedSize (void) const
{
  return 2 * this->associations.size () * IPV6_ADDRESS_SIZE;
}

void
MessageHeader::Hna::Print (std::ostream &os) const
{
  /// \todo
}

void
MessageHeader::Hna::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  uint8_t buf[16];
  for (size_t n = 0; n < this->associations.size (); ++n)
    {
      WriteTo (i,this->associations[n].address);
      this->associations[n].mask.GetBytes (buf);
      i.Write (buf, 16);
    }
}

uint32_t
MessageHeader::Hna::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  Buffer::Iterator i = start;

  NS_ASSERT (messageSize % (IPV6_ADDRESS_SIZE * 2) == 0);
  int numAddresses = messageSize / IPV6_ADDRESS_SIZE / 2;
  this->associations.clear ();
  uint8_t buf[16];
  for (int n = 0; n < numAddresses; ++n)
    {
      Ipv6Address address;
      (ReadFrom (i,address));
      i.Read (buf,16);
      Ipv6Prefix mask (buf);
      this->associations.push_back ((Association) { address, mask});
    }
  return messageSize;
}

}
}  // namespace olsr6, ns3

