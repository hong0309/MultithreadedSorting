#include "SelectionSort.h"

#include <utility>

void SelectionSort::Sort(std::vector<int>& data)
{
	const size_t n = data.size();
	if (n < 2)
	{
		return;
	}
	for (size_t i = 0; i < n - 1; ++i)
	{
		if (ShouldStop()) return;

		size_t minIndex = i;
		for (size_t j = i + 1; j < n; ++j)
		{
			if (ShouldStop()) return;

			if (data[j] < data[minIndex])
			{
				minIndex = j;
			}
		}
		if (minIndex != i)
		{
			std::swap(data[i], data[minIndex]);
			NotifyStep(data);
		}
	}
}

std::string_view SelectionSort::GetName() const
{
	return "Selection Sort";
}