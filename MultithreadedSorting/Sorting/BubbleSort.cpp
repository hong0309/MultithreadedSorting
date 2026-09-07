#include "BubbleSort.h"

#include <utility>

void BubbleSort::Sort(std::vector<int>& data)
{
    const size_t n = data.size();

    if (n < 2)
    {
        return;
    }

    for (size_t i = 0; i < n - 1; ++i)
    {
        if (ShouldStop()) return;

        bool swapped = false;

        for (size_t j = 0; j < n - 1 - i; ++j)
        {
            if (ShouldStop()) return;

            if (data[j] > data[j + 1])
            {
                std::swap(data[j], data[j + 1]);
                swapped = true;

                NotifyStep(data);
            }
        }

        if (!swapped)
        {
            break;
        }
    }
}

std::string_view BubbleSort::GetName() const
{
    return "Bubble Sort";
}