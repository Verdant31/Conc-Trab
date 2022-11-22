#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

//ACESSOS EM ORDENS DIFERENTES PRODUZEM ALTA PROPABILIDADE DE DEADLOCK

std::mutex X;
std::mutex Y;

int x = 0;
int y = 0;

void foo()
{
	while(true)
	{
		X.lock();
		std::printf("foo locked X...\n");
		
		Y.lock();
		std::printf("foo locked Y...\n");
		
		std::printf("foo is in critical section...\n");
		x++;
		y--;
		std::this_thread::sleep_for(std::chrono::microseconds(1));
		
		std::printf("foo unlocking X...\n");
		X.unlock();
		
		std::printf("foo unlocking Y...\n");
		Y.unlock();
	}
}

void bar()
{
	while(true)
	{
		Y.lock();
		std::printf("bar locked Y...\n");
		
		X.lock();
		std::printf("bar locked X...\n");
		
		std::printf("bar is in critical section...\n");
		x--;
		y++;
		std::this_thread::sleep_for(std::chrono::microseconds(1));
		
		std::printf("bar unlocking Y...\n");
		Y.unlock();
		
		std::printf("bar unlocking X...\n");
		X.unlock();
	}
}

int main()
{
	std::thread t0(foo);
	std::thread t1(bar);
	
	t0.join();
	t1.join();
	
	return 0;
}
