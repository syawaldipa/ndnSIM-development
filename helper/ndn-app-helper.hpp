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

#ifndef NDN_APP_HELPER_H
#define NDN_APP_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/ptr.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-helpers
 * \brief A helper to make it easier to instantiate an ns3::NdnConsumer Application
 * on a set of nodes.
 */
class AppHelper {
public:
  /**
   * \brief Create an NdnAppHelper to make it easier to work with Ndn apps
   *
   * \param app Class of the application
   */
  AppHelper(const std::string& prefix);

  /**
   * @brief Set the prefix consumer will be requesting
   */
  void
  SetPrefix(const std::string& prefix);

  /**
   * \brief Helper function used to set the underlying application attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void
  SetAttribute(std::string name, const AttributeValue& value);

  /**
   * Install an ns3::NdnConsumer on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an NdnConsumer
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer
  Install(NodeContainer c);

  /**
   * Install an ns3::NdnConsumer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an NdnConsumer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer
  Install(Ptr<Node> node);

  /**
   * Install an ns3Consumer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param nodeName The node on which an NdnConsumer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer
  Install(std::string nodeName);

private:
  /**
   * \internal
   * Install an ns3::NdnConsumer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an NdnConsumer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application>
  InstallPriv(Ptr<Node> node);
  ObjectFactory m_factory;
};

} // namespace ndn
} // namespace ns3

#endif // NDN_APP_HELPER_H