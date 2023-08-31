/****************************************************************************
 * Copyright (C) 2023 by Sebastián Sánchez                                                      
 *                                                                          
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

/**
 * @file consumer.h
 * @author Sebastián Sánchez 
 * @date Aug 2032
 * @brief Data structures and headers use by consumer
 */
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
