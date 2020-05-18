#include <algorithm>
#include <chrono>
#include <immintrin.h>
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

float getRandomNumber(float min, float max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
    // Равномерно распределяем рандомное число в нашем диапазоне

    float randomNumber = static_cast<float>(rand() * fraction * (max - min + 1) + min) / (float)100000; // генерируем случайное число в заданом диапазоне
    return randomNumber; // возвращаем случайное число в заданом диапазоне
}

void CreateVectors(std::vector<std::vector<float>>& vA)
{

    float i = getRandomNumber(1000000, 9999999);

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

std::vector<std::vector<float>> vMulti(const std::vector<std::vector<float>>& vA, const std::vector<std::vector<float>>& vB)
{
    if (vA[vA.size() - 1].size() != vB.size()) {
        std::vector<std::vector<float>> error(0);
        return error;
    }
    const size_t M = vA.size();
    const size_t N = vB[vB.size() - 1].size();
    const size_t K = vB.size();
    size_t i = 0;
    std::vector<std::vector<float>> vMulti(M, std::vector<float>(N));

    for (size_t m = 0; m < M; ++m) {

        for (size_t n = 0; n < N; ++n) {
            float sum_m_n = 0;

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
template <typename T>
T* vcToNew(const std::vector<std::vector<T>>& vA)
{

    size_t M = vA.size();
    size_t K = vA[0].size();
    T* A = new float[M * K];

    for (size_t m = 0; m < M; ++m) {

        for (size_t k = 0; k < K; ++k) {
            A[m * K + k] = vA[m][k];
        }
    }
    return A;
}

template <typename T>
void PrintVectorPtr(const T* A, size_t M, size_t K)
{
    for (size_t m = 0; m < M; ++m) {
        const float* a = A + m * K;
        for (size_t k = 0; k < K; ++k) {
            std::cout << a[k] << ' ';
        }
        std::cout << '\n';
    }
}

void gemm_v0(int M, int N, int K, const float* A, const float* B, float* C)
{
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i * N + j] = 0;
            for (int k = 0; k < K; ++k)
                C[i * N + j] += A[i * K + k] * B[k * N + j];
        }
    }
}

void gemm_v1(int M, int N, int K, const float* A, const float* B, float* C)
{
    for (int i = 0; i < M; ++i) {
        float* c = C + i * N;
        for (int j = 0; j < N; ++j)
            c[j] = 0;
        for (int k = 0; k < K; ++k) {
            const float* b = B + k * N;
            float a = A[i * K + k];
            for (int j = 0; j < N; ++j)
                c[j] += a * b[j];
        }
    }
}
void gemm_v2(int M, int N, int K, const float* A, const float* B, float* C)
{
    for (int i = 0; i < M; ++i) {
        float* c = C + i * N;
        for (int j = 0; j < N; j += 8)
            _mm256_storeu_ps(c + j + 0, _mm256_setzero_ps());
        for (int k = 0; k < K; ++k) {
            const float* b = B + k * N;
            __m256 a = _mm256_set1_ps(A[i * K + k]);
            for (int j = 0; j < N; j += 16) {
                _mm256_storeu_ps(c + j + 0, _mm256_fmadd_ps(a, _mm256_loadu_ps(b + j + 0), _mm256_loadu_ps(c + j + 0)));
                _mm256_storeu_ps(c + j + 8, _mm256_fmadd_ps(a, _mm256_loadu_ps(b + j + 8), _mm256_loadu_ps(c + j + 8)));
            }
        }
    }
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
    std::vector<std::vector<float>> vA(M, std::vector<float>(K, 1));
    std::vector<std::vector<float>> vB(K, std::vector<float>(N, 2));

    CreateVectors(vA);
    CreateVectors(vB);

    float* ptrA = vcToNew(vA);
    float* ptrB = vcToNew(vB);

    //    std::cout << "\nFirst matrix vector\n";
    //    PrintVector(vA);
    //    std::cout << "\nSecond matrix vector\n";
    //    PrintVector(vB);

    //    std::cout << "\nFirst matrix pointer\n";
    //    PrintVectorPtr(ptrA, M, K);
    //    std::cout << "\nSecond matrix Pointer \n";
    //    PrintVectorPtr(ptrB, K, N);

    {

        Timer timer;
        std::vector<std::vector<float>> vC = vMulti(vA, vB);

        std::cerr << '\n'
                  << " vMulti Vector TIME "
                  << timer.elapsed() << " milliseconds" << std::endl;
        //        std::cout << "\nResult matrix\n";
        //        PrintVector(vC);
    }

    float* ptrC = new float[M * N];

    {

        Timer timer;
        gemm_v0(M, N, K, ptrA, ptrB, ptrC);
        std::cerr
            << '\n'
            << " vMulti pointer0 TIME "
            << timer.elapsed() << " milliseconds" << std::endl;
        //        std::cout << "\nResult matrix\n";
        //        PrintVectorPtr(ptrC, M, N);
    }

    {

        Timer timer;
        gemm_v1(M, N, K, ptrA, ptrB, ptrC);
        std::cerr
            << '\n'
            << " vMulti pointer1 TIME "
            << timer.elapsed() << " milliseconds" << std::endl;
        //        std::cout << "\nResult matrix\n";
        //        PrintVectorPtr(ptrC, M, N);
    }

    {

        Timer timer;
        gemm_v2(M, N, K, ptrA, ptrB, ptrC);
        std::cerr
            << '\n'
            << " vMulti pointer2 TIME "
            << timer.elapsed() << " milliseconds" << std::endl;
        //        std::cout << "\nResult matrix\n";
        //        PrintVectorPtr(ptrC, M, N);
    }

    std::cerr << '\n'
              << " MAIN TIME "
              << t.elapsed() << " milliseconds" << std::endl;

    delete[] ptrA;
    delete[] ptrB;
    delete[] ptrC;
    //    std::cout << std::setw(5) << getRandomNumber(1, 99) << " \n"; // из заданого диапазона
    return EXIT_SUCCESS;
}
