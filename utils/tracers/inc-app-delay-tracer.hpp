 /*
  * ******************************************************************************
  * Copyright (c) 2021 Robert Bosch GmbH.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 3 as
  * published by the Free Software Foundation;
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * All rights reserved. This program and the accompanying materials
  * are made available under the terms of the GNU GENERAL PUBLIC LICENSE v3
  * which accompanies this distribution, and is available at
  * https://www.gnu.org/licenses/gpl-3.0.de.html
  *
  * Based on the app-delay-tracer.cpp implementation of ndnSIM
  * https://ndnsim.net/current/doxygen/classns3_1_1ndn_1_1AppDelayTracer.html
  *
  *  Date: November 8th, 2021
  *  Contributors:
  *      Robert Bosch GmbH - initial API and functionality
  *          Dennis Grewe <dennis.grewe@de.bosch.com>
  *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
  * *****************************************************************************
  */

#ifndef INC_APP_DELAY_TRACER_H
#define INC_APP_DELAY_TRACER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include <ns3/nstime.h>
#include <ns3/event-id.h>
#include <ns3/node-container.h>

#include <tuple>
#include <list>

namespace ns3 {

class Node;
class Packet;

namespace ndn{
  class App;

namespace inc{

/**
 * @ingroup ndn-tracers
 * @brief Tracer to obtain application-level delays
 */
class IncAppDelayTracer : public SimpleRefCount<IncAppDelayTracer> {
public:
  /**
   * @brief Helper method to install tracers on all simulation nodes
   *
   * @param file File to which traces will be written.  If filename is -, then std::out is used
   *
   */
  static void
  InstallAll(const std::string& file);

  /**
   * @brief Helper method to install tracers on the selected simulation nodes
   *
   * @param nodes Nodes on which to install tracer
   * @param file File to which traces will be written.  If filename is -, then std::out is used
   *
   */
  static void
  Install(const NodeContainer& nodes, const std::string& file);

  /**
   * @brief Helper method to install tracers on a specific simulation node
   *
   * @param nodes Nodes on which to install tracer
   * @param file File to which traces will be written.  If filename is -, then std::out is used
   * @param averagingPeriod How often data will be written into the trace file (default, every half
   *        second)
   */
  static void
  Install(Ptr<Node> node, const std::string& file);

  /**
   * @brief Helper method to install tracers on a specific simulation node
   *
   * @param nodes Nodes on which to install tracer
   * @param outputStream Smart pointer to a stream
   * @param averagingPeriod How often data will be written into the trace file (default, every half
   *        second)
   *
   * @returns a tuple of reference to output stream and list of tracers.
   *          !!! Attention !!! This tuple needs to be preserved for the lifetime of simulation,
   *          otherwise SEGFAULTs are inevitable
   */
  static Ptr<IncAppDelayTracer>
  Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream);

  /**
   * @brief Explicit request to remove all statically created tracers
   *
   * This method can be helpful if simulation scenario contains several independent run,
   * or if it is desired to do a postprocessing of the resulting data
   */
  static void
  Destroy();

  /**
   * @brief Trace constructor that attaches to all applications on the node using node's pointer
   * @param os    reference to the output stream
   * @param node  pointer to the node
   */
  IncAppDelayTracer(shared_ptr<std::ostream> os, Ptr<Node> node);

  /**
   * @brief Trace constructor that attaches to all applications on the node using node's name
   * @param os        reference to the output stream
   * @param nodeName  name of the node registered using Names::Add
   */
  IncAppDelayTracer(shared_ptr<std::ostream> os, const std::string& node);

  /**
   * @brief Destructor
   */
  ~IncAppDelayTracer();

  /**
   * @brief Print head of the trace (e.g., for post-processing)
   *
   * @param os reference to output stream
   */
  void
  PrintHeader(std::ostream& os) const;

private:
  void
  Connect();

  void
  LastRetransmittedInterestDataDelay(Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount);

  void
  FirstInterestDataDelay(Ptr<App> app, uint32_t seqno, Time delay, uint32_t rextCount,
                         int32_t hopCount);

private:
  std::string m_node;
  Ptr<Node> m_nodePtr;

  shared_ptr<std::ostream> m_os;
};

} // namespace inc
} // namespace ndn
} // namespace ns3

#endif // CCNX_CS_TRACER_H
