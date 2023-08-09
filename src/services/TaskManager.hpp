#pragma once

#include <Arduino.h>
#include <functional>
#include "CircularBuffer.h"

struct Task
{
    const char *name;
    unsigned long dueDate;
    bool finished;
    std::function<void()> job;
    std::function<bool()> runCondition;
};

class TaskManager
{
public:
    static constexpr size_t taskLimit = 20;
    TaskManager();

    void addTask(const char *name, unsigned long dueDate, std::function<void()> job, std::function<bool()> runCondition);
    void performTasks();
    size_t taskCount() const;

private:
    CircularBuffer<Task, taskLimit> taskBuffer;
};
