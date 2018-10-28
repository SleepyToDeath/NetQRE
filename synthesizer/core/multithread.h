#include "redundancy.h"
#include "incomplete_execution.h"
#include <mutex>
#include <queue>
#include <thread>

class WorkerThread;

enum TaskType {
	FILTER, ACCEPT, KILL
};

class MeansOfProduction {
	public:
	std::shared_ptr<RedundancyPlan> rp;
};

class Mailbox {
	public:
	TaskType type;
	bool pending_task;
	shared_ptr<IESyntaxTree> candidate;
	shared_ptr<IEExample> examples;

	bool finished_task; /* only used in snapshot returned by find_finished_task() */
	bool accept;
	shared_ptr<IESyntaxTree> simplified;
};

class MasterThread : public std::enable_shared_from_this<MasterThread> {
	public:

	MasterThread(int population, MeansOfProduction m);
	~MasterThread();
	void hire_workers();

	void do_filter(shared_ptr<IESyntaxTree> candidate, shared_ptr<IEExample> examples);
	void do_accept(shared_ptr<IESyntaxTree> candidate, shared_ptr<IEExample> examples);

	/*	
		If a task is finished, return snapshot of the worker's mailbox 
		Otherwise return nullptr 
	*/
	Mailbox find_finished_task();
	bool all_tasks_done();

	/* runtime */
	/*	
		`done_queue` can be updated by any worker or the master,
		so it requires a lock.
		`idle_queue` is only updated by the master,
		so it requires no lock.
	*/
	std::mutex done_lock;
	std::queue<shared_ptr<WorkerThread> > done_queue;
	std::queue<shared_ptr<WorkerThread> > idle_queue;

	/* configuration */
	int population;
	MeansOfProduction m;

	/* placeholder */
	std::vector<std::thread> thread_pool;
	std::vector<shared_ptr<WorkerThread> > worker_pool;
};

class WorkerThread : public std::enable_shared_from_this<WorkerThread> {
	public:

	WorkerThread(shared_ptr<MasterThread> master, MeansOfProduction m);

	void working_loop();

	Mailbox msg;
	MeansOfProduction m;
	shared_ptr<MasterThread> master;
};


