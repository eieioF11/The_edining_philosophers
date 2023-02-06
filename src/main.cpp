#include <iostream>
#include <array>
#include <mutex>
#include <cassert>
#include <thread>
#include <queue>
#include <random>
#include <semaphore.h>

#define THREAD_NUM 5
#define LOOP 3

std::random_device rnd;     // 非決定的な乱数生成器を生成
std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
std::uniform_int_distribution<> rand_time(1, 5);

struct fork
{
    int num;   //フォークナンバー
    sem_t sem; //セマフォ
    void get_fork(int philosopher_number)//フォークの取得
    {
        std::cout << "Philosopher" << philosopher_number << " get fork" << num ;
        sem_wait(&sem);
        std::cout << " DONE " << std::endl;
    }
    void put_fork(int philosopher_number)//フォークを置く
    {
        std::cout << "Philosopher" << philosopher_number << " put fork" << num ;
        sem_post(&sem);
        std::cout << " DONE " << std::endl;
    }
};

void think(int philosopher_number)
{
    std::cout << "Philosopher" << philosopher_number << " thinking" << std::endl;
    std::chrono::milliseconds timeout(rand_time(mt)*1000);
    std::this_thread::sleep_for(timeout);
}
void eat(int philosopher_number)
{
    std::cout << "Philosopher" << philosopher_number << " is eating" << std::endl;

    std::chrono::milliseconds timeout(rand_time(mt)*1000);
    std::this_thread::sleep_for(timeout);

    std::cout << "Philosopher" << philosopher_number << " has finished eating" << std::endl;
}
void task(fork &left, fork &right, int philosopher_number)
{
    for(int i=0;i<LOOP;i++)
    {
        think(philosopher_number);
        if(philosopher_number!=THREAD_NUM-1)
        {
            left.get_fork(philosopher_number);//左のフォーク取得
            right.get_fork(philosopher_number);//右のフォーク取得
        }
        else//THREAD_NUM-1番目の哲学者のフォークを取る順番を変える
        {
            right.get_fork(philosopher_number);//右のフォーク取得
            left.get_fork(philosopher_number);//左のフォーク取得
        }

        eat(philosopher_number);

        if(philosopher_number!=THREAD_NUM-1)
        {
            left.put_fork(philosopher_number);//左のフォークを置く
            right.put_fork(philosopher_number);//右のフォークを置く
        }
        else
        {
            right.put_fork(philosopher_number);//右のフォークを置く
            left.put_fork(philosopher_number);//左のフォークを置く
        }
    }
}

int main()
{
    std::cout << "start\n";
    std::array<fork, THREAD_NUM> forks;              // フォーク
    std::array<std::thread, THREAD_NUM> philosopher; // 哲学者

    int fork_count = 1;
    for (auto &f : forks)
    {
        f.num = fork_count;
        fork_count++;
        sem_init(&f.sem, 0, 1);//1で初期化
    }

    // Philosophers Start reading
    std::cout << "Philosopher " << (0 + 1) << " is reading.." << std::endl;
    philosopher[0] = std::thread(task, std::ref(forks[0]), std::ref(forks[THREAD_NUM - 1]), (0 + 1));

    for (int i = 1; i < THREAD_NUM; ++i)
    {
        std::cout << "Philosopher " << (i + 1) << " is reading.." << std::endl;
        philosopher[i] = std::thread(task, std::ref(forks[i]), std::ref(forks[i - 1]), (i + 1));
    }

    for (auto &ph : philosopher)
        ph.join();
    std::cout << "finish\n";
    return 0;
}
