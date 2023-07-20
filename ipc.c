#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int *shmaddr;
int shmid, semid;

void handler_1(int sig)
{
}

void handler_2(int sig)
{
	if (sig == SIGINT)
	{
		//deallocating!
		shmdt(shmaddr);
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid, 0, IPC_RMID, 0);
		_exit(0);
	}
}

int main(int argc, char **argv)
{
	char buf1[] = "Bad amount of parametrs\n";
	char buf2[] = "Not possible number of bees\n";
	char buf3[] = "Not possible size of pot\n";

	if (argc != 3)
	{
		write(2, buf1, sizeof buf1 - 1);
		exit(1);
	}
	int N = strtol(argv[1], NULL, 10); // number of bees
	if (N <= 0)
	{
		write(2, buf2, sizeof buf2 - 1);
		exit(2);
	}
	int S = strtol(argv[2], NULL, 10); // size of pot
	if (S <= 0)
	{
		write(2, buf3, sizeof buf3 -1);
		exit(3);
	}
	int pid[N+1]; // sons' pids

	shmid = shmget(IPC_PRIVATE, S*(sizeof (int)), 0666);
	shmaddr = (int*) shmat(shmid, NULL, 0);
	*shmaddr = 0;
	semid = semget(IPC_PRIVATE, 2, 0666);
	semctl(semid, 0, SETVAL, (int) 0);
	struct sembuf sem_bear_up = {0,1,0};
	struct sembuf sem_bear_down = {0,-1,0};
	semctl(semid, 1, SETVAL, (int) 1);
	struct sembuf sem_bee_up = {1,1,0};
	struct sembuf sem_bee_down = {1,-1,0};

	if (pid[N] = fork())
	{
	}
	else
	{
		for(;;)
		{
			semop(semid, &sem_bear_down, 1);
			sleep(2);
			printf("bear wakes up and stars eating\n");
			sleep(S/2);
			*shmaddr = 0;
			printf("%d/", *shmaddr);
			printf("%d", S);
			printf(" of honey in the pot\n");
			sleep(1);
			printf("bear goes to sleep\n");
			semop(semid, &sem_bee_up, 1);
		}
		exit(0);
	}
	for (int i = 0; i < N; i++)
	{
		if ((pid[i] = fork()))
		{
		}
		else
		{
			for(;;)
			{
				sleep(2);
				semop(semid, &sem_bee_down, 1);
				*shmaddr = *shmaddr + 1;
				printf("%d/", *shmaddr);
				printf("%d", S);
				printf(" of honey in the pot\n");
				//sleep(1);

				if (*shmaddr == S)
				{
					semop(semid, &sem_bear_up, 1);
				}
				else
				{
					semop(semid, &sem_bee_up, 1);
				}
			}
			exit(0);
		}
	}
	signal(SIGINT, handler_1);
	pause();
	for (int i = 0; i < (N+1); i++)
	{
		kill(pid[i], SIGINT);
		wait(NULL);
	}
	signal(SIGINT, handler_2);
	pause();

	return 0;
}
