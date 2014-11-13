/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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
 *
 */

#include "ndn-face.h"

#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/assert.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"

#include "ns3/ndn-header-helper.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include <boost/ref.hpp>

#include "ns3/logger.hpp"

NS_LOG_COMPONENT_DEFINE ("ndn.Face");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (Face);

TypeId
Face::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::Face")
    .SetParent<Object> ()
    .SetGroupName ("Ndn")
    .AddAttribute ("Id", "Face id (unique integer for the Ndn stack on this node)",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0),
                   MakeUintegerAccessor (&Face::m_id),
                   MakeUintegerChecker<uint32_t> ())
    ;
  return tid;
}

Face::Face(const nfd::FaceUri& remoteUri, const nfd::FaceUri& localUri, bool isLocal)
  : m_idNfd(INVALID_FACEID)
  , m_remoteUri(remoteUri)
  , m_localUri(localUri)
  , m_isLocal(isLocal)
  , m_isOnDemand(false)
  , m_isFailed(false)
{
  onReceiveInterest += [this](const ::ndn::Interest&) { ++m_counters.getNInInterests(); };
  onReceiveData     += [this](const ::ndn::Data&) {     ++m_counters.getNInDatas(); };
  onSendInterest    += [this](const ::ndn::Interest&) { ++m_counters.getNOutInterests(); };
  onSendData        += [this](const ::ndn::Data&) {     ++m_counters.getNOutDatas(); };
}

FaceId
Face::getId() const
{
  return m_idNfd;
}

// this method is private and should be used only by the FaceTable
void
Face::setId(FaceId faceId)
{
  m_idNfd = faceId;
}

void
Face::setDescription(const std::string& description)
{
  m_description = description;
}

const std::string&
Face::getDescription() const
{
  return m_description;
}

bool
Face::isMultiAccess() const
{
  return false;
}

bool
Face::isUp() const
{
  return true;
}

bool
Face::decodeAndDispatchInput(const ::ndn::Block& element)
{
  try {
    /// \todo Ensure lazy field decoding process

    if (element.type() == ::ndn::tlv::Interest)
      {
        ::ndn::shared_ptr< ::ndn::Interest> i = ::ndn::make_shared< ::ndn::Interest>();
        i->wireDecode(element);
        this->onReceiveInterest(*i);
      }
    else if (element.type() == ::ndn::tlv::Data)
      {
        ::ndn::shared_ptr< ::ndn::Data> d = ::ndn::make_shared< ::ndn::Data>();
        d->wireDecode(element);
        this->onReceiveData(*d);
      }
    else
      return false;

    return true;
  }
  catch (::ndn::tlv::Error&) {
    return false;
  }
}

void
Face::fail(const std::string& reason)
{
  if (m_isFailed) {
    return;
  }

  m_isFailed = true;
  this->onFail(reason);

  this->onFail.clear();
}

void
Face::close ()
{
}

template<typename FaceTraits>
void
Face::copyStatusTo(FaceTraits& traits) const
{
  traits.setFaceId(getId())
    .setRemoteUri(getRemoteUri().toString())
    .setLocalUri(getLocalUri().toString());

  if (isLocal()) {
    traits.setFaceScope(::ndn::nfd::FACE_SCOPE_LOCAL);
  }
  else {
    traits.setFaceScope(::ndn::nfd::FACE_SCOPE_NON_LOCAL);
  }

  if (isOnDemand()) {
    traits.setFacePersistency(::ndn::nfd::FACE_PERSISTENCY_ON_DEMAND);
  }
  else {
    traits.setFacePersistency(::ndn::nfd::FACE_PERSISTENCY_PERSISTENT);
  }
}

template void
Face::copyStatusTo< ::ndn::nfd::FaceStatus>(::ndn::nfd::FaceStatus&) const;

template void
Face::copyStatusTo< ::ndn::nfd::FaceEventNotification>(::ndn::nfd::FaceEventNotification&) const;

::ndn::nfd::FaceStatus
Face::getFaceStatus() const
{
  ::ndn::nfd::FaceStatus status;
  copyStatusTo(status);

  this->getCounters().copyTo(status);

  return status;
}

/**
 * By default, Ndn face are created in the "down" state
 *  with no IP addresses.  Before becoming useable, the user must
 * invoke SetUp on them once an Ndn address and mask have been set.
 */
Face::Face (Ptr<Node> node)
  : m_node (node)
  , m_upstreamInterestHandler (MakeNullCallback< void, Ptr<Face>, ::ndn::shared_ptr< ::ndn::Interest> > ())
  , m_upstreamDataHandler (MakeNullCallback< void, Ptr<Face>, ::ndn::shared_ptr< ::ndn::Data> > ())
  , m_ifup (false)
  , m_id ((uint32_t)-1)
  , m_metric (0)
  , m_flags (0)
{
  NS_LOG_FUNCTION (this << node);

  NS_ASSERT_MSG (node != 0, "node cannot be NULL. Check the code");
}
Face::~Face ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

