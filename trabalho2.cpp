#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

#define MAX_PETS 16
#define MAX_SLEEP_US 4
#define NUM_THREADS 100

void do_stuff(int const id, char const *kind, char const *action)
{
	std::printf("%s started %s...\n", kind, action);
	std::this_thread::sleep_for(std::chrono::microseconds(std::rand() % MAX_SLEEP_US));
	std::printf("%s stopped %s...\n", kind, action);
}

void cat(int const);
void dog(int const);

int main()
{
	auto pets = std::vector<std::thread>(NUM_THREADS);

	for (int i = 0; i < pets.size(); ++i)
	{
		pets.at(i) = std::thread(i % 2 ? cat : dog, i);
	}

	for (int i = 0; i < pets.size(); ++i)
	{
		pets.at(i).join();
	}

	return 0;
}
/*
	CURITIBA 11/2022
	UNIVERSIDADE POSITIVO
	PROGRAMACAO CONCORRENTE/DESENVOLVIMENTO DE SISTEMAS

	TRABALHO 2
	- ADAPTACAO DO "PROBLEMA DO BANHEIRO UNICO" (VER DETALHES NA ESPECIFICACAO)

	TAREFA
	- COMPLETAR/COMPLEMENTAR AS FUNCOES "CAT" E "DOG" NO FINAL DESTE ARQUIVO

	REGRAS
	- VOCE PODE ADICIONAR CODIGO A VONTADE DESDE QUE SEJA A PARTIR DA LINHA COM O COMENTARIO "TODO"
	- VOCE PODE INCLUIR CABECALHOS A VONTADE
	- NADA DO QUE ESTA ESCRITO PODE SER APAGADO

	INFORMACOES IMPORTANTES
	- A ACAO "EATING" EH CRITICA, A ACAO "PLAYING" EH NAO-CRITICA
	- DEVE HAVER EXCLUSAO MUTUA ENTRE GATOS E CACHORROS NA AREA DE COMIDA
	- O NUMERO DE PETS NA AREA DE COMIDA NAO PODE ULTRAPASAR O VALOR DA MACRO "MAX_PETS"
	- NAO DEVE HAVER STARVATION DE GATOS OU CACHORROS

	DICAS
	- HA UMA CLASSE "SEMAFORO" DISPONIVEL PARA USO
	- LEMBRE-SE DE COMPILAR PARA C++11 (-std=c++11) OU SUPERIOR
	- A COMPREENSAO DO CODIGO EH PARTE DO TRABALHO
*/

// 1. Enquanto houver cães na área de comida, os
// gatos que chegarem devem esperar, mas os cães que chegarem podem continuar entrando
// até o limite do espaço.

// 2. Quando o último cão sair, os gatos que estavam esperando podem
// começar a entrar e os cães que chegarem devem esperar. (O processo é recíproco para o
// caso em que gatos ocupem a área primeiro.)

// 3. se houver cães comendo e chegar um gato, os cães que
// chegarem depois deste gato devem parar de entrar para que o gato possa entrar assim que
// o último cão que está lá dentro sair. (O processo é recíproco para caso haja gatos comendo
// e chegue um cão.)
//////////////////////////////////////////////////////////////
auto empty = semaphore(MAX_PETS);
auto full = semaphore(0);
std::mutex mtx;

int dogs_count = 0;
int cats_count = 0;

std::vector<int> buffer;
std::vector<int> animalsEating;

bool dogsAreEating = false;
bool catsAreEating = false;

bool dogsAreHungry = false;
bool catsAreHungry = false;

bool findAnimal(int id)
{
	bool find = false;
	for (int i = 0; i < animalsEating.size(); i++)
	{
		if (animalsEating[i] == id)
		{
			find = true;
			break;
		}
	}
	return find;
}

void cat(int const id)
{
	while (true)
	{
		do_stuff(id, "cat", "playing");
		std::printf("%d cats count \n", cats_count);
		// Se os cachorros estão pedindo pra comer, o gato não pode mais entrar
		if (dogsAreHungry && not(cats_count >= MAX_PETS))
			break;
		// Se o numero de gatos já está no limite, não pode mais entrar gato
		if (cats_count >= MAX_PETS)
		{
			cats_count = 0;
			break;
		}
		// Se nenhuma das afirmativas acima é verdade, então o gato pode comer
		empty.acquire();
		mtx.lock();

		dogsAreEating = false;
		catsAreEating = true;
		cats_count++;

		do_stuff(id, "cat", "eating");

		mtx.unlock();
		empty.release();
	}
}

void dog(int const id)
{
	while (true)
	{
		do_stuff(id, "dog", "playing");
		std::printf("%d dogs count \n", dogs_count);

		// Se os gatos estão pedindo pra comer, o cachorro não pode mais entrar
		if (catsAreHungry && not(dogs_count >= MAX_PETS))
			break;
		// Se o numero de cachorros já está no limite, não pode mais entrar cachorro
		if (dogs_count >= MAX_PETS)
		{
			dogs_count = 0;
			break;
		}
		// Se nenhuma das afirmativas acima é verdade, então o cachorro pode comer
		full.acquire();
		mtx.lock();

		catsAreEating = false;
		dogsAreEating = true;
		dogs_count++;

		do_stuff(id, "dog", "eating");

		dogs_count--;

		mtx.unlock();
		empty.release();
	}
}

//////////////////////////////////////////////////////////////
