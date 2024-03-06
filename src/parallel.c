// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;

static void process_node(unsigned int idx);

/* TODO: Define graph synchronization mechanisms. */
pthread_mutex_t mutex_sum;
pthread_mutex_t mutex_visited;

/* TODO: Define graph task argument. */
typedef struct task_args {
	os_node_t *curr_node;
} task_args;

void destroy_args(void *task_args)
{
	free(task_args);
}

void task_func(void *t_args)
{
	task_args *args = (task_args *) t_args;

	pthread_mutex_lock(&mutex_sum);
	sum += args->curr_node->info;
	pthread_mutex_unlock(&mutex_sum);

	for (unsigned int i = 0; i < args->curr_node->num_neighbours; i++) {
		pthread_mutex_lock(&mutex_visited);
		if (graph->visited[args->curr_node->neighbours[i]] == NOT_VISITED) {
			graph->visited[args->curr_node->neighbours[i]] = PROCESSING;
			pthread_mutex_unlock(&mutex_visited);

			task_args *new_task_args;
			os_task_t *new_task;

			new_task_args = (task_args *) malloc(sizeof(task_args));
			new_task_args->curr_node = graph->nodes[args->curr_node->neighbours[i]];
			new_task = create_task(task_func, new_task_args, destroy_args);
			enqueue_task(tp, new_task);
		} else {
			pthread_mutex_unlock(&mutex_visited);
		}
	}
}

static void process_node(unsigned int idx)
{
	task_args *args;
	os_task_t *task;

	graph->visited[idx] = PROCESSING;
	args = (task_args *) malloc(sizeof(task_args));
	args->curr_node = graph->nodes[idx];
	task = create_task(task_func, args, destroy_args);
	enqueue_task(tp, task);
	for (int i = 0; i < 4; i++)
		sem_post(&(tp->sem_dequeue));
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);

	/* TODO: Initialize graph synchronization mechanisms. */
	pthread_mutex_init(&mutex_sum, NULL);
	pthread_mutex_init(&mutex_visited, NULL);

	tp = create_threadpool(NUM_THREADS);
	process_node(0);
	wait_for_completion(tp);
	destroy_threadpool(tp);

	pthread_mutex_destroy(&mutex_sum);
	pthread_mutex_destroy(&mutex_visited);

	printf("%d", sum);

	return 0;
}
