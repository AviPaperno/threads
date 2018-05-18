#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define THREADS_NUMBER 4

//Mutex for the console
pthread_mutex_t consoleMut; 
//Mutexes for threads
pthread_mutex_t mut[THREADS_NUMBER];

int start[] = { 1,1,1,1 };
int del[] = { 0,0,0,0 };
int checkEnd[] = { 1,1,1,1 };
pthread_t threads[THREADS_NUMBER];

struct threadInfo 
{
	int id;
	int priority;
};

struct threadInfo statuses[THREADS_NUMBER];

pthread_cond_t lock[THREADS_NUMBER];

void *threadFunc(void *arg) {
	struct threadInfo *info = (struct threadInfo *)arg;
	while (1)
	{
		if (start[info->id] == 1)
		{
			sleep(2);
			start[info->id]--;
			
			pthread_mutex_lock(&consoleMut);
			printf("Thread %d with priotity %d stoped\n", (info->id + 1), info->priority);
			pthread_mutex_unlock(&consoleMut);
			
			//Get mutex
			pthread_mutex_lock(&mut[info->id]);
			//Free mutex and sleep on condition variable
			pthread_cond_wait(&lock[info->id], &mut[info->id]);
			//Free mutex after awakening get
			pthread_mutex_unlock(&mut[info->id]);
		}
		else
		{
			checkEnd[info->id]--;
			del[info->id]++;
			pthread_exit(0);
		}
	}
}

int main()
{
	srand((unsigned int)time(0));
	int check;
	
	//Initalization of mutex for the console
	pthread_mutex_init(&consoleMut,NULL);
	
	//Initialization of mutexses for threads
	for (int i = 0; i < THREADS_NUMBER; i++)
		pthread_mutex_init(&mut[i],NULL);
	
	//Initialization of condition variables
	for (int i = 0; i < THREADS_NUMBER; i++)
		pthread_cond_init(&lock[i],NULL);
	
	for (int i = 0; i < THREADS_NUMBER; i++)
	{
		//Set information about thread
		statuses[i].id = i;
		statuses[i].priority = rand() % 50 + 10;
		
		//Creating the thread
		int result = pthread_create(&threads[i], NULL, threadFunc, (void *)&statuses[i]);
		
		//Checking result of creating
		if (result != 0)
		{
			printf("Thread can't created. error: %i",result);
			exit(0);
		}
		//Set priority of thread
		pthread_setschedprio(threads[i],statuses[i].priority);
		printf("Thread %d with priority %d started\n", i+1,statuses[i].priority);
	}

	while ((del[0] == 0) || (del[1] == 0) || (del[2] == 0) || (del[3] == 0))
	{
		if ((start[0] == 0) && (start[1] == 0) && (start[2] == 0) && (start[3] == 0))
		{
			int flag;
			printf("%s \n","Restart(1) or destroy(0) ?");
			do
			{
				flushall();
				flag = scanf("%d",&check);
				if (flag != 1)
					printf("%s \n","Error!");
			} while (flag != 1);

			if (check == 1)
			{
				for (int i = 0; i < THREADS_NUMBER; i++)
				{
					start[i]++;
					pthread_cond_signal(&lock[i]);
					printf("Thread %d with priority %d started\n", i+1,statuses[i].priority);
				}
			}
			else
			{
				for (int i = 0; i < THREADS_NUMBER; i++)
					pthread_cond_signal(&lock[i]);
				int checkFin = THREADS_NUMBER;
				while (checkFin > 0)
				{
					for (int i = 0; i < THREADS_NUMBER; i++)
					{
						if (checkEnd[i] == 0)
						{
							printf("%s %i %s \n","Thread",(i + 1),"destroyed");
							checkEnd[i]++;
							checkFin--;
						}
					}
				}
			}
		}
	}
	for (int i = 0; i < THREADS_NUMBER; i++)
		pthread_detach(threads[i]);

	return 0;
}
