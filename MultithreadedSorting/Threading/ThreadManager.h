#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <string>

#include "../Sorting/SortAlgorithm.h"

struct SortResult
{
    std::string name;
    long long elapsedMicroseconds;
    std::vector<int> sortedData;
};

class ThreadManager
{
public:
    std::vector<SortResult> Run(
        const std::vector<std::unique_ptr<SortAlgorithm>>& algorithms,
        const std::vector<int>& original
    );

private:
	std::mutex coutMutex;
    std::mutex resultMutex;
};