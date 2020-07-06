#include <algorithm> // для std::generate(), std::find() и std::min_element()
#include <assert.h>
#include <bitset>
#include <boost/thread.hpp>
#include <cerrno>
#include <chrono>
#include <cmath> // для std::abs()
#include <ctime>
#include <execution>
#include <filesystem>
#include <immintrin.h>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <parallel/algorithm>
#include <random>
#include <ranges>
#include <span>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

using list_type = std::vector<int>;
namespace config {
constexpr int multiplierMin { 2 };
constexpr int multiplierMax { 4 };
constexpr int maximumWrongAnswer { 4 };
}

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

    float randomNumber = static_cast<int>(rand() * fraction * (max - min + 1) + min); // генерируем случайное число в заданом диапазоне
    return randomNumber; // возвращаем случайное число в заданом диапазоне
}

bool playAgain()
{
    while (true) // цикл продолжается до тех пор, пока пользователь не введёт корректное значение
    {
        std::cout << "\nWould you like to play again (y/n)?";
        char answer;
        std::cin >> answer;

        // Переменные типа char могут принимать любые символы из пользовательского ввода, поэтому нам не стоит беспокоиться по поводу возникновения неудачного извлечения

        std::cin.ignore(32767, '\n'); // удаляем лишний балласт

        // Выполняем проверку пользовательского ввода
        if (answer == 'y')
            return true; // возвращаем обратно в caller
        else if (answer == 'n')
            return false; // возвращаем обратно в caller
        else // в противном случае, сообщаем пользователю что что-то пошло не так
            std::cout << "Oops, that input is invalid.  Please try again.\n";
    }
}

int getInteger()
{

    while (true) // цикл продолжается до тех пор, пока пользователь не введёт корректное значение
    {
        int x;
        std::cin >> x;

        // Проверка на предыдущее извлечение
        if (std::cin.fail()) // если предыдущее извлечение оказалось неудачным,
        {
            std::cin.clear(); // то возвращаем cin в 'обычный' режим работы
            std::cin.ignore(32767, '\n'); // и удаляем значения предыдущего ввода из входного буфера
            std::cout << "Oops, that input is invalid.  Please try again.\n";
        } else {
            std::cin.ignore(32767, '\n'); // удаляем лишние значения

            return x;
        }
    }
}

list_type generateNumbers(int start, int count, int multiplier)
{
    list_type numbers(static_cast<list_type::size_type>(count));

    int i { start };

    for (auto& number : numbers) {
        number = ((i * i) * multiplier);
        ++i;
    }

    return numbers;
}

list_type generateUserNumbers(int multiplier)
{
    int start {};
    int count {};

    std::cout << "Start where? ";
    start = getInteger();

    std::cout << "How many? ";
    count = getInteger();

    // Здесь пропущена проверка пользовательского ввода. Все функции подразумевают корректный пользовательский ввод

    return generateNumbers(start, count, multiplier);
}

int getUserGuess()
{
    int guess {};

    std::cout << "> ";
    guess = getInteger();

    return guess;
}

// Ищем значение guess в numbers и удаляем его.
// Возвращаем true, если значение было найдено. В противном случае, возвращаем false
bool findAndRemove(list_type& numbers, int guess)
{
    if (auto found { std::find(numbers.begin(), numbers.end(), guess) };
        found == numbers.end()) {
        return false;
    } else {
        numbers.erase(found);
        return true;
    }
}

// Находим значение в numbers, которое ближе всего к guess
int findClosestNumber(const list_type& numbers, int guess)
{
    return *std::min_element(numbers.begin(), numbers.end(), [=](int a, int b) {
        return (std::abs(a - guess) < std::abs(b - guess));
    });
}

void printTask(list_type::size_type count, int multiplier)
{
    std::cout << "I generated " << count
              << " square numbers. Do you know what each number is after multiplying it by "
              << multiplier << "?\n";
}

// Вызывается, когда пользователь правильно угадывает число
void printSuccess(list_type::size_type numbersLeft)
{
    std::cout << "Nice! ";

    if (numbersLeft == 0) {
        std::cout << "You found all numbers, good job!\n";
    } else {
        std::cout << numbersLeft << " number(s) left.\n";
    }
}

// Вызывается, когда пользователь угадывает число, которого нет в numbers
void printFailure(const list_type& numbers, int guess)
{
    int closest { findClosestNumber(numbers, guess) };

    std::cout << guess << " is wrong!";

    if (std::abs(closest - guess) <= config::maximumWrongAnswer) {
        std::cout << " Try " << closest << " next time.\n";
    } else {
        std::cout << '\n';
    }
}

// Возвращаем false, если игра окончена. В противном случае, возвращаем true
bool playRound(list_type& numbers)
{
    int guess { getUserGuess() };

    if (findAndRemove(numbers, guess)) {
        printSuccess(numbers.size());

        return !numbers.empty();
    } else {
        printFailure(numbers, guess);
        return false;
    }
}
int main()
{

    std::mt19937_64 mersenne(static_cast<long>(time(0))); // инициализируем Вихрь Мерсенна стартовым числом основаным на времени
    srand(static_cast<long>(mersenne())); // в качестве стартового числа используем Вихрь Мерсенна случайн  ым стартовым числом//
    rand(); // сбрасываем первый результат
    setlocale(LC_ALL, "ru_RU");
    //    std::ios::sync_with_stdio(false);
    //    std::cin.tie(nullptr);
    Timer t;
    do {

        int multiplier { getRandomNumber(config::multiplierMin, config::multiplierMax) };
        list_type numbers { generateUserNumbers(multiplier) };

        printTask(numbers.size(), multiplier);

        while (playRound(numbers));

    } while (playAgain());
    using namespace std::literals;
    std::cout << "hello world"s;
    std::cerr << '\n'
              << " MAIN TIME "
              << t.elapsed() << " milliseconds" << std::endl;
    std::cin;
    return EXIT_SUCCESS;
}
