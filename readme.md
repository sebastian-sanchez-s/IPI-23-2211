This repository contains code generated in the winter research 
project IPI-23-2211 and computes thoese standard young tableux
that represent outer sums.

### Usage

You must git clone the project and its submodules with:
```sh
git clone --recurse-submodules git@github.com:sebastian-sanchez-s/IPI-23-2211.git
```
A Dockerfile is available to avoid dependency issues. If you haven't installed
docker yet, follow the instructions according to your platform in
the [docker website](https://docs.docker.com/engine/install/).
Once installed and running, start building the docker image with:
```sh
docker build -t <img_name> .
```
This command will download the required dependencies and compile the application.
After the image has been built, you can launch it using:
```sh
docker run -it <img_name>
```
After this step you should find yourself with a shell prompt.
In order to run the application for a table with C columns and R rows type:
```sh
./runseq.sh C R
```
This action will result in the creation of three separated directories,
each containing files related to tables of distinct dimensions:
- feasible/: feasible tableaux.
- banned/: banned tables that are intrinsic to that file.
- raw/: feasible and banned data generated in each thread.

### Algorithm

#### Design Pattern

The application comprises two fundamental components: Producers and Consumers.
Producers are responsible for constructing standard Young tableaux,
while Consumers are tasked with determining the feasibility of a given table (or solving a table).

#### Objects and File Dependencies


#### Main

Main set up the communication between Producers and Consumers. 

Main-Consumer: Launches Consumers and continuously listens for their availability to solve a table. 

Main-Producer: Initiates Producers with a designated "seed", which represents a partially filled 
table. Producers fill all possible tables based on this seed. 

#### Producer

A Producer receives two parameters: a resource identifier index (implementation detail)
and a starting position for building the table.
1. A minimal configuration is computed, that is, for the given seed,
the Producer fill the cells with the least available number for that position.
2. Once a table is completed, the Producer checks if the table
contains any subtable that was banned for smaller tables. If no banned subtable
is found, the table is sent to a Consumer to solve it.
3. At last, the Producer start going backwards in the table trying to replace the number 
already inserted in a cell. If a replacement is found, it goes forwards again. This proccess
continues till we cannot go any backwards (ie. it reached the position
received as the second parameter).

#### Consumer

A consumer reads the tables to solve from stdin, after sending a signal to
stdout (print 1). Once a table is read, the inequalities are build for the
cddlib solver. If a table is feasible, its elements are saved in the
`raw/PcNCOLrNROWtINDX` file, where (NCOL, NROW) is the dimension of the table
and INDX is the consumer identification (each consumer writes to its own file).
Non-feasible tables are saved in the same way, but replacing the initial P with
an N.
