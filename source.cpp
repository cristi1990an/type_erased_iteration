#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <set>

#include "any_subrange.hpp"

int main()
{
	std::vector vector{ 1,2,3 };
	std::set set{ 4,5,6 };
	auto view = std::views::iota(0, 10)
		| std::views::filter([](int val)
			{
				return val > 6;
			})
		| std::views::common;


	any_subrange<int> subrange; // instantiated based ONLY on the iterator reference type

	static_assert(std::ranges::input_range<any_subrange<int>>);

	subrange = vector;
	for (int val : subrange)
	{
		std::cout << val << ' ';
	}
	// 1 2 3 

	subrange = set;
	for (int val : subrange)
	{
		std::cout << val << ' ';
	}
	// 4 5 6

	subrange = view;
	for (int val : subrange)
	{
		std::cout << val << ' ';
	}
	// 7 8 9

	std::cout << '\n';

	any_subrange<int&> mutable_subrange{ vector };

	for (int& val : mutable_subrange)
	{
		++val;
	}

	subrange = vector;
	for (int val : subrange)
	{
		std::cout << val << ' ';
	}
	// 2 3 4

	static_assert(std::ranges::input_range<any_subrange<int&>>);
	static_assert(std::ranges::output_range<any_subrange<int&>, int>);
}

