#include "MergeSort.h"

void MergeSort::Sort(std::vector<int>& data)
{
    const size_t n = data.size();
    if (n < 2)
    {
        return;
    }

    MergeSortRecursive(data, 0, static_cast<int>(n) - 1);
}

void MergeSort::MergeSortRecursive(std::vector<int>& data, int left, int right)
{
    if (ShouldStop()) return;

    if (left >= right)
    {
        return;
    }

    const int mid = left + (right - left) / 2;

    MergeSortRecursive(data, left, mid);
    if (ShouldStop()) return;
    MergeSortRecursive(data, mid + 1, right);
    if (ShouldStop()) return;
    Merge(data, left, mid, right);
}

void MergeSort::Merge(std::vector<int>& data, int left, int mid, int right)
{
    std::vector<int> leftPart(data.begin() + left, data.begin() + mid + 1);
    std::vector<int> rightPart(data.begin() + mid + 1, data.begin() + right + 1);

    size_t i = 0;
    size_t j = 0;
    int k = left;

    while (i < leftPart.size() && j < rightPart.size())
    {
        if (ShouldStop()) return;

        if (leftPart[i] <= rightPart[j])
        {
            data[k] = leftPart[i];
            NotifyStep(data);
            ++i;
        }
        else
        {
            data[k] = rightPart[j];
            NotifyStep(data);
            ++j;
        }
        ++k;
    }

    while (i < leftPart.size())
    {
        if (ShouldStop()) return;

        data[k] = leftPart[i];
        NotifyStep(data);
        ++i;
        ++k;
    }

    while (j < rightPart.size())
    {
        if (ShouldStop()) return;

        data[k] = rightPart[j];
        NotifyStep(data);
        ++j;
        ++k;
    }
}

std::string_view MergeSort::GetName() const
{
    return "Merge Sort";
}