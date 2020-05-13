#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime> // раскомментируйте, если используете Code::Blocks
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random> // для std::random_device и std::mt19937
#include <set>
#include <sstream>
#include <string>

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

int64_t getRandomNumber(int64_t min, int64_t max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
    // Равномерно распределяем рандомное число в нашем диапазоне

    int64_t randomNumber = static_cast<int>(rand() * fraction * (max - min + 1) + min); // генерируем случайное число в заданом диапазоне
    return randomNumber; // возвращаем случайное число в заданом диапазоне
}


int main()
{
    std::mt19937_64 mersenne(static_cast<unsigned int>(time(0))); // инициализируем Вихрь Мерсенна стартовым числом основаным на времени
    srand(static_cast<int64_t>(mersenne())); // в качестве стартового числа используем Вихрь Мерсенна случайным стартовым числом//
    rand(); // сбрасываем первый результат
    setlocale(LC_ALL, "ru_RU");
    //    std::ios::sync_with_stdio(false);
    //    std::cin.tie(nullptr);
    Timer t;

    size_t time = 0;
    std::cout << "Введите высоту ";
    double tempHeight = 0;
    std::cin >> tempHeight;

    const double initialHeight = tempHeight;
    const double gravity = 9.8;
    double height = 0;
    double distanceFallen = 0;
    double speed = 0;
    do {
        distanceFallen = (gravity * time * time) / 2;

        height = initialHeight - distanceFallen;

        double fallenTime = sqrt(2 * distanceFallen / gravity);

        if (height > 0.0) {

            std::cout << "At " << time << " seconds, the ball is at height:\t" << height << " meters\n";
            std::cout << "Speed " << speed << " at " << fallenTime << " seconds\n";

        } else {

            double finTime = sqrt(2 * initialHeight / gravity);
            speed = 4.9 + gravity * (finTime - 1);

            std::cout << "At " << sqrt(2 * initialHeight / gravity) << " seconds, the ball is on the ground.\n";
            std::cout << "Speed " << speed << " at " << finTime << " seconds\n";
        }

        speed = 4.9 + gravity * (fallenTime);
        ++time;

    } while (height > 0.0);

    std::cerr << '\n'
              << " MAIN TIME "
              << t.elapsed() << " milliseconds" << std::endl;

    //    std::cout << std::setw(5) << getRandomNumber(1, 99) << " \n"; // из заданого диапазона
    return EXIT_SUCCESS;
}
