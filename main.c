#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <pthread.h>

#define p_Num 4

int keep_run = 1;

typedef struct
{
	char name[20];
	int num;
} RaceArg;

static void *func_action(void *args)
{
	RaceArg *r = (RaceArg *)args;
	cpu_set_t get;

	while (keep_run)
	{
		sleep(1);

		printf("\n ---------------- \n");

		CPU_ZERO(&get);
		if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
		{
			fprintf(stderr, "get thread affinity failed\n");
		}

		for (int j = 0; j < p_Num; j++)
		{
			if (CPU_ISSET(j, &get))
			{
				// get 每次都会被清空，遍历查询此cpu core 是否绑定该线程
				printf("index: %d thread %ld is running in processor %d\n", r->num, (int)pthread_self(), j);
			}
		}
	}
	return NULL;
}

static void exit_application(int signal)
{
	signal = signal;

	printf("\n 😎 exit signal: %d\n", signal);

	keep_run = 0;
}

int main()
{
	/* Global shutdown handler */
	signal(SIGINT, exit_application);

	cpu_set_t mask;

	pthread_t threadPool[p_Num] = {0};

	RaceArg p_arg[p_Num] = {0};

	for (int i = 0; i < p_Num; i++)
	{
		printf("p_arg[i].name: %s ---\n", p_arg[i].name);

		p_arg[i].num = i;
	}

	for (int i = 0; i < p_Num; ++i)
	{
		if (-1 == pthread_create(&(threadPool[i]), NULL, func_action, (void *)(&p_arg[i])))
		{
			printf("error join thread. %d", i);
			abort();
		}

		CPU_ZERO(&mask);
		CPU_SET(i, &mask);

		if (pthread_setaffinity_np(threadPool[i], sizeof(mask), &mask) < 0)
		{
			fprintf(stderr, "set thread affinity failed\n");
		}
	}

	for (int i = 0; i < p_Num; ++i)
	{
		if (pthread_join(threadPool[i], NULL))
		{
			printf("error join thread.");
			abort();
		}
	}

	printf("\n 🍁 -----end\n");

	return 0;
}
