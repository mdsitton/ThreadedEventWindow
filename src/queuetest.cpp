#include <atomic>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <thread>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "queue.hpp"
#include "timer.hpp"

struct QueueData
{
	double eventTime;
};

FpsTimer timer;

double smallestTime;
const int maxIters = 10000000;
QueueSPSC<QueueData, 128> queue;
std::atomic<bool> running;
struct QueueInfo
{
	int loc;
	double lat;
};

QueueInfo infoData[maxIters];

void second_thread(double referenceTime)
{
	QueueData data {referenceTime};
	double startTime = timer.get_time();

	int count = 0;
	while (running)
	{
		while (queue.pop(data))
		{
			infoData[count] = {queue.queue_length(), (timer.get_time() - data.eventTime) * 1000};
			count++;
		}
	}

	std::cout << "Thread 2 END OF DATA time: " << (timer.get_time() - startTime) << " s" << std::endl;

	for (auto &info : infoData)
	{
		if (info.lat > (smallestTime*16))
		{
			std::cout << "queue loc " << info.loc << " Queue latency: " << info.lat << std::endl;
		}
	}
}

int main()
{
	std::cout << "LockFree?: " << queue.is_lock_free() << std::endl;

	std::cout << " Time accuracy " << timer.get_smallest_precision() << std::endl;
	std::cout << " Timer frequency " << 1000 * timer.get_smallest_precision() << " kHz" << std::endl;

    double startTime = 0;
	running = true;

	std::thread secondaryThread(second_thread, startTime);
	QueueData data {startTime};
	startTime = timer.get_time();

	for (int i = 0; i < maxIters; i++)
	{
		data.eventTime = timer.get_time();
		queue.push(data);
	}
	running = false;
	std::cout << "Thread 1 END OF DATA time: " << (timer.get_time() - startTime) << " s" << std::endl;
	secondaryThread.join();
	return 0;
}