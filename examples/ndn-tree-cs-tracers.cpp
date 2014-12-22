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

// ndn-tree-cs-tracers.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;
using ns3::ndn::StackHelper;
using ns3::ndn::AppHelper;
using ns3::ndn::GlobalRoutingHelper;
using ns3::ndn::CsTracer;

/**
 * This scenario simulates a tree topology (using topology reader module)
 *
 *    /------\      /------\      /------\      /------\
 *    |leaf-1|      |leaf-2|      |leaf-3|      |leaf-4|
 *    \------/      \------/      \------/      \------/
 *         ^          ^                ^           ^
 *         |          |                |           |
 *    	    \        /                  \         /
 *           \      /  			 \  	 /    10Mbps / 1ms
 *            \    /  			  \ 	/
 *             |  |  			   |   |
 *    	       v  v                        v   v
 *          /-------\                    /-------\
 *          | rtr-1 |                    | rtr-2 |
 *          \-------/                    \-------/
 *                ^                        ^
 *      	  |	 		   |
 *      	   \			  /  10 Mpbs / 1ms
 *      	    +--------+  +--------+
 *      		     |  |
 *                           v  v
 *      		  /--------\
 *      		  |  root  |
 *                        \--------/
 *
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     ./waf --run=ndn-tree-cs-tracers
 */

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-tree.txt");
  topologyReader.Read();

  // Install CCNx stack on all nodes
  StackHelper ndnHelper;
  ndnHelper.SetContentStoreChoice(false);
  ndnHelper.SetContentStore("ns3::ndn::cs::Lru", "MaxSize",
                            "100"); // default ContentStore parameters
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumers[4] = {Names::Find<Node>("leaf-1"), Names::Find<Node>("leaf-2"),
                            Names::Find<Node>("leaf-3"), Names::Find<Node>("leaf-4")};
  Ptr<Node> producer = Names::Find<Node>("root");

  for (int i = 0; i < 4; i++) {
    AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    consumerHelper.SetAttribute("Frequency", StringValue("10")); // 100 interests a second

    // Each consumer will express the same data /root/<seq-no>
    consumerHelper.SetPrefix("/root");
    ApplicationContainer app = consumerHelper.Install(consumers[i]);
    app.Start(Seconds(0.01 * i));
  }

  AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  // Register /root prefix with global routing controller and
  // install producer that will satisfy Interests in /root namespace
  ccnxGlobalRoutingHelper.AddOrigins("/root", producer);
  producerHelper.SetPrefix("/root");
  producerHelper.Install(producer);

  // Calculate and install FIBs
  ccnxGlobalRoutingHelper.CalculateRoutes();

  Simulator::Stop(Seconds(20.0));

  CsTracer::InstallAll("cs-trace.txt", Seconds(1));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}