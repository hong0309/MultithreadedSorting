#pragma once

#include "SortAlgorithm.h"

class MergeSort : public SortAlgorithm
{
public:
    void Sort(std::vector<int>& data) override;
    std::string_view GetName() const override;

private:
    void MergeSortRecursive(std::vector<int>& data, int left, int right);
    void Merge(std::vector<int>& data, int left, int mid, int right);
};