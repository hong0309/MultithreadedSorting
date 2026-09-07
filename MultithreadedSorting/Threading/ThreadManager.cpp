#include "ThreadManager.h"

#include <iostream>
#include <thread>
#include <chrono>

std::vector<SortResult> ThreadManager::Run(const std::vector<std::unique_ptr<SortAlgorithm>>& algorithms, const std::vector<int>& original)
{
    std::vector<std::thread> threads;
    std::vector<SortResult> results;

    for (const auto& algorithm : algorithms)
    {
        threads.emplace_back([&algorithm, &original,&results, this]()
            {
                std::vector<int> data = original;
                auto start = std::chrono::high_resolution_clock::now();
                algorithm->Sort(data);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

                SortResult result;
                result.name = std::string(algorithm->GetName());
                result.elapsedMicroseconds = duration.count();
                result.sortedData = data;

                {
                    std::lock_guard<std::mutex> lock(resultMutex);
                    results.push_back(result);
                }

                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << result.name << " : " << result.elapsedMicroseconds << " us\n";

                    /*
                    for (int value : result.sortedData)
                    {
                        std::cout << value << ' ';
                    }
                    */

                    std::cout << '\n';
                }
            });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }
	return results;
}