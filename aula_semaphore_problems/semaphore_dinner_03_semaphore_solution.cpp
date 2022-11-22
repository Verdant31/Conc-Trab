#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <semaphore> //(C++20 APENAS)

#define NUM_THREADS 5

// PROBLEMA DO JANTAR DOS FILOSOFOS
// SOLUCAO EFETIVA E EFICIENTE COM SEMAFOROS
//(INFELIZMENTE AINDA OS SEMAFOROS EM C++ AINDA NAO POSSUEM CONSTRUTOR DEFAULT)

std::mutex mtx;

enum
{
	THINKING,
	HUNGRY,
	EATING
};

int philosophers[NUM_THREADS] = {{0}};

std::binary_semaphore semaphores[NUM_THREADS] = {
	std::binary_semaphore(0),
	std::binary_semaphore(0),
	std::binary_semaphore(0),
	std::binary_semaphore(0),
	std::binary_semaphore(0)};

int right(int id)
{
	return (id + NUM_THREADS - 1) % NUM_THREADS;
}

int left(int id)
{
	return (id + 1) % NUM_THREADS;
}

void think(int id)
{
	std::printf("philosopher %d is thinking...\n", id);

	std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void eat(int id)
{
	std::printf("philosopher %d is eating...\n", id);

	std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void test(int id)
{
	if (philosophers[id] == HUNGRY && philosophers[right(id)] != EATING && philosophers[left(id)] != EATING)
	{
		philosophers[id] = EATING;

		std::printf("philosopher %d acquired chopsticks...\n", id);

		semaphores[id].release();
	}
}

void take_chopsticks(int id)
{
	mtx.lock();

	philosophers[id] = HUNGRY;

	test(id);

	mtx.unlock();

	semaphores[id].acquire();
}

void put_chopsticks(int id)
{
	mtx.lock();

	std::printf("philosopher %d released chopsticks...\n", id);

	philosophers[id] = THINKING;

	test(right(id));
	test(left(id));

	mtx.unlock();
}

void exist(int const id)
{
	while (true)
	{
		think(id);

		take_chopsticks(id);

		eat(id);

		put_chopsticks(id);
	}
}

int main()
{
	std::thread threads[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; ++i)
	{
		threads[i] = std::thread(exist, i);
	}

	for (int i = 0; i < NUM_THREADS; ++i)
	{
		threads[i].join();
	}

	return 0;
}
