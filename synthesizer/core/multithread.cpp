#include "multithread.h"

using std::string;

MasterThread::MasterThread(int population, MeansOfProduction m) {
	this->population = population;
	this->m = m;
	buzy_workers = 0;
}

void MasterThread::hire_workers() {
	for (int i=0; i<population; i++)
	{
		auto new_worker = shared_ptr<WorkerThread>(new WorkerThread(shared_from_this(), m));
		worker_pool.push_back(new_worker);
		thread_pool.push_back(std::thread(&WorkerThread::working_loop, std::ref((*new_worker))) );
	}
}

void MasterThread::do_task(Mailbox msg) {
	task_lock.lock();
	pending_tasks.push(msg);
	task_lock.unlock();
}

void MasterThread::do_filter(shared_ptr<IESyntaxTree> candidate, shared_ptr<IEExample> examples) {
	Mailbox msg;
	msg.type = FILTER;
	msg.candidate = candidate;
	msg.examples = examples;
	do_task(msg);
}

void MasterThread::do_accept(shared_ptr<IESyntaxTree> candidate, shared_ptr<IEExample> examples) {
	Mailbox msg;
	msg.type = FILTER;
	msg.candidate = candidate;
	msg.examples = examples;
	do_task(msg);
}

Mailbox MasterThread::find_finished_task() {
	task_lock.lock();

	Mailbox snapshot;
	if (!finished_tasks.empty())
	{
		snapshot = finished_tasks.front();
		finished_tasks.pop();
		snapshot.finished_task = true;
	}
	else
	{
		snapshot.finished_task = false;
	}

	task_lock.unlock();

	return snapshot;
}

bool MasterThread::all_tasks_done() {
	return buzy_workers == 0;
}

MasterThread::~MasterThread() {

	task_lock.lock();
	if (!all_tasks_done())
		throw string("[ERROR] There's unfinished task when exiting!\n");
	for (int i=0; i<population; i++)
	{
		Mailbox fire;
		fire.type = KILL;
		pending_tasks.push(fire);
	}
	task_lock.unlock();

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
		master->task_lock.lock();
		if (!master->pending_tasks.empty())
		{
			msg = master->pending_tasks.front();
			master->pending_tasks.pop();
			master->buzy_workers++;
			master->task_lock.unlock();
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
			master->task_lock.lock();
			master->finished_tasks.push(msg);
			master->buzy_workers--;
		}
		master->task_lock.unlock();
	}
}
