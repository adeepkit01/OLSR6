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

#include "olsr6-helper.h"
#include "ns3/olsr6-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-list-routing.h"

namespace ns3 {

Olsr6Helper::Olsr6Helper ()
{
  m_agentFactory.SetTypeId ("ns3::olsr6::RoutingProtocol");
}

Olsr6Helper::Olsr6Helper (const Olsr6Helper &o)
  : m_agentFactory (o.m_agentFactory)
{
  m_interfaceExclusions = o.m_interfaceExclusions;
}

Olsr6Helper*
Olsr6Helper::Copy (void) const
{
  return new Olsr6Helper (*this);
}

void
Olsr6Helper::ExcludeInterface (Ptr<Node> node, uint32_t interface)
{
  std::map< Ptr<Node>, std::set<uint32_t> >::iterator it = m_interfaceExclusions.find (node);

  if (it == m_interfaceExclusions.end ())
    {
      std::set<uint32_t> interfaces;
      interfaces.insert (interface);

      m_interfaceExclusions.insert (std::make_pair (node, std::set<uint32_t> (interfaces) ));
    }
  else
    {
      it->second.insert (interface);
    }
}

Ptr<Ipv6RoutingProtocol>
Olsr6Helper::Create (Ptr<Node> node) const
{
  Ptr<olsr6::RoutingProtocol> agent = m_agentFactory.Create<olsr6::RoutingProtocol> ();

  std::map<Ptr<Node>, std::set<uint32_t> >::const_iterator it = m_interfaceExclusions.find (node);

  if (it != m_interfaceExclusions.end ())
    {
      agent->SetInterfaceExclusions (it->second);
    }

  node->AggregateObject (agent);
  return agent;
}

void
Olsr6Helper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

int64_t
Olsr6Helper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
      NS_ASSERT_MSG (ipv6, "Ipv6 not installed on node");
      Ptr<Ipv6RoutingProtocol> proto = ipv6->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<olsr6::RoutingProtocol> olsr6 = DynamicCast<olsr6::RoutingProtocol> (proto);
      if (olsr6)
        {
          currentStream += olsr6->AssignStreams (currentStream);
          continue;
        }
      // Olsr6 may also be in a list
      Ptr<Ipv6ListRouting> list = DynamicCast<Ipv6ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv6RoutingProtocol> listProto;
          Ptr<olsr6::RoutingProtocol> listOlsr6;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listOlsr6 = DynamicCast<olsr6::RoutingProtocol> (listProto);
              if (listOlsr6)
                {
                  currentStream += listOlsr6->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);

}

} // namespace ns3
