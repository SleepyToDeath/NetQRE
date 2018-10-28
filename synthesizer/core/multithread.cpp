#include "multithread.h"

using std::string;

MasterThread::MasterThread(int population, MeansOfProduction m) {
	this->population = population;
	this->m = m;
}

void MasterThread::hire_workers() {
	for (int i=0; i<population; i++)
	{
		auto new_worker = shared_ptr<WorkerThread>(new WorkerThread(shared_from_this(), m));
		idle_queue.push(new_worker);
		worker_pool.push_back(new_worker);
		thread_pool.push_back(std::thread(&WorkerThread::working_loop, std::ref((*new_worker))) );
	}
}

void MasterThread::do_filter(shared_ptr<IESyntaxTree> candidate, shared_ptr<IEExample> examples) {
	auto worker = idle_queue.front();
	worker->msg.type = FILTER;
	worker->msg.candidate = candidate;
	worker->msg.examples = examples;
	idle_queue.pop();
	worker->msg.pending_task = true;
}

void MasterThread::do_accept(shared_ptr<IESyntaxTree> candidate, shared_ptr<IEExample> examples) {
	auto worker = idle_queue.front();
	worker->msg.type = ACCEPT;
	worker->msg.candidate = candidate;
	worker->msg.examples = examples;
	idle_queue.pop();
	worker->msg.pending_task = true;
}

Mailbox MasterThread::find_finished_task() {
	done_lock.lock();

	Mailbox snapshot;
	if (!done_queue.empty())
	{
		snapshot = done_queue.front()->msg;
		idle_queue.push(done_queue.front());
		done_queue.pop();
		snapshot.finished_task = true;
	}
	else
	{
		snapshot.finished_task = false;
	}

	done_lock.unlock();

	return snapshot;
}

bool MasterThread::all_tasks_done() {
	return idle_queue.size() == population;
}

MasterThread::~MasterThread() {
	if (!all_tasks_done())
		throw string("[ERROR] There's unfinished task when exiting!\n");
	for (int i=0; i<population; i++)
	{
		idle_queue.front()->msg.type = KILL;
		idle_queue.front()->msg.pending_task = true;
		idle_queue.pop();
	}
	for (int i=0; i<thread_pool.size(); i++)
		thread_pool[i].join();
}

WorkerThread::WorkerThread(shared_ptr<MasterThread> master, MeansOfProduction m) {
	this->master = master;
	this->m = m;
}

void WorkerThread::working_loop() {
	while(true)
	{
		if (msg.pending_task)
		{
			msg.pending_task = false;
			switch(msg.type)
			{
				case FILTER:
				msg.simplified = m.rp->filter(msg.candidate, msg.examples);
				break;

				case ACCEPT:
				msg.accept = msg.candidate->to_program()->accept(msg.examples);
				break;

				case KILL:
				return;

				default:
				break;
			}
			master->done_lock.lock();
			master->done_queue.push(shared_from_this());
			master->done_lock.unlock();
		}
	}
}
