[![BuildTest](https://github.com/anubhav-cs/lapkt-ipc-2023/actions/workflows/build_test.yml/badge.svg)](https://github.com/anubhav-cs/lapkt-ipc-2023/actions/workflows/build_test.yml)


![LAPKT](cmake/docs/resources/logo/lapkt-low-resolution-logo-color-on-transparent-background.png)

LAPKT aims to make your life easier if your purpose is to create, use or extend basic to advanced Automated Planners. It's an open-source Toolkit written in C++ and Python with simple interfaces that give you complete flexibility by decoupling parsers from problem representations and algorithms. It has been succesfully used in embedded systems, webservices, compilations, replanning and contains some of the high-performance planners from the last International Planning Competition 2014.


# Pypi package(linux and windows): Jump right in!

- Install package

		python3 -m pip install lapkt

- Checkout lapkt options

		lapkt_cmd.py -h

The python script can be found [here](https://github.com/LAPKT-dev/LAPKT-public/blob/Devel2.0/src/python/_package/script/lapkt_cmd.py)
### *Note* - as of Oct 2022 `pypi` package for `macos` is `work in progress` 

## Important platform requirements:  

**Platform agnostic**

1. The directory where the `pip` command installs the scripts, including `lapkt_cmd.py`, is generally on the system `PATH`, if not, it needs to be added manually.
2. Python version [ `3.7`, `3.7`, `3.8`, `3.9`, `3.10` ] are supported

**@Windows**

1. `clingo/gringo` python package requires `MSVCP140.dll` which comes with visual studio redistributable. [latest vc-redist](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)
2. To be able to run `lapkt_run.py` script directly from command line, change the default handler for ".py" files to `Python'.

AUTHORS
=======

- Anubhav Singh <anubhav.singh.er@protonmail.com>
- Miquel Ramirez <miquel.ramirez@gmail.com>
- Nir Lipovetzky <nirlipo@gmail.com>
- Javier Segovia-Aguas <javier.segovia@upf.edu>

CONTENTS
========

1. Overview of toolkit components
2. Building LAPKT
3. Search algorithms implemented
4. Examples
5. Contributing
6. Requirements

1 - OVERVIEW
===========

LAPKT separates search engines from the data structures used to represent
planning tasks. This second component receives the name of 'interface' since
it is indeed the interface that provides the search model to be solved.

Search engine components are meant to be modular, allowing users of LAPKT to
assemble and combine features of different search engines to come up with customized
search strategies, within reason and without sacrificing (much) efficiency. In order to
do so, LAPKT makes heavy use of C++ templates and the Static Strategy design pattern.
At the time of writing this, the modularity and decoupling of components isn't as high 
as I would like, there's still a lot of work to do :)

LAPKT is bound to change rapidly and dramatically over the next months, so please keep
this in mind when using this library.

2 - Introduction to LAPKT 4 Devs
================================


[Build instructions](developersguide/build.md)

`cmake` is the primary tool used to build the LAPKT's C++(backend) source code. We also use it to generate Python/C++ library package which is ready to go as a `pypi` package. 

[Apptainer configuration](Apptainer.ApproximateNoveltySearchTarskiGrounder)




