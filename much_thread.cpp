/*TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp
LIBS += -pthread -lboost_thread -lboost_system 
*/

#include <boost/thread.hpp>
#include <chrono>
#include <iostream> //Для вывода
#include <thread> //Для использования пространства thread::
using namespace std;
class Timer {
private:
    // Псевдонимы типов используются для удобного доступа к вложенным типам
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<clock_t> m_beg;

public:
    Timer()
        : m_beg(clock_t::now())
    {
    }
    void reset()
    {
        m_beg = clock_t::now();
    }

    double elapsed() const
    {
        return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count() * 1000;
    }
};

int main()
{
    size_t cores_count = boost::thread::physical_concurrency() - 1; //Узнаем к-во ядер
    cores_count = thread::hardware_concurrency() / 2 - 1;
    cout << cores_count << '\n';
    const size_t loop = 100000;
    vector<thread> threadsv;
    threadsv.reserve(loop);
    Timer t;
    int m = 0;
    for (size_t i = 1; i <= loop; ++i) {
        threadsv[i] = std::thread([&m]() { ++m; });
        threadsv[i].join();
    }
    
    for (auto &thr : threads) {
      thr.join();
  }

     std::cout << t.elapsed() << std::endl;

    std::cout << '\n';
    return EXIT_SUCCESS;
}
