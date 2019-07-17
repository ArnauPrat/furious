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

## Motivation

Entity Component Systems are effectively using the Relational Model to compose the entities. The entity id or index, is the entities primary-key while components are typically stored in arrays or similar data structures indexed by the entity index. Such data structures are equivalent to tables in databases. 

In Furious, there exists a table for each different component type. Such tables, are split into blocks of a fixed size (64 entries by default) and each position of a block corresponds to an entity. Each block contains additional data such as the block offset, a bitmap to store if a given position in the block is valid, or whether the corresponding component in a position is enabled or not. 


<img src="figures/tables.png" width="330" height="666">
 
On the other hand, systems can be seen as some sort of composition of a declarative select-like query plus an imperative game logic part containing the actual game update code. The select part, is the part where the system expresses the subset of entities the system applies to. For instance, in the above "UpdatePosition" example, the select part could be expressed, in natural language, as "Select all entitities having a Position and a Velocity component". Then, for all the entities resulting from such query, the imperative game logic part is executed. 

The goal of furious is to apply Database concepts such as query compilation and query optimizers to Entity Component Systems.

In traditional ECSs, the user expresses imperatively where and in which order to execute the different systems in the main game loop. For instance, execute a system that implements the AI of the enemies, another that updates all entities positions,then another one that renders those entities with a mesh, etc. The optimal system execution order (which may change as the development of the game evolves) on complex games with tens of systems and components might not be trivial, thus the user must spend time in optimizing their code, something that could be automated by a tool.

In furious, like in traditional ECSs, the code to declare and create entities is written as usual and compiled and linked with the rest of the game code. However, the systems' code is written in the so called "furious scripts". Such scripts, written in C++, are compiled all together with the Furious C++ to C++ compiler (fcc), which produces a single C++ source code file with a set of predefined functions that can be called from game's code, and which are then compiled and linked with the rest of the game and the furious runtime library.

![](figures/furious.png)

Furious scripts are composed by two parts:
* A declarative part (the query) which specifies the entities the system should run on
* An imperative part where the logic of the system is implemented

Given a set of "furious scripts" implementing the game logic, the fcc compiler understands the declarative part (the query) of the furious scripts and for each one, creates an execution plan. Given all the execution plans, the compiler is able to merge them and produce a newly optimized execution plan using a cost model, which in turn is translated to a single C++11 src file implementing the same game logic expressed in the scripts. The optimization process's goal is to run the game logic as fast as possible by:

* minimizing the use of function calls (and no virtual calls) so the produced code is as explicit as possible to the final compiler, so it can optimize register allocation 
* place the code in tight for loops with inlined code, which opertes on aligned memory arrays to be easy to auto-vectorize by the compiler
* take advantage of common system's subqueries to reduce the amount of times the same data is fetched from memory every frame, improving data locality
* understanding dependencies between systems and their components access modes (READ or READ_WRITE), allowing for an automatic and safe parallelization of the execution plans

Additionally, thanks to the source to source furious compiler, system's code can be kept into independent modular scripts, making the code presumably more maintainable since no hand-crafted optimizations need to be performed.

The following is a simple example of a furious script, which implements an "UpdatePosition" system.

```cpp                                                                                                                                                                                                                   

#include <furious/lang/lang.h>      
#include "components/position.h"
#include "components/velocity.h"
                   
   
/** Components code declared in position.h and velocity.h files

struct Position
{
  FURIOUS_COMPONENT(Position);
  float m_x;
  float m_y;
  float m_z;
};

struct Velocity
{
  FURIOUS_COMPONENT(Velocity;
  float m_x;
  float m_y;
  float m_z;
};

**/
                                                                                                                                                                                                                   
BEGIN_FURIOUS_SCRIPT                                                                                                                                                                                               
   
// The imperative part (System's implementation)
struct UpdatePosition                                                                                                                                                                                         
{                                                                                                                                                                                                                                                                                                                                                                                                                                   
  void run(furious::Context* context,                                                                                                                                                                              
           uint32_t id,                                                                                                                                                                                            
           Position* position,                                                                                                                                                                                
           const Velocity* velocity)                                                                                                                                                                            
  {                                                                                                                                                                                                                
        position->m_x += velocity->m_x*context->m_dt;        
        position->m_y += velocity->m_y*context->m_dt; 
        position->m_z += velocity->m_y*context->m_dt; 
  }                                                                                                                                                                                                                
                                                                                                                                                                                                    
};                                                                                                                                                                                                                 
   
// The declarative part (the query)
furious::match<Position, Velocity>().foreach<UpdatePosition>();                                                                                                                                    
                                                                                                                                                                                 
                                                                                                                                                                                                                   
END_FURIOUS_SCRIPT 

```
which would produce the following simple execution plan, which would join tables Position and Velocity, and for each joined entity, would run the update position system.

