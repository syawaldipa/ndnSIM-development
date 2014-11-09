/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  Regents of the University of California,
 *                     Arizona Board of Regents,
 *                     Colorado State University,
 *                     University Pierre & Marie Curie, Sorbonne University,
 *                     Washington University in St. Louis,
 *                     Beijing Institute of Technology
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
 **/

#include "ns3/ndnSIM/NFD/daemon/table/fib-nexthop.hpp"

namespace nfd {
namespace fib {

NextHop::NextHop(shared_ptr<ns3::ndn::Face> face)
  : m_face(face), m_cost(0)
{
}

NextHop::NextHop(const NextHop& other)
  : m_face(other.m_face), m_cost(other.m_cost)
{
}

shared_ptr<ns3::ndn::Face>
NextHop::getFace() const
{
  return m_face;
}

void
NextHop::setCost(uint64_t cost)
{
  m_cost = cost;
}

uint64_t
NextHop::getCost() const
{
  return m_cost;
}

} // namespace fib
} // namespace nfd
