## Scenario HYdrO: Hybrid Orchestration of In-Network Computations

One of the main goals of incSIM are investigations on orchestration and management
aspects of an in-network compute system.
As part of results of the simulations, we investigate that the supported
Named Function Networking resulotion strategy *Find-or-Execute* performs not that
well, when such system is deployed in Edge Computing scenarios (cf. [Resolution strategies for networking the IoT at the edge via named functions](https://ieeexplore.ieee.org/abstract/document/8319235)).
Instead of creating yet another resolution strategy to support special types
of application classes within Named Function Networking, we developed a concept
of *hybrid orchestration* to orchestrate functions, i.e., a centralized instance
gathers the state of the network and provides hints to the individual computing
nodes for an efficient resolution of computation requests. The resulting work
is published in:

* Uthra Ambalavanan, Dennis Grewe, Naresh Nayak, Joerg Ott. 2021. HYdrO: Hybrid Orchestration of In-Network Computations for the Internet of Things. Proceedings of the 11th International Conference on the Internet of Things (IoT'21), St. Gallen, Switzerland, DOI:https://doi.org/10.1145/3494322.3494331

A simulation scenario was set up to experiment the benefit of hybrid orchesration over an NFN network called nfn-hydro.cc.
This scenario helps set up a hierarchical topology with the given number of compute nodes and consumers.
It is possible to program the required number of compute nodes in each tier using the command line inputs while running the scenario.
The centralized orchestrator can be enabled for the hybrid operation by assigning orchestration_switch to true.
The periodicity of operation of hybrid orchestrator can be provided as a command line input.
When the orchestration switch is false, the network behaves as plain NFN.
Similarly there are several parameters that can be tuned while running the simulation scenario.
The list of acceptable command line inputs and their values can be fetched from the simulation scenario file.

A basic command used for running the experimental scenario with hybrid orchestration for a simulation duration of 500s, consisting of 80 consumers, 7 compute nodes distributed across different tiers, 100 functions is given below.
Here the orchestrator periodicity is set to 3s. The seed is used for random number generation while setting up the network topology.

```sh
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf --run "nfn-hydro --orchestration-switch=true --sim-time=200 --consumer-nodes=40 --t1-nodes=1 --t2-nodes=2 --t3-nodes=4  --functions=100 --interval=3 --seed=1"
```

Command to run the scenario with hybrid orchestration disabled is:

```sh
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf --run "nfn-hydro  --orchestration_switch=false --sim-time=200 --consumer-nodes=40 --t1-nodes=1 --t2-nodes=2 --t3-nodes=4 --functions=100 --seed=1"
```

The following command line parameters are supported in this experiment. Further parameters are set within the simulation scenario script (see [nfn-hydro.cc](../scratch/nfn-hydro.cc)).

| Parameter            |  Description                                                                             |   Default       |
|----------------------|------------------------------------------------------------------------------------------|-----------------|
| seed                 | The simulation seed used by the RandomNumberGenerator to reproduce runs.                 |       1         |
| orchestration_switch | A boolean flag indicating to use an orchestrating component or not.                      |     false       |
| t1-nodes             | Number of Tier-1 nodes within the hierarchical topology given as integer.                |       1         |
| t2-nodes             | Number of Tier-2 nodes within the hierarchical topology given as integer.                |       2         |
| t3-nodes             | Number of Tier-3 nodes within the hierarchical topology given as integer.                |       4         |
| functions            | Number of supported functions within the system requested by consumer given as integer.  |       2         |
| consumer-nodes       | Number of consumer nodes requesting for a compute results given as integer.              |       20        |
| interval             | Number of seconds an orchestrating entity pulls state from compute nodes (only applicable when orchestration_switch == true). |      3        |
| sim-time             | Number of seconds to simulate the entire scenario.                                       |      200        |
