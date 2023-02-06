#include <iostream>
#include <memory>
#include <array>
#include <mutex>
#include <cassert>
#include <thread>
#include <queue>
#include <random>
#include <semaphore.h>

#define THREAD_NUM 5
#define LOOP 3

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
class philosopher
{
    private:
        std::random_device rnd;     // 非決定的な乱数生成器を生成
        int philosopher_number;
        fork left;
        fork right;

        std::thread th;

        void random_wait()//ランダム時間待機
        {
            std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
            std::uniform_int_distribution<> rand_time(1, 5);
            std::chrono::milliseconds timeout(rand_time(mt)*1000);
            std::this_thread::sleep_for(timeout);//ランダム時間待機
        }
        void think()
        {
            std::cout << "Philosopher" << philosopher_number << " thinking" << std::endl;

            random_wait();
        }
        void eat()
        {
            std::cout << "Philosopher" << philosopher_number << " is eating" << std::endl;

            random_wait();

            std::cout << "Philosopher" << philosopher_number << " has finished eating" << std::endl;
        }
        void task()
        {
            for(int i=0;i<LOOP;i++)
            {
                think();//思考
                //フォークの取得
                if(philosopher_number%2==0)
                {
                    left.get_fork(philosopher_number);//左のフォーク取得
                    right.get_fork(philosopher_number);//右のフォーク取得
                }
                else
                {
                    right.get_fork(philosopher_number);//右のフォーク取得
                    left.get_fork(philosopher_number);//左のフォーク取得
                }

                eat();//食事

                //フォークを置く
                if(philosopher_number%2==0)
                {
                    right.put_fork(philosopher_number);//右のフォークを置く
                    left.put_fork(philosopher_number);//左のフォークを置く
                }
                else
                {
                    left.put_fork(philosopher_number);//左のフォークを置く
                    right.put_fork(philosopher_number);//右のフォークを置く
                }
            }
        }
    public:
        philosopher(fork &left, fork &right, int philosopher_number)
        {
            this->philosopher_number=philosopher_number;
            this->left=left;
            this->right=right;
            std::cout << "Philosopher " << this->philosopher_number << " is reading.." << std::endl;
            this->th=std::thread(&philosopher::task,this);//スレッド作成
        }
        std::thread* get_thread()
        {
            return &th;
        }
};

int main()
{
    std::cout << "start\n";
    std::array<fork, THREAD_NUM> forks;              // フォーク
    std::array<std::shared_ptr<philosopher>, THREAD_NUM> philosophers; // 哲学者

    //fork 設定
    int fork_count = 1;
    for (auto &f : forks)
    {
        f.num = fork_count++;
        sem_init(&f.sem, 0, 1);//1で初期化
        std::cout << "fork " << f.num << std::endl;
    }
    //哲学者 設定
    philosophers[0] = std::make_shared<philosopher>(forks[0],forks[THREAD_NUM - 1], (0 + 1));
    for (int i = 1; i < THREAD_NUM; ++i)
        philosophers[i] = std::make_shared<philosopher>(forks[i],forks[i - 1], (i + 1));

    for (auto &ph : philosophers)
        ph->get_thread()->join();//スレッドの処理が終わるまで待機

    std::cout << "finish\n";
    return 0;
}
