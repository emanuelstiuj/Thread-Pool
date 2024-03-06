# Thread Pool

Implemented a generic thread pool, then use it to traverse a graph and compute the sum of the elements contained by the nodes.

The threads are created when the thread pool is created.
Each thread continuously polls the task queue for available tasks.
Once tasks are put in the task queue, the threads poll tasks, and start running them.
A thread pool creates **N** threads upon its creation and does not destroy (join) them throughout its lifetime.
That way, the penalty of creating and destroying threads ad-hoc is avoided.
For synchronization I used mutexes, semaphores, spinlocks, condition variables.

