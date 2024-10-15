/*
Copyright (C) 2017-2024 J. Williams

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* Functions handling the thread pool */

#include "thread_manager.h"
#include "data_ops.h"

//monolithic callback function for threads in the pool
int tpFunc(void *data){
  thread_data *tdat = ((thread_data*)(intptr_t)(data)); //get the thread data (double cast to avoid warning)
  //printf("Initialized thread %u.\n",tdat->threadNum);

  //main loop for the thread
  while(tdat->threadState != THREADSTATE_KILL){
    SDL_Delay(THREAD_UPDATE_DELAY);
    switch(tdat->threadState){
      case THREADSTATE_SEARCH:
        //run a search agent
        break;
      case THREADSTATE_IDLE:
        //printf("Thread %u is idle.\n",tdat->threadNum);
        break;
      default:
        break;
    }
  }

  //printf("Terminating thread %u.\n",tdat->threadNum);
  return 1;
}

int startThreadPool(app_data *restrict dat, app_state *restrict state, thread_manager_state *restrict tms){

  //determine number of threads
  int numCores = SDL_GetNumLogicalCPUCores();
  if(numCores <= 0){
    tms->numThreads = 1;
  }else if(numCores <= 2){
    tms->numThreads = (uint8_t)(numCores);
  }else{
    tms->numThreads = (uint8_t)(numCores-1);
  }
  if(tms->numThreads > MAX_NUM_THREADS){
    tms->numThreads = MAX_NUM_THREADS;
  }
  tms->masterThreadState = THREADSTATE_IDLE;
  printf("Starting %u thread(s).\n",tms->numThreads);

  for(uint8_t i=0;i<tms->numThreads;i++){
    char threadName[16];
    SDL_snprintf(threadName,16,"tp_%u",i);
    tms->threadData[i].threadNum = i;
    tms->threadData[i].threadState = THREADSTATE_IDLE;
    tms->threadData[i].threadPar = 0;
    //assign data and state pointers
    tms->threadData[i].state = state;
    tms->threadData[i].dat = dat;
    SDL_Thread *thread = SDL_CreateThread(tpFunc,threadName,(void *)(intptr_t)(&tms->threadData[i]));
    SDL_DetachThread(thread);
    if(thread==NULL){
      printf("ERROR: startThreadPool - couldn't create thread %u - %s\n",i,SDL_GetError());
      return -1;
    }
    
  }
  
  return 0;
}

void stopThreadPool(thread_manager_state *restrict tms){
  printf("Stopping %u thread(s).\n",tms->numThreads);
  for(uint8_t i=0;i<tms->numThreads;i++){
    tms->threadData[i].threadState = THREADSTATE_KILL;
  }
  tms->numThreads = 0;
}
