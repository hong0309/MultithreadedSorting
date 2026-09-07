#pragma once

#include "SortAlgorithm.h"

class QuickSort : public SortAlgorithm
{
public:
    void Sort(std::vector<int>& data) override;
    std::string_view GetName() const override;

private:
	void QuickSortRecursive(std::vector<int>& data, int left, int right);
	int Partition(std::vector<int>& data, int left, int right);
};