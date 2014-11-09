/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California,
 *                      Arizona Board of Regents,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University,
 *                      Washington University in St. Louis,
 *                      Beijing Institute of Technology,
 *                      The University of Memphis
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ns3/ndnSIM/NFD/daemon/fw/best-route-strategy.hpp"

namespace nfd {
namespace fw {

const Name BestRouteStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/best-route/%FD%01");

BestRouteStrategy::BestRouteStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder, name)
{
}

BestRouteStrategy::~BestRouteStrategy()
{
}

static inline bool
predicate_PitEntry_canForwardTo_NextHop(shared_ptr<pit::Entry> pitEntry,
                                        const fib::NextHop& nexthop)
{
  return pitEntry->canForwardTo(*nexthop.getFace());
}

void
BestRouteStrategy::afterReceiveInterest(const ns3::ndn::Face& inFace,
                   const Interest& interest,
                   shared_ptr<fib::Entry> fibEntry,
                   shared_ptr<pit::Entry> pitEntry)
{
  if (pitEntry->hasUnexpiredOutRecords()) {
    // not a new Interest, don't forward
    return;
  }

  const fib::NextHopList& nexthops = fibEntry->getNextHops();
  fib::NextHopList::const_iterator it = std::find_if(nexthops.begin(), nexthops.end(),
    bind(&predicate_PitEntry_canForwardTo_NextHop, pitEntry, _1));

  if (it == nexthops.end()) {
    this->rejectPendingInterest(pitEntry);
    return;
  }

  shared_ptr<ns3::ndn::Face> outFace = it->getFace();
  this->sendInterest(pitEntry, outFace);
}

} // namespace fw
} // namespace nfd