Face& Face::operator= (const Face &)
{
  return *this;
}

Ptr<Node>
Face::GetNode () const
{
  return m_node;
}

void
Face::RegisterProtocolHandlers (const InterestHandler &interestHandler, const DataHandler &dataHandler)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_upstreamInterestHandler = interestHandler;
  m_upstreamDataHandler = dataHandler;
}

void
Face::UnRegisterProtocolHandlers ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_upstreamInterestHandler = MakeNullCallback< void, Ptr<Face>, ::ndn::shared_ptr< ::ndn::Interest> > ();
  m_upstreamDataHandler = MakeNullCallback< void, Ptr<Face>, ::ndn::shared_ptr< ::ndn::Data> > ();
}


bool
Face::SendInterest ( ::ndn::shared_ptr<const ::ndn::Interest> interest)
{
  NS_LOG_FUNCTION (this << boost::cref (*this) << interest->getName ());

  if (!IsUp ())
    {
      return false;
    }
  // I assume that this should work...
  ::ndn::Convert::Convert convert;
  Ptr<Packet> packet = Create <Packet> ();
  ::ndn::Block block = interest->wireEncode ();
  convert.InterestToPacket (::ndn::make_shared <::ndn::Block> (block), packet);
  return Send (packet);

  //return Send (Wire::FromInterest (interest));
}

bool
Face::SendData (::ndn::shared_ptr<const ::ndn::Data> data)
{
  NS_LOG_FUNCTION (this << data);

  if (!IsUp ())
    {
      return false;
    }
  // I assume that this should work..
  ::ndn::Convert::Convert convert;
  Ptr<Packet> packet = Create <Packet> ();
  ::ndn::Block block = data->wireEncode ();
  convert.InterestToPacket (::ndn::make_shared <::ndn::Block> (block), packet);
  return Send (packet);

  //return Send (Wire::FromData (data));
}

bool
Face::Send (Ptr<Packet> packet)
{
  FwHopCountTag hopCount;
  bool tagExists = packet->RemovePacketTag (hopCount);
  if (tagExists)
    {
      hopCount.Increment ();
      packet->AddPacketTag (hopCount);
    }

  return true;
}

bool
Face::Receive (Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (this << p << p->GetSize ());

  if (!IsUp ())
    {
      // no tracing here. If we were off while receiving, we shouldn't even know that something was there
      return false;
    }

  Ptr<Packet> packet = p->Copy (); // give upper layers a rw copy of the packet
  try
    {
      //Let's see..
      ::ndn::Convert::Convert convert;
      ::ndn::Block block = convert.FromPacket (packet);
      uint32_t type = block.type();
      if (type == 0x05) {
        ::ndn::Interest interest;
        interest.wireDecode (block);
        ReceiveInterest (::ndn::make_shared <::ndn::Interest> (interest));
      }
     else
       if (type == 0x06) {
         ::ndn::Data data;
         data.wireDecode (block);
         ReceiveData (::ndn::make_shared <::ndn::Data> (data));
       }
    }
  catch (UnknownHeaderException)
    {
      NS_FATAL_ERROR ("Unknown NDN header. Should not happen");
      return false;
    }

  return false;
}

bool
Face::ReceiveInterest (::ndn::shared_ptr< ::ndn::Interest> interest)
{
  if (!IsUp ())
    {
      // no tracing here. If we were off while receiving, we shouldn't even know that something was there
      return false;
    }

  m_upstreamInterestHandler (this, interest);
  return true;
}

bool
Face::ReceiveData (::ndn::shared_ptr< ::ndn::Data> data)
{
  if (!IsUp ())
    {
      // no tracing here. If we were off while receiving, we shouldn't even know that something was there
      return false;
    }

  m_upstreamDataHandler (this, data);
  return true;
}

void
Face::SetMetric (uint16_t metric)
{
  NS_LOG_FUNCTION (metric);
  m_metric = metric;
}

uint16_t
Face::GetMetric (void) const
{
  return m_metric;
}

void
Face::SetFlags (uint32_t flags)
{
  m_flags = flags;
}

bool
Face::operator== (const Face &face) const
{
  NS_ASSERT_MSG (m_node->GetId () == face.m_node->GetId (),
                 "Faces of different nodes should not be compared to each other: " << *this << " == " << face);

  return (m_id == face.m_id);
}

bool
Face::operator< (const Face &face) const
{
  NS_ASSERT_MSG (m_node->GetId () == face.m_node->GetId (),
                 "Faces of different nodes should not be compared to each other: " << *this << " == " << face);

  return (m_id < face.m_id);
}

std::ostream&
Face::Print (std::ostream &os) const
{
  os << "id=" << GetId ();
  return os;
}

std::ostream&
operator<< (std::ostream& os, const Face &face)
{
  face.Print (os);
  return os;
}

} // namespace ndn
} // namespace ns3
