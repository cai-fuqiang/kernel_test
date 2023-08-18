#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shm.h"
char *p_shm = NULL;
int init_shm(int create) {
    int shmid;
    key_t key;

    key = ftok("/home/wang", 'R');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    p_shm = shmat(shmid, NULL, 0);
    if (p_shm == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    return 0;
}

char *get_shm()
{
	return p_shm;
}
