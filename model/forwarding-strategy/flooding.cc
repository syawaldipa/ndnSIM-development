/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#include "flooding.h"

#include "ns3/ndn-interest-header.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("NdnSimFlooding");

namespace ns3 {
namespace ndnSIM {

using namespace __ndn_private;

NS_OBJECT_ENSURE_REGISTERED (Flooding);
    
TypeId Flooding::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndnSIM::Flooding")
    .SetGroupName ("Ndn")
    .SetParent <Nacks> ()
    .AddConstructor <Flooding> ()
    ;
  return tid;
}
    
Flooding::Flooding ()
{
}

bool
Flooding::DoPropagateInterest (const Ptr<NdnFace> &incomingFace,
                               Ptr<NdnInterestHeader> header,
                               const Ptr<const Packet> &packet,
                               Ptr<NdnPitEntry> pitEntry)
{
  NS_LOG_FUNCTION (this);

  int propagatedCount = 0;

  BOOST_FOREACH (const NdnFibFaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<i_metric> ())
    {
      NS_LOG_DEBUG ("Trying " << boost::cref(metricFace));
      if (metricFace.m_status == NdnFibFaceMetric::NDN_FIB_RED) // all non-read faces are in the front of the list
        break;
      
      if (metricFace.m_face == incomingFace) 
        {
          NS_LOG_DEBUG ("continue (same as incoming)");
          continue; // same face as incoming, don't forward
        }

      if (!WillSendOutInterest (metricFace.m_face, header, pitEntry))
        {
          continue;
        }

      //transmission
      Ptr<Packet> packetToSend = packet->Copy ();
      metricFace.m_face->Send (packetToSend);

      DidSendOutInterest (metricFace.m_face, header, packet, pitEntry);
      
      propagatedCount++;
    }

  NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
  return propagatedCount > 0;
}

} // namespace ndnSIM
} // namespace ns3