#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random> // для std::random_device и std::mt19937
#include <thread>
#include <vector>
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

int getRandomNumber(int min, int max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
    // Равномерно распределяем рандомное число в нашем диапазоне

    int randomNumber = static_cast<int>(rand() * fraction * (max - min + 1) + min); // генерируем случайное число в заданом диапазоне
    return randomNumber; // возвращаем случайное число в заданом диапазоне
}

void CreateVectors(std::vector<std::vector<int>>& vA)
{

    size_t i = getRandomNumber(100000, 999999);

    for (auto& e : vA) {
        for (auto& x : e)
            x = i++;
    };
}

template <typename T>
void PrintVector(const std::vector<std::vector<T>>& vA)
{
    for (const auto e : vA) {
        for (const auto x : e) {
            std::cout << x << ' ';
        }
        std::cout << '\n';
    };
}
std::vector<std::vector<int64_t>> vMulti(const std::vector<std::vector<int>>& vA, const std::vector<std::vector<int>>& vB)
{
    if (vA[vA.size() - 1].size() != vB.size()) {
        std::vector<std::vector<int64_t>> error(0);
        return error;
    }
    const size_t M = vA.size();
    const size_t N = vB[vB.size() - 1].size();
    const size_t K = vB.size();
    size_t i = 0;
    std::vector<std::vector<int64_t>> vMulti(M, std::vector<int64_t>(N));

    std::vector<std::thread> threads(M);

    for (size_t m = 0; m < M; ++m) {

        for (size_t n = 0; n < N; ++n) {
            int64_t sum_m_n = 0;

            for (size_t k = 0; k < K; ++k) {

                sum_m_n += vA[m][k] * vB[k][n];
                //                ++i;
            }

            vMulti[m][n] = sum_m_n;
        }
    }
    // can't use const auto& here since .join() is not marked const

    std::cout << "COUNT   " << i << '\n';
    return vMulti;
}

int main()
{
    std::mt19937_64 mersenne(static_cast<int64_t>(time(0))); // инициализируем Вихрь Мерсенна стартовым числом основаным на времени
    srand(static_cast<int64_t>(mersenne())); // в качестве стартового числа используем Вихрь Мерсенна случайн  ым стартовым числом//
    rand(); // сбрасываем первый результат
    setlocale(LC_ALL, "ru_RU");
    //    std::ios::sync_with_stdio(false);
    //    std::cin.tie(nullptr);

    Timer t;

    const size_t M = 1152;
    const size_t N = 1152;
    const size_t K = 1152;
    std::vector<std::vector<int>> vA(M, std::vector<int>(K, 1));
    std::vector<std::vector<int>> vB(K, std::vector<int>(N, 2));

    CreateVectors(vA);
    CreateVectors(vB);

    //    std::cout << "\nFirst matrix\n";
    //    PrintVector(vA);
    //    std::cout << "\nSecond matrix\n";
    //    PrintVector(vB);

    {

        Timer timer;
        std::vector<std::vector<int64_t>> vC = vMulti(vA, vB);

        std::cerr << '\n'
                  << " vMulti TIME "
                  << timer.elapsed() << " milliseconds" << std::endl;
        //        std::cout << "\nResult matrix\n";
        //        PrintVector(vC);
    }
    std::cerr << '\n'
              << " MAIN TIME "
              << t.elapsed() << " milliseconds" << std::endl;

    //    std::cout << std::setw(5) << getRandomNumber(1, 99) << " \n"; // из заданого диапазона
    return EXIT_SUCCESS;
}
