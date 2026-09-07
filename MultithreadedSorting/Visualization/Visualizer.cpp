#include "Visualizer.h"

void Visualizer::Update(
    const std::vector<int>& newData)
{
    std::lock_guard<std::mutex> lock(dataMutex);

    data = newData;
}

std::vector<int> Visualizer::GetData()
{
    std::lock_guard<std::mutex> lock(dataMutex);

    return data;
}
