#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>

#include "packer.h"

typedef struct
{
    int *other_balls;
    int num_other_balls_arrived;
    int first_ball;
    sem_t mutex;
    sem_t waiting;
    sem_t first_ball_queue;
    sem_t wait_for_last;
    sem_t wait_for_saved_list;
} barrier;

barrier bar[3];
int n;
void remove_duplicate(int *source, int *dest, int n, int id);

void packer_init(int balls_per_pack)
{
    n = balls_per_pack;
    for (int i = 0; i < 3; i++)
    {
        bar[i].other_balls = NULL;
        bar[i].num_other_balls_arrived = 0;
        sem_init(&bar[i].mutex, 0, 1);
        sem_init(&bar[i].first_ball_queue, 0, 1);
        sem_init(&bar[i].wait_for_last, 0, 0);
        sem_init(&bar[i].wait_for_saved_list, 0, 0);
    }
}

void packer_destroy(void)
{
    for (int i = 0; i < 3; i++)
    {
        sem_destroy(&bar[i].mutex);
        sem_destroy(&bar[i].first_ball_queue);
        sem_destroy(&bar[i].wait_for_last);
        sem_destroy(&bar[i].wait_for_saved_list);
    }
}

void pack_ball(int colour, int id, int *other_ids)
{
    barrier *b = &bar[colour - 1];
    sem_wait(&b->mutex);
    if (b->other_balls == NULL)
    { // First ball to arrive
        sem_wait(&b->first_ball_queue);
        b->other_balls = other_ids;
        b->first_ball = id;
        b->num_other_balls_arrived = 0;
        sem_post(&b->mutex);
        sem_wait(&b->wait_for_last);
        remove_duplicate(b->other_balls, other_ids, n, id);
        sem_post(&b->wait_for_saved_list);
        sem_post(&b->first_ball_queue);
    }
    else
    { // Other balls arrive
        b->other_balls[b->num_other_balls_arrived] = id;
        b->num_other_balls_arrived++;
        if (b->num_other_balls_arrived == n - 1)
        {
            {
                sem_post(&b->wait_for_last);
            }
            remove_duplicate(b->other_balls, other_ids, n, id);
            other_ids[n - 2] = b->first_ball;
            for (int i = 0; i < n - 1; i++)
            {
                sem_wait(&b->wait_for_saved_list);
            }
            b->other_balls = NULL;
            sem_post(&b->mutex);
        }
        else
        {
            sem_post(&b->mutex);
            sem_wait(&b->wait_for_last);
            remove_duplicate(b->other_balls, other_ids, n, id);
            other_ids[n - 2] = b->first_ball;
            sem_post(&b->wait_for_saved_list);
        }
    }
}

void remove_duplicate(int *source, int *dest, int n, int id)
{
    int k = 0;
    int l = 0;
    while (source[k] != id && k < n)
    {
        dest[l] = source[k];
        k++;
        l++;
    }
    k++;
    while (k < n)
    {
        dest[l] = source[k];
        l++;
        k++;
    }
}