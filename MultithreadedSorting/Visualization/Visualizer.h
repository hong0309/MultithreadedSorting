#pragma once

#include <mutex>
#include <string>
#include <vector>

class Visualizer
{
public:
    void Update(const std::vector<int>& data);
    std::vector<int> GetData();

private:
    std::vector<int> data;
    std::mutex dataMutex;
};
