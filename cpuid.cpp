#include "BigInt.h"
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

class Timer {
private:
  // Псевдонимы типов используются для удобного доступа к вложенным типам
  using clock_t = std::chrono::high_resolution_clock;
  using second_t = std::chrono::duration<double, std::ratio<1>>;

  std::chrono::time_point<clock_t> m_beg;

public:
  Timer() : m_beg(clock_t::now()) {}
  void reset() { m_beg = clock_t::now(); }

  double elapsed() const {
    return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg)
               .count() *
           1000;
  }
};

#ifdef _WIN32
#include <intrin.h>
#include <limits.h>
typedef unsigned __int32 uint32_t;

#else
#include <stdint.h>
#endif

class CPUID {
  uint32_t regs[4];

public:
  explicit CPUID(unsigned i) {
#ifdef _WIN32
    __cpuid((int *)regs, (int)i);

#else
    asm volatile("cpuid"
                 : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
                 : "a"(i), "c"(0));
    // ECX is set to zero for CPUID function 4
#endif
  }

  const uint32_t &EAX() const { return regs[0]; }
  const uint32_t &EBX() const { return regs[1]; }
  const uint32_t &ECX() const { return regs[2]; }
  const uint32_t &EDX() const { return regs[3]; }
};

int main() {

  std::mt19937_64 mersenne(
      static_cast<long>(time(0))); // инициализируем Вихрь Мерсенна стартовым
                                   // числом основаным на времени
  srand(static_cast<long>(
      mersenne())); // в качестве стартового числа используем Вихрь Мерсенна
                    // случайн  ым стартовым числом//
  rand();           // сбрасываем первый результат
  setlocale(LC_ALL, "ru_RU");
  //    std::ios::sync_with_stdio(false);
  //    std::cin.tie(nullptr);
  Timer t;

  CPUID cpuID(0); // Get CPU vendor

  std::string vendor;
  vendor += std::string((const char *)&cpuID.EBX(), 4);
  vendor += std::string((const char *)&cpuID.EDX(), 4);
  vendor += std::string((const char *)&cpuID.ECX(), 4);

  std::cout << "CPU vendor = " << vendor << std::endl;

  std::cerr << '\n'
            << " MAIN TIME " << t.elapsed() << " milliseconds" << std::endl;
  std::cin;
  return EXIT_SUCCESS;
}
