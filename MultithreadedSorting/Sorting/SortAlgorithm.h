#pragma once

#include <atomic>
#include <functional>
#include <string_view>
#include <vector>

class SortAlgorithm
{
public:
    using StepCallback =
        std::function<void(const std::vector<int>&)>;

    virtual ~SortAlgorithm() = default;

    virtual void Sort(std::vector<int>& data) = 0;
    virtual std::string_view GetName() const = 0;

    void SetStepCallback(StepCallback callback)
    {
        stepCallback = callback;
    }

    void RequestStop()
    {
        stopRequested = true;
    }

    void ResetStop()
    {
        stopRequested = false;
    }

protected:
    bool ShouldStop() const
    {
        return stopRequested;
    }

    void NotifyStep(const std::vector<int>& data)
    {
        if (stepCallback)
        {
            stepCallback(data);
        }
    }

private:
    StepCallback stepCallback;

    std::atomic<bool> stopRequested = false;
};