```
- foreach (4) - "UpdatePosition ()"
   |- join(3)
    |- scan (2) - "Position"
    |- scan (1) - "Velocity"
```

Now, let's supose we add another furious script to our game, that updates the velocity each frame:

```cpp                                                                                                                                                                                                                   

#include <furious/lang/lang.h>      
#include "components/velocity.h"
                   
                                                                                                                                                                                                                   
BEGIN_FURIOUS_SCRIPT                                                                                                                                                                                               
   
// The imperative part (System's implementation)
struct UpdateVelocity                                                                                                                                                                                       
{    
  UpdateVelocity(float acceleration) :
  m_accl(acceleration)
  {
  }
  
  void run(furious::Context* context,                                                                                                                                                                              
           uint32_t id,                                                                                                                                                                                            
           Velocity* velocity)                                                                                                                                                                            
  {                                                                                                                                                                                                         
        velocity->m_x += context->m_dt*m_accl;
        velocity->m_y += context->m_dt*m_accl;
        velocity->m_z += context->m_dt*m_accl;
  }
  
  float m_accl;
                                                                                                                                                                                                    
};                                                                                                                                                                                                                 
   
// The declarative part (the query)
furious::match<Velocity>().foreach<UpdateVelocity>(0.1f);                                                                                                                                    
                                                                                                                                                                                 
                                                                                                                                                                                                                   
END_FURIOUS_SCRIPT 
```

which would produce the following execution plan:

```
- foreach (6) - "UpdateVelocity ()"
   |- scan (5) - "Velocity"
```

then, the fcc compiler, could merge both execution plans as follows:

```
- foreach (4) - "UpdatePosition ()"
   |- join(3)
    |- scan (2) - "Position"
    |- foreach (6) - "UpdateVelocity ()"
       |- scan (5) - "Velocity"
```

chaining the execution of UpdateVelocity with UpdatePosition, and thus, the Velocity table blocks would only be accessed once. Note also that there is an output dependency between UpdateVelocity and UpdatePosition systems, since UpdatePosition reads the Velocity value written by UpdateVelocity. Furious creates a dependency graph between systems to check for dependencies, and provides a mechanism to let the programmer break them when they are cyclic.


## Compilation and Installation

Furious uses CMAKE and the Make toolchain to build on Linux, which is currently the only supported platform. To compile the project:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=<path/to/install/prefix> ..
make && make install
```

## Using Furious 

### Game Engine integration

The furious runtime library defines the following functions, whose implementation is found in the furious_runtime.cpp file, generated by the FCC compiler (the name of the file can be customized) from the furious scripts. These are the means of integrating the furious scripts with your game engine. 

```cpp

// Initializes a furious Database structure
extern void __furious_init(Database* database);                                                                                                                                                                                                                                           

// Executes the frame update code
extern void __furious_frame(float delta,                                                                                                                                                                                                                                                  
                            Database* database,                                                                                                                                                                                                                                           
                            void* user_data);                                                                                                                                                                                                                                             
                                 
// Executes the post frame update code                                
extern void __furious_post_frame(float delta,                                                                                                                                                                                                                                             
                                 Database* database,                                                                                                                                                                                                                                      
                                 void* user_data);    
                                 
// Releases acquired resources                             
extern void __furious_release();   
```

The central object of furious is the ```cpp Database```, which is the one responsible of storing all the data regarding entities, components, tags and references. This database can be accessed from your game engine, as we will see below, to create, remove, update entities etc. Such database, is initialized as follows:

```cpp
#include <furious/furious.h>

// ...
furious::Database* database = new furious::Database();
__furious_init(database);


// ...
__furious_release();
delete database;
```
In order to call the code to update your entities, the ```cpp __furious_frame()``` and ```cpp __furious_post_frame()``` methods are used:

```cpp

