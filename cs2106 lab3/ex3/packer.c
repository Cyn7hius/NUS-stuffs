#include <semaphore.h>
#include <stdio.h>
#include "packer.h"

// You can declare global variables here
typedef struct
{
    int ball_id;
    sem_t mutex;
    sem_t other_ball_arrived;
    sem_t safe_to_reset;
} sema;

sema semaphores[5];

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    for (int i = 1; i < 5; i++)
    {
        sem_init(&semaphores[i].other_ball_arrived, 0, 0);
        sem_init(&semaphores[i].mutex, 0, 1);
        sem_init(&semaphores[i].safe_to_reset, 0, 0);
        semaphores[i].ball_id = -1;
    }
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    for (int i = 1; i < 5; i++)
    {
        sem_destroy(&semaphores[i].other_ball_arrived);
        sem_destroy(&semaphores[i].mutex);
        sem_destroy(&semaphores[i].safe_to_reset);
    }
}

int pack_ball(int colour, int id)
{
    // Write your code here.
    int other;
    sema *color = &semaphores[colour];
    sema *other_color = &semaphores[colour % 2 == 0 ? colour - 1 : colour + 1];

    if (colour == 1 || colour == 3) {
        //Wait until mutex is free and enter mutex
        sem_wait(&color->mutex);
        color->ball_id = id;
        // Signal to other semaphore that ball 1/3 has arrived
        sem_post(&color->other_ball_arrived);
        //Wait for the other ball(2/4) to arrive
        sem_wait(&other_color->other_ball_arrived);
        //Record id of ball 2/4
        other = other_color->ball_id;
        //Reset semaphore 2/4
        sem_post(&other_color->safe_to_reset);
        //Reset semaphore 1/3
        sem_wait(&color->safe_to_reset);
        color -> ball_id = -1;
        sem_post(&color->mutex);
    } else {
        //Wait until mutex is free and enter mutex
        sem_wait(&color->mutex);
        color->ball_id = id;
        // Wait for ball to arrive in semaphore 1/3
        sem_wait(&other_color->other_ball_arrived);
        other = other_color->ball_id;
        //Take id of ball 1/3 and signal that ball 2/4 has arrived
        sem_post(&color->other_ball_arrived);
        //Id of ball 2/4 taken, reset current semaphore
        sem_wait(&color->safe_to_reset);
        color -> ball_id = -1;
        //Reset semaphore 1/3
        sem_post(&other_color->safe_to_reset);
        sem_post(&color->mutex);
    }

    return other;
}