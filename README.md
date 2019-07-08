# furious
[![Build Status](https://travis-ci.org/ArnauPrat/furious.svg?branch=master)](https://travis-ci.org/ArnauPrat/furious)

Furious is an experimental Entity Component System  (ECS) library for game engines written in C++11. As a research prototype, its main goal is to serve as a research platform to investigate the interesection of Modern database systems and Game Engines. 

## ECS Overview

Entity Component Systems, which are very popular nowadays in commercial game engines such as Unity.

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/0_Byw9UMn9g/0.jpg)](https://www.youtube.com/watch?v=0_Byw9UMn9g)

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/p65Yt20pw0g/0.jpg)](https://www.youtube.com/watch?v=p65Yt20pw0g)

In entity component systems, entities, usually represented as an **id** or an **index** to one or more arrays containing the so-called **components**. A component is typically either a simple or complex data type storing a semantically undivisible pice of data. Entities are then build as a composition of components, compared to other paradigms used in game engines, where entities are typically build, for instance, using inheritance. 

For instance, a player on a First Person Shooter (FPS) game, could be build as an entity having the following components: a mesh, a transform (position, rotation, scale), a rigid-body, a health pool, a player-controller, etc.

In ECSs, components do not contain any-kind of game logic. Game Logic is expresed in separate objects or structures called **systems**. Systems, are meant to be executed on those entities fulfilling a certain condition. Typically, this condition is of the form of all entities having a subset of components. For example, a system UpdatePosition might be in charge of updating the position of all entities having a transform and a velocity component every frame.

Such a way of building entities and expressing logic, have several advantages over other paradigms traditionally used in game engines:
* Flexibility: Composition is more flexible than inheritance when it comes to build complex entities, preventing the appearance of the so-called "diamond of death".
* Data locality: Since components contain only tightly semantically related data, and systems are build to operate only on such data in a streaming fashion (tight loops), spatial locality is enforced and CPU prefetchers exploited.
* Code locality: Since the logic is stored in systems, and systems are run on all entities that qualify, code locality is achieved.

## Entity Component Systems and Databases

Entity Component Systems are effectively using the Relational Model to compose the entities. The entity id or index, is the entities primary-key while components are typically stored in arrays or similar data structures indexed by the entity index. Such data structures are equivalent to tables in databases. 

On the other hand, systems can be seen as some sort of composition of a declarative select-like query plus an imperative game logic part containing the actual game update code. The select part, is the part where the system expresses the subset of entities the system applies to. For instance, in the above "UpdatePosition" example, the select part could be expressed, in natural language, as "Select all entitities having a Position and a Velocity component". Then, for all the entities resulting from such query, the imperative game logic part is executed. 

## The furious approach

The goal of furious is to apply Database concepts such as query compilation and query optimizers to Entity Component Systems. 
Like in traditional ECSs, the code to declare and create entities is written as usual and compiled and linked with the rest of the game code. However, the systems' code is written in the so called "furious scripts". Such scripts, written in C++, are compiled alltogether with the Furious C++ to C++ compiler (fcc), which produces a single C++ source code file which is then compiled and linked with the rest of the game and the furious runtime library.

Such an approach allows the fcc compiler to have a complete view of all (or part of) the game logic expressed as furious scripts. With such a global view, the fcc compiler can produce a single C++11 src file that implements the expressed game logic. 

The fcc compiler works under the assumption that logic of modern games is complex and is decomposed into tens of systems using tens or hundreds of different components, forming a complex bipartite graph between systems and components. 
Finding the optimal execution order of such systems is not trivial. For this reason and thanks to the global view fcc has of the game logic, fcc can build and optimize an execution plan with the goal of minimizing the amount of work and memory accesses, improve data locality, and the overall execution time, similar how Databases optimize queries. 

Moreover, the produced code makes use of tight for loops, no virtual function calls and operates in chunks of consecutive data, which alltogether allows the final C++ compiler to highly optimize the code and auto-vectorize it. 

![](figures/furious.png)

## Compilation and Installation

Furious uses CMAKE and the Make toolchain to build on Linux, which is currently the only supported platform. To compile the project:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=<path/to/install/prefix> ..
make && make install
```