// Game loop
while(true) 
{
 //...
 __furious_frame(delta, database, NULL);
 //...
 __furious_post_frame(delta, database, NULL);
 //...
}
```
The first parameter is the delta time form the last frame, the database is the database to update and the last parameter is a pointer to user_data that can be accessed from the systems (more details below).


### Creating/destroying entities

Entities can be created and destroyed at eny point of your code. Furious provides the following macros for such task.

```cpp
#include <furious/furious.h>

// ...
Entity entity = FURIOUS_CREATE_ENTITY(database);
// ...
FURIOUS_DESTROY_ENTITY(entity);
// ...
```
### Definning components

Defining components is straightforward. They are declared as a simple ```cpp struct```, with a macro used to automatically create several methods furious needs. 

```cpp
#include <furious/furious.h>

struct Position 
{
  FURIOUS_COMPONENT(Position);
  
  Position(float x, float y, float z) :
   m_x(x), 
   m_y(y),
   m_z(z)
   {
   }
  
  float m_x;
  float m_y;
  float m_z;
};
```

### Adding, removing and accessing components

```cpp
#include <furious/furious.h>

//...
// Adding a component
FURIOUS_ADD_COMPONENT(entity, Position, 0.0f, 0.0f, 0.0f);

//...
// Getting pointer to a component
Position* position = FURIOUS_GET_COMPONENT(entity, Position);

//...
// Removing a component
FURIOUS_REMOVE_COMPONENT(entity, Position);

```

### Adding and removing tags to/from entities

Furious supports the tagging entities. The tagging of entities, as we will see below, is useful to restrict the application of systems to entities based on predicates using tags. Entities can have multiple tags.

```cpp
#include <furious/furious.h>

//...
// Tagging an entity
FURIOUS_ADD_TAG(entity, "tag");

//...
// Checking if an entity is tagged
bool has_tag = FURIOUS_HAS_TAG(entity, "tag");

//...
// Removing a tag
FURIOUS_REMOVE_TAG(entity, "tag");
```


### Adding and removing references between entities

Furious supports adding references between entities. This is useful to write systems that "transfer" data from one entity to another, such, for instance, to update an entities Transform using a parents Transform in a transform hierarchy.

References have a label that indicates the reference type, which allows having multiple references and apply different systems depending on such references. Currently, references are always directional and 1 to n, that is, an entity can only reference a single entity through a given reference type, but it can be reference by more than one entity.

```cpp
#include <furious/furious.h>

//...
// Tagging an entity
FURIOUS_ADD_REFERENCE(entity1, "refname", entity2);

//...
// Getting a reference
Entity entityX = FURIOUS_GET_REFERENCE(entity, "refname");

//...
// Removing a reference
FURIOUS_REMOVE_REFERENCE(entity, "refname");
```

### Writting Furious scripts


```cpp                                                                                                                                                                                                                   

#include <furious/lang/lang.h>      
#include "components/position.h"
#include "components/velocity.h"
                   
   
/** Components code declared in position.h and velocity.h files

struct Position
{
  FURIOUS_COMPONENT(Position);
  float m_x;
  float m_y;
  float m_z;
};

struct Velocity
{
  FURIOUS_COMPONENT(Velocity;
  float m_x;
  float m_y;
  float m_z;
};

**/
                                                                                                                                                                                                                   
BEGIN_FURIOUS_SCRIPT                                                                                                                                                                                               
   
// The imperative part (System's implementation)
struct UpdatePosition                                                                                                                                                                                         
{                                                                                                                                                                                                                                                                                                                                                                                                                                   
  void run(furious::Context* context,                                                                                                                                                                              
           uint32_t id,                                                                                                                                                                                            
           Position* position,                                                                                                                                                                                
           const Velocity* velocity)                                                                                                                                                                            
  {                                                                                                                                                                                                                
        position->m_x += velocity->m_x*context->m_dt;        
        position->m_y += velocity->m_y*context->m_dt; 
        position->m_z += velocity->m_y*context->m_dt; 
  }                                                                                                                                                                                                                
                                                                                                                                                                                                    
};                                                                                                                                                                                                                 
   
// The declarative part (the query)
furious::match<Position, Velocity>().foreach<UpdatePosition>();                                                                                                                                    
                                                                                                                                                                                 
                                                                                                                                                                                                                   
END_FURIOUS_SCRIPT
```

#### Predicates 

#### Expand



