#  :pushpin: Version 1.1.0
USG (c) August 6, 2023.

# :stuck_out_tongue_winking_eye: Introduction
This ecs library draws significant inspiration from the fantastic "flecs" library by SanderMertens, 
which is written in C and C++ and has been a valuable resource. 
Despite the possibility of unpredictable errors in the current version, 
the flexibility and functionality it offers make it a valuable asset in my projects. 
With this ecs system, I efficiently manage entities, components, and systems, streamlining development and enhancing application performance. 
Thanks to "flecs" and its influence, my own ecs implementation has been a useful for my development process.


# :snowflake: Requirements
* C++ 11
* STL container

# :tulip: Example Image License
WoodCutter image license OGA-BY 3.0 License\
Author CraftPix.net 2D Game Assets\

# :shell: OpenGL 3D Own Simple Engine system (Camera, OBJECT Loader, Light, System PipeLine, Components, Collision, I/O samples)
https://github.com/Naptwen/USG_ECS/assets/47798805/20cebe56-17be-4fe6-b3ba-d7bff8dcf98f

![test3d](https://user-images.githubusercontent.com/47798805/235448616-0d4eb17e-be0f-4c45-bd6e-821e6709f8e4.gif)

# :coffee: Testing my own SDL game Engine

["Survival Strategy Construction Management Game"]

https://github.com/Naptwen/USG_ECS/assets/47798805/173b23ee-a5d3-4459-9cf2-8a34f0d22d6c

https://github.com/Naptwen/USG_ECS/assets/47798805/4cdf2da8-ff75-4b9f-9613-3dd4aca726e9

https://user-images.githubusercontent.com/47798805/222470263-8412c071-7f00-4779-9082-5c0cc3efd542.mp4

# :mortar_board: How to use
Below code is sample code for the ecs system

```cpp
#include "UECS.h"
#include <iostream>

struct Velocity {
    unsigned int vx;
    unsigned int vy;
};

struct Position {
    unsigned int px;
    unsigned int py;
};

struct Player {};
struct Monster{};

void positionSetting(uecs::Entity& e)
{
    e.get<Position>()->px += 10;
    e.get<Position>()->py += 10;
    std::cout << "start position : " << e.get<Position>()->px << " " << e.get<Position>()->py << std::endl;
}

void move(uecs::Entity& e)
{
   // get the velocity value then add it to the player position
   e.get<Position>()->px += e.get<Velocity>()->vx;
   e.get<Position>()->py += e.get<Velocity>()->vy;
   std::cout << "move position : " << e.get<Position>()->px << " " << e.get<Position>()->py << std::endl;
}
int main() {
    std::cout << "Hello ECS" << std::endl;

    struct Velocity v = { 1, 1 };
    struct Position p = { 0, 0 };
    struct Player tag = {};
    // Creating world
    uecs::World world;
    // Creating player entity without specific name
    uecs::World::Entity player = world.entity().set<Velocity>(v).set<Position>(p).set<Player>(tag);
    //  Creating monster entity with speicific name
    uecs::World::Entity monster = world.entity("monster").set<Velocity>(v).set<Position>(p).set<Monster>({});
    //The "positionSetting" system functions exclusively for entities that have the "monster" struct, 
    //and the "Awake" phase is executed only once during the initial setup.
    world.system<Velocity, Position, Monster>(positionSetting, uecs::PHASE::Awake); 
    // The "move" system operates exclusively on entities with the "player" struct, 
    // updating continuously while the "update_progress" is active.
    world.system<Velocity, Position, Player>(move, uecs::PHASE::Update);
    // Initiate the progress that runs only once.
    world.once_progress();
    for (int i = 0; i < 100; ++i)
    {
        //  the progress that runs continuously.
        world.update_progress();
    }

    std::cout << "Bye ECS" << std::endl;
    return 0;
}

```
