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
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 *			    Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */

#ifndef NFN_INTEREST_RESOLUTION_ENGINE_H
#define NFN_INTEREST_RESOLUTION_ENGINE_H

#include "ns3/inc-orchestration-compute-node.h"
#include "ns3/ptr.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/INC-Computation.hpp"



namespace ns3{
namespace ndn{
    /**
 *  A helper class used to resolve interest for producer class. Implemented with simplified NFN interest parsing logic
 *
 *
 */
    class NfnInterestResolutionEngine
    {
    public:
      enum NRE_Decision{NACK, FETCH, EXECUTE, FORWARD};
      //will resolve the interest and return the fetching decisions,
      //fetching decisions are a list of names that producer needs
      //to pull corresponding content from network before capable of executing the function
      std::pair<NRE_Decision, shared_ptr<vector<Name>>>
      GetFetchDecisions(shared_ptr<const Interest> interest, Ptr<ndn::inc::IncOrchestrationComputeNode> computeNode);

    private:

    bool DataCacheAvailable =true;
      shared_ptr<Name>
	  ExtractFunctionName(shared_ptr<const Interest> interest);

      std::vector<Name>
	  ExtractDataNames(shared_ptr<const Interest> interest);
    };

  } // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCER_H
