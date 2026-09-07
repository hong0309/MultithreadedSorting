#include "QuickSort.h"

#include <utility>

void QuickSort::Sort(std::vector<int>& data)
{
	const size_t n = data.size();
	if (n < 2)
	{
		return;
	}
	QuickSortRecursive(data, 0, n - 1);
}

void QuickSort::QuickSortRecursive(std::vector<int>& data, int left, int right)
{
    if (ShouldStop()) return;

    if (left < right)
    {
        int pivotIndex = Partition(data, left, right);
        if (ShouldStop()) return;
        QuickSortRecursive(data, left, pivotIndex);
        QuickSortRecursive(data, pivotIndex + 1, right);
    }
}

int QuickSort::Partition(std::vector<int>& data, int left, int right)
{
    const int pivot = data[left + (right - left) / 2];
    int i = left;
    int j = right;

    while (true)
    {
        if (ShouldStop()) return j;

        while (data[i] < pivot) {
            if (ShouldStop()) return j;
            ++i; }
        while (data[j] > pivot) { 
            if (ShouldStop()) return j;
            --j; }

        if (i >= j)
        {
            return j;
        }
        std::swap(data[i], data[j]);
        NotifyStep(data);

        ++i;
        --j;
    }
}

std::string_view QuickSort::GetName() const
{
    return "Quick Sort";
}