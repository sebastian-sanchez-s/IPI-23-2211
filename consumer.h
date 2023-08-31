#pragma once

#include <stdio.h>
#include <pthread.h>

// @brief consumer data structure. Used to interproccess communication. 
struct consumer_data_t {
  int i;                  //!< resource index.
  pid_t pid;              //!< proccess id.
  pthread_t listener;     //!< listener for proccess in main.
  FILE *fs_w;             //!< file where consumer writes to.
  FILE *fs_r;             //!< file where main writes to communicate with consumer.
};
