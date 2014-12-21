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

#ifndef NDN_FACE_H
#define NDN_FACE_H

#include <ostream>
#include <algorithm>

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/type-id.h"
#include "ns3/traced-callback.h"

#include <ndn-cxx/name.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/management/nfd-face-traits.hpp>
#include <ndn-cxx/management/nfd-face-event-notification.hpp>
#include <ndn-cxx/management/nfd-face-status.hpp>
#include <ndn-cxx/util/face-uri.hpp>

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/model/ndn-ns3.hpp"

namespace ns3 {

class Packet;
class Node;

namespace ndn {

/**
 * \ingroup ndn
 * \defgroup ndn-face Faces
 */
/**
 * \ingroup ndn-face
 * \brief Virtual class defining NDN face
 *
 * This class defines basic functionality of NDN face. Face is core
 * component responsible for actual delivery of data packet to and
 * from NDN stack
 *
 * \see ndn::AppFace, ndn::NetDeviceFace
 */

class Face : public Object, public ::nfd::Face {
public:
  class Error : public std::runtime_error {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  /**
   * \brief Default constructor
   */
  Face(Ptr<Node> node);

  virtual ~Face();

  static TypeId
  GetTypeId();

  /**
   * \brief NDN protocol handlers
   *
   * \param face Face from which packet has been received
   * \param packet Original packet
   */
  typedef Callback<void, Ptr<Face>, shared_ptr<::ndn::Interest>> InterestHandler;
  typedef Callback<void, Ptr<Face>, shared_ptr<::ndn::Data>> DataHandler;

  /** \brief Close the face
   *
   *  This terminates all communication on the face and cause
   *  onFail() method event to be invoked
   */
  virtual void
  close();

  virtual void
  RegisterProtocolHandlers(){};

  virtual void
  UnRegisterProtocolHandlers(){};

  /**
   * @brief Get node to which this face is associated
   */
  Ptr<Node>
  GetNode() const;

  ////////////////////////////////////////////////////////////////////

  /**
   * @brief Send out interest through the face
   * @param interest Interest to send out
   * @param packet "payload" that is attached to the interest (can carry some packet tags)
   *
   * @returns true if interest is considered to be send out (enqueued)
   */
  virtual void
  sendInterest(const Interest& interest);

  /**
   * @brief Send out Dat packet through the face
   * @param data Data packet to send out
   * @param packet Data packet payload, can also carry packet tags
   *
   * @returns true if Data packet is considered to be send out (enqueued)
   */
  virtual void
  sendData(const Data& data);

  ////////////////////////////////////////////////////////////////////

  /**
   * \brief Assign routing/forwarding metric with face
   *
   * \param metric configured routing metric (cost) of this face
   */
  virtual void
  SetMetric(uint16_t metric);

  /**
   * \brief Get routing/forwarding metric assigned to the face
   *
   * \returns configured routing/forwarding metric (cost) of this face
   */
  virtual uint16_t
  GetMetric(void) const;

  /**
   * These are face states and may be distinct from actual lower-layer
   * device states, such as found in real implementations (where the
   * device may be down but ndn face state is still up).
   */

  /**
   * \brief Enable or disable this face
   */
  inline void
  SetUp(bool up = true);

  /**
   * \brief Returns true if this face is enabled, false otherwise.
   */
  inline bool
  IsUp() const;

  /**
   * @brief Get face flags
   *
   * Face flags may indicate various properties of the face.  For example, if the face is an
   *application face,
   * than the returned flags have Face::APPLICATION bit set.
   *
   * @see ndn::Face::Flags for the list of currently defined face flags
   */
  inline uint32_t
  GetFlags() const;

  /**
   * @brief List of currently defined face flags
   */
  enum Flags {
    APPLICATION = 1 ///< @brief An application face
  };

  /**
   * @brief Print information about the face into the stream
   * @param os stream to write information to
   */
  virtual std::ostream&
  Print(std::ostream& os) const;

  /**
   * \brief Set face Id
   *
   * Id is purely informative and should not be used for any other purpose
   *
   * \param id id to set
   */
  inline void
  SetId(uint32_t id);

  /**
   * \brief Get face Id
   *
   * Id is purely informative and should not be used for any other purpose
   *
   * \returns id id to set
   */
  inline uint32_t
  GetId() const;

  /**
   * \brief Compare two faces. Only two faces on the same node could be compared.
   *
   * Internal index is used for comparison.
   */
  bool
  operator==(const Face& face) const;

  /**
   * \brief Compare two faces. Only two faces on the same node could be compared.
   *
   * Internal index is used for comparison.
   */
  inline bool
  operator!=(const Face& face) const;

  /**
   * \brief Compare two faces. Only two faces on the same node could be compared.
   *
   * Internal index is used for comparison.
   */
  bool
  operator<(const Face& face) const;

protected:
  /**
   * @brief Send packet down to the stack (towards app or network)
   */
  virtual bool
  Send(Ptr<Packet> packet);

  /**
   * @brief Send packet up to the stack (towards forwarding strategy)
   */
  virtual bool
  Receive(Ptr<const Packet> p);

  /**
   * @brief Set face flags
   */
  void
  SetFlags(uint32_t flags);

private:
  // Face (const Face &); ///< \brief Disabled copy constructor
  Face&
  operator=(const Face&); ///< \brief Disabled copy operator

protected:
  Ptr<Node> m_node; ///< \brief Smart pointer to Node

private:
  bool m_ifup;
  uint32_t m_id;     ///< \brief id of the interface in NDN stack (per-node uniqueness)
  uint16_t m_metric; ///< \brief metric of the face
  uint32_t m_flags;  ///< @brief faces flags (e.g., APPLICATION)
  ::nfd::FaceId m_idNfd;
  ::ndn::util::FaceUri m_remoteUri;
  ::ndn::util::FaceUri m_localUri;
};

std::ostream&
operator<<(std::ostream& os, const Face& face);

inline bool
Face::IsUp(void) const
{
  return m_ifup;
}

inline void
Face::SetUp(bool up /* = true*/)
{
  m_ifup = up;
}

inline uint32_t
Face::GetFlags() const
{
  return m_flags;
}

inline bool
operator<(const Ptr<Face>& lhs, const Ptr<Face>& rhs)
{
  return *lhs < *rhs;
}

void
Face::SetId(uint32_t id)
{
  m_id = id;
}

uint32_t
Face::GetId() const
{
  return m_id;
}

inline bool
  Face::operator!= (const Face &face) const
{
  return !(*this == face);
}

} // namespace ndn
} // namespace ns3

#endif // NDN_FACE_H
