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
 * Based on the upd-echo-server-helper implementation of ns3
 * https://www.nsnam.org/doxygen/udp-echo-helper_8h_source.html
 * Originated by Mathieu Lacage <mathieu.lacage@sophia.inria.fr> under GPLv3
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#ifndef NDN_ORCHESTRATION_HELPER_H
#define NDN_ORCHESTRATION_HELPER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/ptr.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"

namespace ns3 {
namespace ndn {
namespace inc {

/**
 * @ingroup ndn-helpers
 * @brief A helper to make it easier to instantiate an ns3::ndn::App applications
 *        on a set of nodes
 */
class NdnOrchestrationHelper {
public:
  /**
   * \brief Create an NdnAppHelper to make it easier to work with Ndn apps
   *
   * \param app Class of the application
   */
  NdnOrchestrationHelper(const std::string& prefix);

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
   * Install an ns3::NdnConsumer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param nodeName The node on which an NdnConsumer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer
  Install(std::string nodeName);

  ApplicationContainer
  Install (Ptr<Node> node, std::vector<std::string> function_enable_list) const;

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

/**
 * @brief An application that can be created using the supplied callback
 *
 * Example:
 *
 *     class SomeApp
 *     {
 *     public:
 *       SomeApp(size_t initParameter);
 *       ...
 *     };
 *
 *     FactoryCallbackApp::Install(node, [] () -> shared_ptr<void> {
 *         return make_shared<SomeApp>(42);
 *       })
 *       .Start(Seconds(1.01));
 */
class FactoryCallbackApp : public Application
{
public:
  typedef std::function<shared_ptr<void>()> FactoryCallback;

  FactoryCallbackApp(const FactoryCallback& factory);

public:
  static ApplicationContainer
  Install(Ptr<Node> node, const FactoryCallback& factory);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication();

  virtual void
  StopApplication();

private:
  FactoryCallback m_factory;
  std::shared_ptr<void> m_impl;
};
} //namespace inc
} // namespace ndn
} // namespace ns3

#endif // NDN_APP_HELPER_H
