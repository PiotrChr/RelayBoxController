#include "TaskManager.hpp"

TaskManager::TaskManager()
{
}

void TaskManager::addTask(const char *name, unsigned long dueDate, std::function<void()> job, std::function<bool()> runCondition)
{
    struct Task task = {name, dueDate, false, job, runCondition};
    taskBuffer.push(task);
}

void TaskManager::performTasks()
{
    for (int i = 0; i < taskBuffer.size(); i++)
    {
        if (!taskBuffer[i].finished && taskBuffer[i].dueDate < millis() && taskBuffer[i].runCondition() == true)
        {
            struct Task task = taskBuffer.shift();
            task.job();
            task.finished = true;
        }
        else
        {
            return;
        }
    }
}

size_t TaskManager::taskCount() const
{
    return taskBuffer.size();
}