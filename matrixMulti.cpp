


#include <iostream>
#include <iostream>
#include <vector>

void CreateVectors(std::vector< std::vector<int>>& vA, const size_t M = 2 , const size_t K = 3)
{
	
	

	size_t i = 1;

	for ( auto& e : vA)
	{
		for (auto& x : e)
			x = i++;
	};
	
	
}

template <typename T>
void PrintVector(const std::vector< std::vector<T>>& vA)
{
	for (const auto e : vA)
	{
		for (const auto x : e)
		{
			std::cout << x << ' ';
		}
		std::cout << '\n';
	};
}
std::vector< std::vector<int64_t>> vMulti(const std::vector< std::vector<int>>& vA, const std::vector< std::vector<int>>& vB)
{
	if (vA[vA.size() - 1].size() != vB.size())
	{
		std::vector< std::vector<int64_t>> error (0) ;
		return error;
	}
	const size_t M = vA.size();
	const size_t N = vB[vB.size() - 1].size();
	const size_t K = vB.size();
	std::vector< std::vector<int64_t>> vMulti(M, std::vector<int64_t>(N));
	
	for (size_t m = 0; m < M; ++m)
	{
		for (size_t n = 0; n < N; ++n)
		{
			int64_t sum_m_n = 0;
			
			for (size_t k = 0; k < K; ++k)
			{
				sum_m_n += vA[m][k] + vB[k][n];
				
			}

			vMulti[m][n] = sum_m_n;
			
		}
			
	}


	return vMulti;
}


int main()
{

	const size_t M = 2;
	const size_t N = 2;
	const size_t K = 3;
	std::vector< std::vector<int>> vA (M, std::vector<int>(K, 1));
	std::vector< std::vector<int>> vB (K, std::vector<int>(N, 2));

	/*CreateVectors(vA, M, K);
	CreateVectors(vB, K, N);*/

	std::cout << "\nFirst matrix\n";
	PrintVector(vA);
	std::cout << "\nSecond matrix\n";
	PrintVector(vB);

	
	std::vector< std::vector<int64_t>> vC =
		vMulti(vA, vB);
	std::cout << "\nResult matrix\n";
	PrintVector(vC);
	

	
	return EXIT_SUCCESS;
}
