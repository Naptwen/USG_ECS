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
	w.entity().add<Velocity>({ 1,1 }).add<Position>({2,2});
	w.Show();
	w.system(Move);
	auto entityList = w.system(MoveDis);
	for (const auto& kv : entityList)
	{
		//return test
		std::cout << "Entity ID : " << kv.first << " Return value : " << kv.second << std::endl;
		//disable component test
		w.entity(kv.first).disable<Position>();
		w.Show();
		//disable component test
		w.entity(kv.first).enable<Position>();
		w.Show();
		//erase component test
		w.entity(kv.first).erase<Position>();
		w.Show();
		//remove entity test
		w.entity(kv.first).remove();
		w.Show();
	}
	return 0;
}
```
