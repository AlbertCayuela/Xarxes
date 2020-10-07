#include "ModuleTaskManager.h"

using namespace std;

void ModuleTaskManager::threadMain()
{
	while (true)
	{
		// TODO 3:
	// - Wait for new tasks to arrive
	// - Retrieve a task from scheduledTasks
	// - Execute it
	// - Insert it into finishedTasks

		Task* task = nullptr;

			unique_lock<mutex> lock(mtx); //Lock the mutex
			while (scheduledTasks.empty())
			{
				event.wait(lock);

				if (exitFlag == true) 
				{
					return;
				}
			}

			task = scheduledTasks.front();
			scheduledTasks.pop();
			task->execute();
			finishedTasks.push(task);

	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())
	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i] = thread(&ModuleTaskManager::threadMain, this);
	}

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)
	while (!finishedTasks.empty())
	{
		Task* task = finishedTasks.front();
		task->owner->onTaskFinished(task);
		finishedTasks.pop();
	}

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	exitFlag = true;
	event.notify_all();
	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i].join();
	}


	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	unique_lock<mutex> lock(mtx); //Lock the mutex
	scheduledTasks.push(task);
	event.notify_one();
}
