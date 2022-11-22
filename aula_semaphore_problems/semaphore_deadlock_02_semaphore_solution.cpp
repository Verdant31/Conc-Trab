#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

// ACESSOS EM ORDENS DIFERENTES PRODUZEM ALTA PROPABILIDADE DE DEADLOCK
// SOLUCAO COM SEMAFOROS (EFICIENTE, TRANSFORMA O PROBLEMA EM SOLUCAO ALTERNADA, O QUE EH BOM PARA CASOS DE APENAS DOIS PROCESSOS)

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

auto empty = semaphore(1);
auto full = semaphore(0);

std::mutex X;
std::mutex Y;

int x = 0;
int y = 0;

void foo()
{
	while (true)
	{
		empty.acquire();

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

		full.release();
	}
}

void bar()
{
	while (true)
	{
		full.acquire();

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

		empty.release();
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
