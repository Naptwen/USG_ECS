This is simple ECS system for c++\
The code is just tested for the short examples

- requirements
** C++ 17

- how2install
** download the header file then add to your cpp
** NO BOT ALLOWED!
- how to use
```cpp
#include "usgecs.h"
struct Position
{
	int x = 1;
	int y = 1;
};

struct Velocity
{
	int x = 1;
	int y = 2;
};
//test for void function
void Test(Position* A) // must pointer type with non void
{
	printf("1. Origin Position %d %d\n", A->x, A->y);
	printf("2. New   Position %d %d\n", A->x, A->y);
}
//test for void function
void Move(Position* A, Velocity* B) // must pointer type with non void
{
	printf("1. Origin Position %d %d\n", A->x, A->y);
	A->x = A->x + B->x;
	A->y = A->y + B->y;
	printf("2. New   Position %d %d\n", A->x, A->y);
}
// test for the order doesn't matter and return vector
float MoveDis(Velocity* B, Position* A) // must pointer type with non void
{
	printf("3. Origin Position %d %d\n", A->x, A->y);
	A->x = A->x + B->x;
	A->y = A->y + B->y;
	printf("4. New   Position %d %d\n", A->x, A->y);
	return std::sqrtf(A->x * A->x + A->y * A->y);
}

int main(int argc, char** argv)
{
	ECS::World w;
	w.entity().add<Velocity>({ 1,1 }).add<Position>({ 2,2 });
	w.Show();
	w.system(Test); //test one variable
	w.system(Move); //test two varialbe
	auto entityList = w.system(MoveDis); //test unordered
	int a = 0;
	//test lambda
	w.search<Velocity>().each([&a](Velocity* A) {std::cout << "Lambda" << std::endl; }); 
	w.search<Position>().each([&a](Position* A) {std::cout << "Lambda" << std::endl; }); 
	w.search<Velocity, Position>().each([&a](Velocity* A, Position* B) {std::cout << "Lambda" << std::endl; });
	//return test
	std::cout << "Entity ID : " << 0 << std::endl;
	std::cout << "Posiiton x : " << w.entity(0).get<Position>().x << std::endl;
	std::cout << "Position y : " << w.entity(0).get<Position>().y << std::endl;
	w.entity(0).get<Position>().x  += 10;
	w.entity(0).get<Position>().y  += 10;
	std::cout << "Posiiton x : " << w.entity(0).get<Position>().x << std::endl;
	std::cout << "Position y : " << w.entity(0).get<Position>().y << std::endl;
	//disable component test
	w.entity(0).disable<Position>();
	w.Show();
	//disable component test
	w.entity(0).enable<Position>();
	w.Show();
	//erase component test
	w.entity(0).erase<Position>();
	w.Show();
	//remove entity test
	w.entity(0).remove();
	w.Show();
	return 0;
}
```
