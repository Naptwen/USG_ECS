This is simple ECS system for c++\
The code is just tested for the short examples\

- how to use
```cpp
struct AA
{
	int x = 0;
	int y = 1;
};

struct BB
{
	int x = 0;
	int y = 1;
};

int testA(AA* A) // must pointer type
{
	//printf("%d %d, %d %d", A.x, A.y, B.x, B.y);
	printf("%d %d", A->x, A->y);
	A->x = A->x + 1;
	printf("%d %d", A->x, A->y);
	printf("Hello world!\n");
	return 1;
}
int testB(BB* B) // must pointer type
{
	//printf("%d %d, %d %d", A.x, A.y, B.x, B.y);
	printf("%d %d", B->x, B->y);
	B->x = B->x + 1;
	printf("%d %d", B->x, B->y);
	printf("Hello world!\n");
	return 1;
}
int testC(AA* A, BB* B)
{
	printf("%d %d, %d %d", A->x, A->y, B->x, B->y);
	printf("Hello world!\n");
	return A->x + B->x;
}

int main(int argc, char** argv)
{
	ECS::World w;
	w.entity().add<ATest>({1,1});
	w.entity().add<BTESTTEST>({2,2});
	w.entity().add<ATest>({3,3}).add<BTESTTEST>({4,4});
	w.Show();
	w.system(testA);
	w.system(testB);
	std::cout << "C return is "<< w.system(testC) << std::endl;
	return 0;
}
```
