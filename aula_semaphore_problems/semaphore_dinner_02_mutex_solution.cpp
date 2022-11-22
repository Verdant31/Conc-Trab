#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define NUM_THREADS 5

//PROBLEMA DO JANTAR DOS FILOSOFOS
//SOLUCAO EFETIVA MAS INEFICIENTE:
//USAR UM MUTEX GERAL PARA FAZER COM QUE APENAS UM PEGUE OS HASHIS POR VEZ

std::mutex chopsticks[NUM_THREADS];

std::mutex mtx;

void think(int i)
{
	std::printf("philosopher %d is thinking...\n", i);
	std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void eat(int i)
{
	std::printf("philosopher %d is eating...\n", i);
	std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void take_chopstick(int i)
{
	chopsticks[i].lock();
}

void put_chopstick(int i)
{
	chopsticks[i].unlock();
}

void exist(int const id)
{
	int const right = id;
	int const left = (id + 1) % NUM_THREADS;
	
	while(true)
	{
		think(id);
		
		mtx.lock();
		
		take_chopstick(right);
		take_chopstick(left);
		
		eat(id);
		
		put_chopstick(left);
		put_chopstick(right);
		
		mtx.unlock();
	}
}

int main()
{
	std::thread threads[NUM_THREADS];
	
	for(int i = 0; i < NUM_THREADS; ++i)
	{
		threads[i] = std::thread(exist, i);
	}
	
	for(int i = 0; i < NUM_THREADS; ++i)
	{
		threads[i].join();
	}
	
	return 0;
}
