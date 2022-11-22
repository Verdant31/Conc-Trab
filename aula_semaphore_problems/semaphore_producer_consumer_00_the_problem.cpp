#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

// PROBLEMA DO PRODUTOR-CONSUMIDOR
// PRODUTOR PRODUZ E CONTINUA SENDO CHAMADO, CONSUMIDOR CONSOME E CONTINUA SENDO CHAMADO

class semaphore
{
	long count;
	std::mutex mtx;
	std::condition_variable cv;

public:
	semaphore(long const c = 0) : count(c) {}

	semaphore(semaphore const &) = delete;
	semaphore(semaphore &&) = default;
	semaphore &operator=(semaphore const &) = delete;
	semaphore &operator=(semaphore &&) = default;
	~semaphore() = default;

	void acquire() // aka "wait", "down", "p"
	{
		auto lock = std::unique_lock<std::mutex>(mtx);
		while (!count)
			cv.wait(lock);
		--count;
	}

	void release() // aka "signal", "up", "v"
	{
		auto lock = std::unique_lock<std::mutex>(mtx);
		++count;
		cv.notify_one();
	}
};

#define N 10

std::mutex mtx;

auto empty = semaphore(N);
auto full = semaphore(0);

std::vector<int> buffer;

void producer()
{
	while (true)
	{
		mtx.lock();

		std::printf("producing values...\n");

		if (buffer.size() < N)
		{
			buffer.push_back(1);
		}

		for (int i = 0; i < buffer.size(); i++)
		{
			printf("%d ", buffer[i]);
		}
		printf("\n");
		std::fflush(stdout);

		mtx.unlock();
	}
}

void consumer()
{
	while (true)
	{
		mtx.lock();

		std::printf("consuming values...\n");

		if (buffer.size())
		{
			buffer.pop_back();
		}

		for (int i = 0; i < buffer.size(); i++)
		{
			printf("%d ", buffer[i]);
		}
		printf("\n");
		std::fflush(stdout);

		mtx.unlock();
	}
}

int main()
{
	std::thread t0(producer);
	std::thread t1(consumer);

	t0.join();
	t1.join();

	return 0;
}
