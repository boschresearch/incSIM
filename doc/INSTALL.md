# Detailed Installation Guide of incSIM

The following document provides the detailed steps to install incSIM.
All the instructions are tested on a the following platforms

* Ubuntu Linux 18.04

## Pre-requisites

incSIM has some core and additional dependencies to be installed - all required
by the ns-3 simulator and the ndnSIM module for ns-3. The following paragraph
provides detailed steps to install all necessary development tools and libraries.

### Core dependencies

* Python >= 3.5
* libsqlite3
* pkg-config
* openssl
* Boost libraries >= 1.54

```sh
ubuntu@ubuntu:~$ sudo apt install build-essential libsqlite3-dev libboost-all-dev libssl-dev git python3-setuptools castxml
```

## Downloading incSIM source

The incSIM package consists of the following pieces:

* a custom branch of NS-3 that contains a few useful patches required by ndnSIM
* a customized python binding generation library (necessary if you want to use NS-3’s python bindings and/or visualizer module)
* the source code of ndnSIM module
* modified source code of ndn-cxx library and NDN Forwarding Daemon (NFD), attached to ndnSIM git repository as git submodules
* the source code of incSIM module to support the development and simulation of in-network compute functions, utilities and exemplary applications

The following commands download all pieces from repositories:

```sh
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
If you previously cloned without '*--recursive*' flag, the correct versions of
submodules can be retrieved using:

```sh
ubuntu@ubuntu:~~/incSIM$ git submodule update --init
```

## Development Configuration of incSIM

As incSIM is based on ndnSIM, it follows the standard NS-3 compilation procedure
to configure and compile incSIM in development mode.

```sh
ubuntu@ubuntu:~$ cd incSIM/ns-3
# to use examples of uther modules use the '--enable-example' target
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf configure --enable-examples
# compile the source (might take a while)
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf
```

### Experiment Configuration of incSIM

When you run real simulation experiments on more powerful machines, you should
configure and compile the simulator using the '*-d optimized*' flag as followed:

```sh
ubuntu@ubuntu:~$ cd incSIM/ns-3
# '-d optimized' flag to configure ns-3 for experimental simulations (speef up)
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf configure -d optimized
# compile the source (might take a while)
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf
```

## Running incSIM

When ns-3 is successfully compiled, you can run default INC scenario using
the following command:

```sh
ubuntu@ubuntu:~$ cd incSIM/ns-3
ubuntu@ubuntu:~/incSIM/ns-3$ ./waf --run "nfn-hydro"
```

To run the sample simulation scenarios with the logging module of ns-3 enabled
(note that this will work only when NS-3 is compiled in debug mode):

```sh
ubuntu@ubuntu:~$ cd incSIM/ns-3
ubuntu@ubuntu:~/incSIM/ns-3$ NS_LOG=ndn.Producer:ndn.Consumer ./waf --run "nfn-hydro"
```
