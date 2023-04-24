# Example
WoodCutter image license OGA-BY 3.0 License\
Author CraftPix.net 2D Game Assets\

# testing 3D now
![test3d](https://user-images.githubusercontent.com/47798805/234048463-df949f8a-aaf1-4c85-b3e4-5c40078f6de6.gif)






This is simple ECS system for c++\

https://user-images.githubusercontent.com/47798805/222470263-8412c071-7f00-4779-9082-5c0cc3efd542.mp4


The code is just tested for the short examples

- requirements
** C++ 17

- how2install
** download the header file then add to your cpp
** NO BOT ALLOWED!

-  New ECS system in c++17 BaseHashTableEcs
```cpp
#include "BaseHashTableECS.h"

struct Player {
    int HP = 5;
    bool ATK = false;
    float DEF = 2.5f;
    int LUK = 1;
};

struct Enemy {
    int HP = 10;
    int ATK = 5;
    int DEF = 3;
    int LUK = 2;
};

void test(Player* player, Enemy* enemy){
	std::cout << "test\n";
	std::cout << player->DEF << std::endl;
	std::cout << enemy->DEF << std::endl;
	player->DEF += 1.5f;
	std::cout << player->DEF << std::endl;
	std::cout << enemy->DEF << std::endl;
}

void test2(Player player, Enemy enemy){
	std::cout << "test2\n";
	std::cout << player.DEF << std::endl;
	std::cout << enemy.DEF << std::endl;
}

void addTest(HASH_TABLE& hash_table){
	Player test1;
	Enemy test2;
	hash_table.Add<Player, Enemy>("PlayerA", test1, test2);
	hash_table.Add<Player>("PlayerB", test1);
	hash_table.Add<Enemy, Player>("Enemy", test2, test1);
}

int main(int argc, char** argv) {
	
	HASH_TABLE hash_table;
	addTest(hash_table);
	hash_table.Delete("Enemy");
	hash_table.Filter<Player*, Enemy*>(test);
	hash_table.Filter<Player, Enemy>(test2);

    getchar();

	
	return 0;
}
```

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
