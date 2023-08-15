#include <semaphore.h>
#include <stdio.h>
#include <stddef.h>
#include "packer.h"

// You can declare global variables here
typedef struct
{
    int ball_one_id;
    int *other_ball_ids; // arr to store the other balls
    int count_other_balls;
    sem_t mutex;
    sem_t safe_to_reset;
    sem_t last_ball_arrived;
} sema;

sema semaphores[4];
int N;
void set_other_ids(int *original_other_ids, int *dest_other_ids, int id_to_remove, int id_to_insert);

void packer_init(int balls_per_pack)
{
    // Write initialization code here (called once at the start of the program).
    N = balls_per_pack;
    for (int i = 1; i < 4; i++)
    {
        sem_init(&semaphores[i].safe_to_reset, 0, 0);
        sem_init(&semaphores[i].mutex, 0, 1);

        sem_init(&semaphores[i].last_ball_arrived, 0, 0); // for the last ball
        semaphores[i].ball_one_id = -1;
        semaphores[i].count_other_balls = 0;
        semaphores[i].other_ball_ids = NULL;
    }
}

void packer_destroy(void)
{
    // Write deinitialization code here (called once at the end of the program).
    for (int i = 1; i < 4; i++)
    {
        sem_destroy(&semaphores[i].safe_to_reset);
        sem_destroy(&semaphores[i].mutex);
        sem_destroy(&semaphores[i].last_ball_arrived);
    }
}

void pack_ball(int colour, int id, int *other_ids)
{
    // Write your code here.
    sema *color = &semaphores[colour];

    sem_wait(&color->mutex);

    // case 1: first ball arrives
    if (color->ball_one_id == -1)
    {
        // printf("first ball: %d\n", id);
        color->ball_one_id = id;
        color->count_other_balls = 0;      // reset it if its not the first pack
        color->other_ball_ids = other_ids; // use the mem space of the driver

        sem_post(&color->mutex); // so that ball_one_id is only set once

        sem_wait(&color->last_ball_arrived);

        sem_post(&color->safe_to_reset);
    }

    // case 2: balls 2 to N-1 arrive
    else if (color->count_other_balls < (N - 2))
    {
        // printf("in between ball: %d\n", id);

        color->other_ball_ids[color->count_other_balls] = id;
        color->count_other_balls++;
        sem_post(&color->mutex);

        sem_wait(&color->last_ball_arrived);

        // handle output
        set_other_ids(color->other_ball_ids, other_ids, id, color->ball_one_id);

        sem_post(&color->safe_to_reset);
    }

    // case 3: last ball arrives
    else
    {
        // printf("last ball: %d\n", id);

        color->other_ball_ids[color->count_other_balls] = id;
        color->count_other_balls++;

        // signal N-1 times so the whole pack knows its ready
        for (int i = 0; i < N - 1; i++)
        {
            sem_post(&color->last_ball_arrived);
        }

        // handle output
        set_other_ids(color->other_ball_ids, other_ids, id, color->ball_one_id);

        // wait until its safe to reset for the whole pack
        for (int j = 0; j < N - 1; j++)
        {
            sem_wait(&color->safe_to_reset);
        }

        // run at the end so that it wraps up everything properly
        color->other_ball_ids = NULL;
        color->ball_one_id = -1;

        sem_post(&color->mutex); // only one last ball allowed
    }
}

void set_other_ids(int *original_other_ids, int *dest_other_ids, int id_to_remove, int id_to_insert)
{
    int destIndex = 0;
    for (int sourceIndex = 0; sourceIndex < N; ++sourceIndex)
    {
        if (original_other_ids[sourceIndex] != id_to_remove)
        {
            dest_other_ids[destIndex++] = original_other_ids[sourceIndex];
        }
    }
    dest_other_ids[N - 2] = id_to_insert;
}
