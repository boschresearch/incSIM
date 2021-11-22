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
  * Based on the cs-tracer.cpp implementation of ndnSIM
  * https://github.com/named-data-ndnSIM/ndnSIM/blob/master/utils/tracers/ndn-cs-tracer.cpp
  *
  * This tracer defines an application monitoring component for the ns3 network
  * simulator. The tracer is intended for data/result generting nodes in the
  * incl. data producers, compute nodes, etc.. The tracer monitors incoming
  * Interests and outgoing Data packets on application level and can be plugged
  * into typical ndnSIM applications.
  *
  *  Date: November 8th, 2021
  *  Contributors:
  *      Robert Bosch GmbH - initial API and functionality
  *          Dennis Grewe <dennis.grewe@de.bosch.com>
  *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
  * *****************************************************************************
  */

#ifndef SRC_INC_COMPUTE_NODE_TRACER_HPP_
#define SRC_INC_COMPUTE_NODE_TRACER_HPP_

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include <ns3/nstime.h>
#include <ns3/event-id.h>
#include <ns3/node-container.h>

#include <tuple>
#include <map>
#include <list>
namespace ns3 {

class Node;
class Packet;

namespace ndn {
namespace inc {

/// @cond include_hidden
struct RequestStats_ResUtil {
  inline void
  Reset()
  {
    m_overallExecutions = 0;
    m_overallCPUUtilized = 0;
    m_overallRAMUtilized = 0;
    m_overallROMUtilized = 0;
    m_overallExecTime = 0;
  }
  double m_overallCPUUtilized = 0;
  double m_overallRAMUtilized = 0;
  double m_overallROMUtilized = 0;
  double m_overallExecTime = 0;
  double m_overallExecutions = 0;
  std::vector<std::string> m_executed_funcNames;
};
/// @endcond

/**
 * @brief INC compute tracer for Interest/Data performance measurements
 */
class IncComputeNodeTracer : public SimpleRefCount<IncComputeNodeTracer> {
public:
  /**
   * @brief Helper method to install tracers on all simulation nodes
   *
   * @param file File to which traces will be written.  If filename is -, then std::out is used
   * @param averagingPeriod How often data will be written into the trace file (default, every half
   * second)
   *
   * @returns a tuple of reference to output stream and list of tracers. !!! Attention !!! This
   * tuple needs to be preserved for the lifetime of simulation, otherwise SEGFAULTs are inevitable
   *
   */
  static void
  InstallAll(const std::string& file, Time averagingPeriod = Seconds(0.5));

  /**
   * @brief Helper method to install tracers on the selected simulation nodes
   *
   * @param nodes Nodes on which to install tracer
   * @param file File to which traces will be written.  If filename is -, then std::out is used
   * @param averagingPeriod How often data will be written into the trace file (default, every half
   * second)
   *
   * @returns a tuple of reference to output stream and list of tracers. !!! Attention !!! This
   * tuple needs to be preserved for the lifetime of simulation, otherwise SEGFAULTs are inevitable
   *
   */
  static void
  Install(const NodeContainer& nodes, const std::string& file, Time averagingPeriod = Seconds(0.5));

  /**
   * @brief Helper method to install tracers on a specific simulation node
   *
   * @param nodes Nodes on which to install tracer
   * @param file File to which traces will be written.  If filename is -, then std::out is used
   * @param averagingPeriod How often data will be written into the trace file (default, every half
   * second)
   *
   * @returns a tuple of reference to output stream and list of tracers. !!! Attention !!! This
   * tuple needs to be preserved for the lifetime of simulation, otherwise SEGFAULTs are inevitable
   *
   */
  static void
  Install(Ptr<Node> node, const std::string& file, Time averagingPeriod = Seconds(0.5));

  /**
   * @brief Helper method to install tracers on a specific simulation node
   *
   * @param nodes Nodes on which to install tracer
   * @param outputStream Smart pointer to a stream
   * @param averagingPeriod How often data will be written into the trace file (default, every half
   * second)
   *
   * @returns a tuple of reference to output stream and list of tracers. !!! Attention !!! This
   * tuple needs to be preserved for the lifetime of simulation, otherwise SEGFAULTs are inevitable
   */
  static Ptr<IncComputeNodeTracer>
  Install(Ptr<Node> node, std::shared_ptr<std::ostream> outputStream,
          Time averagingPeriod = Seconds(0.5));

  /**
   * @brief Explicit request to remove all statically created tracers
   *
   * This method can be helpful if simulation scenario contains several independent run,
   * or if it is desired to do a postprocessing of the resulting data
   */
  static void
  Destroy();

  /**
   * @brief Trace constructor that attaches to the node using node pointer
   * @param os    reference to the output stream
   * @param node  pointer to the node
   */
  IncComputeNodeTracer(std::shared_ptr<std::ostream> os, Ptr<Node> node);

  /**
   * @brief Trace constructor that attaches to the node using node name
   * @param os        reference to the output stream
   * @param nodeName  name of the node registered using Names::Add
   */
  IncComputeNodeTracer(std::shared_ptr<std::ostream> os, const std::string& node);

  /**
   * @brief Destructor
   */
  ~IncComputeNodeTracer();

  /**
   * @brief Print head of the trace (e.g., for post-processing)
   *
   * @param os reference to output stream
   */
  void
  PrintHeader(std::ostream& os) const;

  /**
   * @brief Print current trace data
   *
   * @param os reference to output stream
   */
  void
  Print(std::ostream& os) const;

  /**
   * @brief Print summerized trace data
   *
   * @param os reference to output stream
   */
  void
  PrintSum(std::ostream& os) const;

private:
  void
  Connect();

  void
  FuncExecution(std::vector<std::string> func_properties);


private:
  void
  SetAveragingPeriod(const Time& period);

  void
  Reset();

  void
  PeriodicPrinter();

private:
  std::string m_node;
  Ptr<Node> m_nodePtr;

  std::shared_ptr<std::ostream> m_os;

  Time m_period;
  EventId m_printEvent;
  RequestStats_ResUtil m_stats;
};

/**
 * @brief Helper to dump the trace to an output stream
 */
inline std::ostream&
operator<<(std::ostream& os, const IncComputeNodeTracer& tracer)
{
  os << "# ";
  tracer.PrintHeader(os);
  os << "\n";
  tracer.Print(os);
  return os;
}
} //namespace inc
} // namespace ndn
} // namespace ns3




#endif /* SRC_INC_COMPUTE_NODE_TRACER_HPP_ */
