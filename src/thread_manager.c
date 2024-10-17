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
        SDL_Log("Running search with agent %u.\n",tdat->threadPar);
        SDL_Log("Query: %s\n",tdat->state->searchString);
        tdat->threadState = THREADSTATE_IDLE; //fdone searching (idle threads will eventually be killed)
        break;
      case THREADSTATE_IDLE:
        //printf("Thread %u is idle.\n",tdat->threadNum);
        break;
      default:
        break;
    }
  }

  //printf("Terminating thread %u.\n",tdat->threadNum);
  tdat->threadState = THREADSTATE_DEAD;
  return 1;
}

int startSearchThreads(app_data *restrict dat, app_state *restrict state, thread_manager_state *restrict tms){

  if(strlen(state->searchString)<=0){
    //no search string, no search threads needed
    return 0;
  }

  uint8_t startNumThreads = tms->numThreads;

  //determine number of threads
  int numCores = SDL_GetNumLogicalCPUCores();
  if(numCores <= 0){
    tms->numThreads = 1;
  }else if(numCores <= 2){
    tms->numThreads = (uint8_t)(numCores);
  }else{
    tms->numThreads = (uint8_t)(numCores-1);
  }
  if(tms->numThreads > SEARCHAGENT_ENUM_LENGTH){
    tms->numThreads = SEARCHAGENT_ENUM_LENGTH;
  }
  tms->masterThreadState = THREADSTATE_SEARCH;
  uint8_t numThreadsToStart = (uint8_t)(tms->numThreads - startNumThreads);
  if(numThreadsToStart > MAX_NUM_THREADS){
    SDL_Log("ERROR: startSearchThreads - trying to start invalid number of threads (%u).\n",numThreadsToStart);
    return -1;
  }
  printf("Starting %u search thread(s).\n",numThreadsToStart);

  uint8_t numThreadsStarted = 0;
  for(uint8_t i=0;i<MAX_NUM_THREADS;i++){
    if(!(tms->aliveThreads & (uint64_t)(1U << i))){
      if(tms->threadData[i].threadState == THREADSTATE_DEAD){
        char threadName[16];
        SDL_snprintf(threadName,16,"tp_%u",i);
        tms->threadData[i].threadNum = i;
        tms->threadData[i].threadState = THREADSTATE_SEARCH;
        tms->threadData[i].threadPar = numThreadsStarted;
        //assign data and state pointers
        tms->threadData[i].state = state;
        tms->threadData[i].dat = dat;
        SDL_Thread *thread = SDL_CreateThread(tpFunc,threadName,(void *)(intptr_t)(&tms->threadData[i]));
        SDL_DetachThread(thread);
        if(thread==NULL){
          printf("ERROR: startSearchThreads - couldn't create thread %u - %s\n",i,SDL_GetError());
          return -1;
        }
        tms->aliveThreads |= (uint64_t)(1U << i);
        numThreadsStarted++;
      }
    }
    if(numThreadsStarted == numThreadsToStart){
      break;
    }
  }
  if(numThreadsStarted != numThreadsToStart){
    SDL_Log("ERROR: startSearchThreads - started an invalid number of threads (%u, should be %u).\n",numThreadsStarted,numThreadsToStart);
    return -1;
  }
  
  return (int)(tms->numThreads);
}

void killIdleThreads(thread_manager_state *restrict tms){
  for(uint8_t i=0;i<MAX_NUM_THREADS;i++){
    if(tms->aliveThreads & (uint64_t)(1U << i)){
      if(tms->threadData[i].threadState == THREADSTATE_IDLE){
        tms->threadData[i].threadState = THREADSTATE_KILL;
        tms->numThreads--;
      }
    }
  }
}

void stopThreadPool(thread_manager_state *restrict tms){
  printf("Stopping %u thread(s).\n",tms->numThreads);
  for(uint8_t i=0;i<tms->numThreads;i++){
    tms->threadData[i].threadState = THREADSTATE_KILL;
  }
  tms->masterThreadState = THREADSTATE_KILL;
  tms->numThreads = 0;
}
