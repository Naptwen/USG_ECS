# Example
WoodCutter image license OGA-BY 3.0 License\
Author CraftPix.net 2D Game Assets\

# testing 3D now
![test3d](https://user-images.githubusercontent.com/47798805/235448616-0d4eb17e-be0f-4c45-bd6e-821e6709f8e4.gif)


This is simple ECS system for c++\

https://user-images.githubusercontent.com/47798805/222470263-8412c071-7f00-4779-9082-5c0cc3efd542.mp4


The code is just tested for the short examples

- requirements
** C++ 11
```cpp
#include <iostream>
#include "UECS.h"

struct POSITION
{
	float x = 1.0f;
	float y = 1.0f;
};


struct VELOCITY
{
	float x = 1.0f;
	float y = 1.0f;
};

int main()
{
	ULOG(USPECIAL, "hello uecs world!");
	uecs::world world = uecs::world();
	auto temp = world.entity().SET<POSITION>({}).SET<VELOCITY>({});
	std::cout << "Before velocity x : " << temp.GET<VELOCITY>().x << std::endl;
	temp.GET<VELOCITY>().x += 1.0f;
	std::cout << "After velocity  x : " << temp.GET<VELOCITY>().x << std::endl;
	ULOG(USPECIAL, "end uecs world!");
	return 0;
}
```
