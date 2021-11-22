# In-Network Compute Simulator - incSIM

This repository holds the source code of the study:

> **HYdrO: Hybrid Orchestration of In-Network Computations for the Internet of Things** by Uthra Ambalavanan, Dennis Grewe, Naresh Nayak, Joerg Ott.
In Proceedings of the 11th International Conference on the Internet of Things (IoT'21), St. Gallen, Switzerland, DOI:https://doi.org/10.1145/3494322.3494331.

The code allows the users to reproduce and the results reported in the study but WITHOUT ANY WARRANTY.
Please cite the above paper when reporting, reproducing or extending the results.

In case of questions, please contact the authors listed below.

## Introdution

In the last decades, advances in virtualization and networking technologies
allows of re-thinking the way distributed applications and services are deployed
and operated. Research activities around the globe are concerned about converging
compute and network resources closer together to optimize the servicing of
distributed applications including new types of communication architectures such
as data-oriented networks. Examples for such development to support
distributed computations directly within network devices includes
[Named Function Networking](https://ieeexplore.ieee.org/abstract/document/6940518)
or [Compute First Networking](https://dl.acm.org/doi/10.1145/3357150.3357395)
running on top of a data-oriented network substrate, e.g., the
[Named Data Networking](https://dl.acm.org/doi/10.1145/2656877.2656887) architecture.

incSIM is a simulation module to investigate aspects of in-network computing systems.
The module is based on the on the popular discrete-event network simulator
[ns-3](https://www.nsnam.org/) using principles of the ns-3 extension
[ndnSIM](https://ndnsim.net/current/) (v2.8) to simulate protocol aspects of the
[Named Data Networking](https://named-data.net/) architecture.
In order to simulate the behavior of distributed compute solutions, incSIM
implements the compute resolution strategy *Find-or-Execute* (FoX) of the
[Named Function Networking](https://ieeexplore.ieee.org/abstract/document/6940518)
architecture.
Finally, incSIM also provisions the ability to enable ochestration machanisms to
influence the behavior of an in-network compute system (e.g., forwarding, storage, etc.).

# Build and Run

The following steps provide information on how to quickly install and run incSIM.
Please make sure you have installed all required packages before downloading and building the sources.
Further details on pre-requisites, installation details or troubleshooting information can be found
in the [Install Guide](./doc/INSTALL.md).

```sh
ubuntu@ubuntu:~$ sudo apt install build-essential libsqlite3-dev libboost-all-dev libssl-dev git python3-setuptools castxml
```

The following commands download all pieces required for incSIM from repositories:

```sh
# create a directory to build the simulator
ubuntu@ubuntu:~$ mkdir incSIM
ubuntu@ubuntu:~$ cd incSIM
# download all components
ubuntu@ubuntu:~/incSIM$ git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3
ubuntu@ubuntu:~/incSIM$ git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen
ubuntu@ubuntu:~/incSIM$ git clone --recursive https://github.com/named-data-ndnSIM/ndnSIM.git ns-3/src/ndnSIM
ubuntu@ubuntu:~/incSIM$ git clone https://github.com/boschresearch/incSIM.git ns-3/src/incSIM
# copy the simulation scenarios to the ns-3 execution directory
ubuntu@ubuntu:~/incSIM$ cp -a ns-3/src/incSIM/scratch/. ns-3/scratch/
```

After receiving the source files, you have to build the simulator components
(that might take a while when doing it the first time).

```sh
ubuntu@ubuntu:~$ cd ns-3
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf configure
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf
```

You can run the HYdrO orchestration scenario with the following command (using default parameters).

```sh
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf --run "nfn-hydro"
```

## More about ...

* [Design of incSIM](./doc/DESIGN.md)
* [Install Guide](./doc/INSTALL.md)
* [License](./LICENSE)
* [Contributors](./Contributors.md)

#### Simulation Scenarios

incSIM is used to evaluate performance metrics of concepts presented in literature.
The following list provides pointers to simulation scenarios:

* [HYdrO: Hybrid Orchestration of In-Network Computations for the Internet of Things](./doc/HYDRO.md)
  * Uthra Ambalavanan, Dennis Grewe, Naresh Nayak, Joerg Ott. 2021. HYdrO: Hybrid Orchestration of In-Network Computations for the Internet of Things. Proceedings of the 11th International Conference on the Internet of Things (IoT'21), St. Gallen, Switzerland, DOI:https://doi.org/10.1145/3494322.3494331

## License

Most of the sources of incSIM are licensed under the GNU GENERAL PUBLIC LICENSE version 3. Please check the [LICENSE-GPL3](./LICENSE-GPL3) for further details.

Besides this, some of the required ns-3 sources are licensed under the GNU GENERAL PUBLIC LICENSE version 2, namely
* udp-orchestration-node-app.cpp
* udp-orchestration-node-app.hpp
* udp-orchestration-communication.cpp
* udp-orchestration-communication.hpp
* wscript

Please check the [LICENSE-GPL2](./LICENSE-GPL2) for further details.

## Helpful Links

* [ndnSIM Getting Started](https://ndnsim.net/2.8/getting-started.html)
* [ndnSIM Examples](https://ndnsim.net/2.8/examples.html)
* [ndnSIM Tutorial](https://ndnsim.net/1.0/tutorial.html)

## Contacts:

* Uthra Ambalavanan [uthra.ambalavanan at de.bosch.com]
* Dennis Grewe [dennis.grewe at de.bosch.com]
