/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2014  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-app-face.hpp"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"

#include "apps/ndn-app.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.AppFace");

namespace ns3 {
namespace ndn {

ATTRIBUTE_HELPER_CPP(Name);


AppFace::AppFace(Ptr<App> app)
  : LocalFace(FaceUri("appFace://"), FaceUri("appFace://"))
  , m_node(app->GetNode())
  , m_app(app)
{
  NS_LOG_FUNCTION(this << app);

  NS_ASSERT(m_app != 0);
}

AppFace::~AppFace()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
AppFace::close()
{
}

void
AppFace::sendInterest(const Interest& interest)
{
  NS_LOG_FUNCTION(this << &interest);

  // to decouple callbacks
  Simulator::ScheduleNow(&App::OnInterest, m_app, interest.shared_from_this());
}

void
AppFace::sendData(const Data& data)
{
  NS_LOG_FUNCTION(this << &data);

  // to decouple callbacks
  Simulator::ScheduleNow(&App::OnData, m_app, data.shared_from_this());
}

// std::ostream&
// AppFace::Print(std::ostream& os) const
// {
//   os << "dev=local(" << GetId() << ")";
//   return os;
// }

} // namespace ndn
} // namespace ns3