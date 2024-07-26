module;
module Iconer.ThreadPool;
import Iconer.Framework;
import <print>;

thread_local constinit size_t workerId{};

void
ServerThreadPool::OnStartWorker(size_t index)
{
	std::println("Worker {} is generated.", index);

	workerId = index;
}

void
ServerThreadPool::OnTerminateWorker() const
{
	std::println("Worker {} is terminated.", workerId);
}