#pragma once

#include "SortAlgorithm.h"

class SelectionSort : public SortAlgorithm
{
public:
    void Sort(std::vector<int>& data) override;
    std::string_view GetName() const override;
};