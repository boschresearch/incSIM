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
 * Based on the code by regents of ndnSIM
 *     https://github.com/named-data-ndnSIM/ndnSIM/blob/master/utils/topology/annotated-topology-reader.hpp
 * Based on the code by Hajime Tazaki <tazaki@sfc.wide.ad.jp>
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 *          Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */

#ifndef INC_NDN_ANNOTATED_TOPOLOGY_READER_H
#define INC_NDN_ANNOTATED_TOPOLOGY_READER_H

#include "ns3/topology-reader.h"
#include "ns3/random-variable-stream.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/incSIM-module.h"
#include "ns3/ndnSIM-module.h"
#include <unordered_map>


namespace ns3{
namespace ndn{
namespace inc{

/**
 * \brief This class is a utility class for reading topology file and initiate nodes, links and initial node function status
 *
 */
class IncNdnAnnotatedTopologyReader : public TopologyReader {
public:
  /**
   * \brief Constructor
   *
   * \param path ns3::Names path
   * \param scale Scaling factor for coordinates in input file
   *
   * \see ns3::Names class
   */
  IncNdnAnnotatedTopologyReader(const std::string& path = "", double scale = 1.0);
  IncNdnAnnotatedTopologyReader();
  virtual ~IncNdnAnnotatedTopologyReader();

  /**
   * \brief Main annotated topology reading function.
   *
   * This method opens an input stream and reads topology file with annotations.
   *
   * \return the container of the nodes created (or empty container if there was an error)
   */

  virtual NodeContainer Read();

  std::pair<NodeContainer, NodeContainer>
  ReadTopology();

  /**
   * \brief Get nodes read by the reader
   */
  virtual NodeContainer
  GetComputeNodeContainer() const;

  virtual NodeContainer
  GetConsumerNodeContainer() const;

  /**
   * \brief Get links read by the reader
   */
  virtual const std::list<Link>&
  GetLinks() const;

  /**
   * \brief Assign IPv4 addresses to all links
   *
   * Note, IPv4 stack should be installed on all nodes prior this call
   *
   * Every link will receive /24 netmask
   *
   * \param base Starting IPv4 address (second link will have base+256)
   */
  virtual void
  AssignIpv4Addresses(Ipv4Address base);

  /**
   * \brief Set bounding box where nodes will be randomly places (if positions are unspecified)
   * \param ulx Upper left x coordinate
   * \param uly Upper left y coordinate
   * \param lrx Lower right x coordinate
   * \param lry Lower right y coordinate
   */
  virtual void
  SetBoundingBox(double ulx, double uly, double lrx, double lry);

  /**
   * \brief Set mobility model to be used on nodes
   * \param model class name of the model
   */
  virtual void
  SetMobilityModel(const std::string& model);

  /**
   * \brief Apply OSPF metric on Ipv4 (if exists) and Ccnx (if exists) stacks
   */
  virtual void
  ApplyOspfMetric();

  /**
   * \brief Save positions (e.g., after manual modification using visualizer)
   */
  virtual void
  SaveTopology(const std::string& file);

  /**
   * \brief Save topology in graphviz format (.dot file)
   */
  virtual void
  SaveGraphviz(const std::string& file);
  /**
   * \brief Get m_inc_compute_nodes
   */
  virtual std::vector<Ptr<IncOrchestrationComputeNode>>
  GetComputeNodes();

  virtual std::vector<Ptr<Node>>
  GetConsumerNodes();

  /**
   * \brief Add a compute node to m_inc_compute_nodes
   */
  void
  AddComputeNode(Ptr<IncOrchestrationComputeNode> value);

  void
  AddConsumerNode(Ptr<Node> node);

  /**
   * \brief Getter for m_initial_func_status
   */
  virtual std::unordered_map<std::string,std::vector<std::string>>
  GetInitialFunctionStatusMap();
 /**
  * \brief Getter for m_initial_func_status
  */
 virtual std::vector<Ptr<INC_Computation>>
 GetFunctionList();

protected:

  /**
   * \brief Create new compute node
   */
  Ptr<IncOrchestrationComputeNode>
  CreateIncComputeNode(const std::string name, double posX, double posY,
                                    uint32_t systemId);
  /**
   * \brief Create new compute node
   */
  Ptr<IncOrchestrationComputeNode>
  CreateIncComputeNode(const std::string name, uint32_t systemId);

  /**
   * \brief Create new consumer node
   */

  Ptr<Node>
  CreateNode(const std::string name, uint32_t systemId);

  Ptr<Node>
  CreateNode(const std::string name, double posX, double posY, uint32_t systemId);

protected:
  /**
   * \brief This method applies setting to corresponding nodes and links
   * NetDeviceContainer must be allocated
   * NodeContainer from Read method
   */
  void
  ApplySettings();

protected:
  std::string m_path;
  NodeContainer m_nodes;
  NodeContainer m_consumer_nodecontainer;
  NodeContainer m_compute_nodecontainer;


private:
  IncNdnAnnotatedTopologyReader(const IncNdnAnnotatedTopologyReader&);
  IncNdnAnnotatedTopologyReader&
  operator=(const IncNdnAnnotatedTopologyReader&);

  Ptr<UniformRandomVariable> m_randX;
  Ptr<UniformRandomVariable> m_randY;

  ObjectFactory m_mobilityFactory;
  double m_scale;

  uint32_t m_requiredPartitions;
  std::vector<Ptr<IncOrchestrationComputeNode>> m_inc_compute_nodes;
  std::vector<Ptr<Node>> m_inc_consumer_nodes;
  std::unordered_map<std::string,std::vector<std::string>> m_initial_func_status;
  std::vector<Ptr<INC_Computation>>  m_function_list;

};
      }//namespace inc
  } // namespace ndn
} // namespace ns3

#endif
