#include "DataGenerator.h"

#include <random>

std::vector<int> DataGenerator::GenerateRandomData(std::size_t count, int minValue, int maxValue)
{
    std::vector<int> data;
    data.reserve(count);

    std::random_device randomDevice;
    std::mt19937 engine(randomDevice());
    std::uniform_int_distribution<int> distribution(minValue, maxValue);

    for (std::size_t i = 0; i < count; ++i)
    {
        data.push_back(distribution(engine));
    }

    return data;
}