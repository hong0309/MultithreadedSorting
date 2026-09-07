#pragma once

#include <cstddef>
#include <vector>

class DataGenerator
{
public:
    std::vector<int> GenerateRandomData(
        std::size_t count,
        int minValue,
        int maxValue
    );
};