#include <semaphore.h>
#include <stdio.h>
#include "packer.h"

// You can declare global variables here
typedef struct
{
    int ball_one_id;
    int ball_two_id;
    sem_t mutex;
    sem_t second_ball_arrived;
    sem_t safe_to_reset;
} sema;

sema semaphores[4];

void packer_init(void)
{
    // Write initialization code here (called once at the start of the program).
    for (int i = 1; i < 4; i++)
    {
        sem_init(&semaphores[i].second_ball_arrived, 0, 0);
        sem_init(&semaphores[i].mutex, 0, 1);
        sem_init(&semaphores[i].safe_to_reset, 0, 0);
        semaphores[i].ball_one_id = -1;
        semaphores[i].ball_two_id = -1;
    }
}

void packer_destroy(void)
{
    // Write deinitialization code here (called once at the end of the program).
    for (int i = 1; i < 4; i++)
    {
        sem_destroy(&semaphores[i].second_ball_arrived);
        sem_destroy(&semaphores[i].mutex);
        sem_destroy(&semaphores[i].safe_to_reset);
    }
}

int pack_ball(int colour, int id)
{
    // Write your code here.
    int other;
    sema *color = &semaphores[colour];

    sem_wait(&color->mutex);
    if (color->ball_one_id == -1)
    {
        color->ball_one_id = id;
        sem_post(&color->mutex);
        sem_wait(&color->second_ball_arrived);
        other = color->ball_two_id;
        sem_post(&color->safe_to_reset);
    }
    else
    {
        color->ball_two_id = id;
        sem_post(&color->second_ball_arrived);
        other = color->ball_one_id;

        // safe_to_reset so it can be reused
        sem_wait(&color->safe_to_reset);
        color->ball_one_id = -1;
        color->ball_two_id = -1;
        sem_post(&color->mutex);
    }

    return other;
}