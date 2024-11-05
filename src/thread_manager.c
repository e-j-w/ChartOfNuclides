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
#include "search_ops.h"

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
        //SDL_Log("Running search with agent %u.\n",tdat->threadPar);
        //SDL_Log("Query: %s\n",tdat->state->ss.searchString);
        switch(tdat->threadPar){
          case SEARCHAGENT_TOKENIZE:
            if(!(tdat->state->ss.finishedSearchAgents & (uint32_t)(1U << SEARCHAGENT_TOKENIZE))){
              tokenizeSearchStr(&tdat->state->ss);
              tdat->state->ss.finishedSearchAgents |= (uint32_t)(1U << SEARCHAGENT_TOKENIZE); //flag search agent as finished
            }
            break;
          case SEARCHAGENT_NUCLIDE:
            while(!(tdat->state->ss.finishedSearchAgents & (uint32_t)(1U << SEARCHAGENT_TOKENIZE))){
              if(tdat->threadState == THREADSTATE_KILL){
                break; //kill waiting thread, if requested
              }
              SDL_Delay(THREAD_UPDATE_DELAY); //wait for tokenization to complete
            }
            //SDL_Log("Searching for nuclides...\n");
            searchNuclides(&tdat->dat->ndat,&tdat->state->ss);
            break;
          case SEARCHAGENT_EGAMMA:
            while(!(tdat->state->ss.finishedSearchAgents & (uint32_t)(1U << SEARCHAGENT_TOKENIZE))){
              if(tdat->threadState == THREADSTATE_KILL){
                break; //kill waiting thread, if requested
              }
              SDL_Delay(THREAD_UPDATE_DELAY); //wait for tokenization to complete
            }
            //SDL_Log("Searching for transitions...\n");
            searchEGamma(&tdat->dat->ndat,&tdat->state->ss);
            break;
          case SEARCHAGENT_ELEVEL:
            while(!(tdat->state->ss.finishedSearchAgents & (uint32_t)(1U << SEARCHAGENT_TOKENIZE))){
              if(tdat->threadState == THREADSTATE_KILL){
                break; //kill waiting thread, if requested
              }
              SDL_Delay(THREAD_UPDATE_DELAY); //wait for tokenization to complete
            }
            //SDL_Log("Searching for levels...\n");
            searchELevel(&tdat->dat->ndat,&tdat->state->ss);
            break;
          default:
            break;
        }
        tdat->threadState = THREADSTATE_IDLE; //fdone searching (idle threads will eventually be killed)
        break;
      case THREADSTATE_IDLE:
        ; //suppress gcc warning
        //send a fake SDL event to ensure the idle state is processed on the next frame
        SDL_Event evt;
        SDL_PushEvent(&evt); //force frame update even if time has passed, as SDL will wait for an event
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

  if(strlen(state->ss.searchString)<=0){
    //no search string, no search threads needed
    //clear search results
    state->ss.numResults = 0;
    return 0;
  }

  //initialize search state
  memset(state->ss.updatedResults,0,sizeof(state->ss.updatedResults));
  state->ss.finishedSearchAgents = 0;
  state->ss.numUpdatedResults = 0;

  //determine number of threads
  tms->numThreads = SEARCHAGENT_ENUM_LENGTH;
  tms->masterThreadState = THREADSTATE_SEARCH;
  if(tms->numThreads > MAX_NUM_THREADS){
    SDL_Log("ERROR: startSearchThreads - trying to start invalid number of threads (%u).\n",tms->numThreads);
    return -1;
  }
  //printf("Starting %u search thread(s).\n",tms->numThreads);

  uint8_t numThreadsStarted = 0;
  uint8_t i=0;
  uint16_t loopCtr = 0;
  while(numThreadsStarted < tms->numThreads){
    if((!(tms->aliveThreads & (uint64_t)(1UL << i)))&&(tms->threadData[i].threadState == THREADSTATE_DEAD)){
      //start a new thread at this slot
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
      tms->aliveThreads |= (uint64_t)(1UL << i);
      numThreadsStarted++;
    }else{
      //active thread, kill it so that this slot can be used again later
      if(tms->threadData[i].threadState != THREADSTATE_KILL){ //check to make sure this thread hasn't already been flagged by killIdleThreads
        tms->threadData[i].threadState = THREADSTATE_KILL;
        tms->aliveThreads = (uint64_t)(tms->aliveThreads & ~(1UL << i)); //unset
      }
    }
    i++;
    loopCtr++;
    if(i >= MAX_NUM_THREADS){
      i=0; //loop back and see if any threads that were killed can now be used
      SDL_Delay(THREAD_UPDATE_DELAY); //oof
    }
    if(loopCtr >= 2048){
      SDL_Log("ERROR: startSearchThreads - probable infinite loop.\n");
      return -1;
    }
  }
  if(numThreadsStarted != tms->numThreads){
    SDL_Log("ERROR: startSearchThreads - started an invalid number of threads (%u, should be %u).\n",numThreadsStarted,tms->numThreads);
    return -1;
  }
  
  return (int)(tms->numThreads);
}

void killIdleThreads(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, thread_manager_state *restrict tms){
  for(uint8_t i=0;i<MAX_NUM_THREADS;i++){
    if(tms->aliveThreads & (uint64_t)(1UL << i)){
      if(tms->threadData[i].threadState == THREADSTATE_IDLE){
        tms->threadData[i].threadState = THREADSTATE_KILL;
        tms->aliveThreads = (uint64_t)(tms->aliveThreads & ~(1UL << i)); //unset
        if(tms->numThreads > 0){
          tms->numThreads--;
        }
      }
    }
  }
  //take action once all threads are done
  if(tms->numThreads == 0){
    if(tms->masterThreadState == THREADSTATE_SEARCH){
      //search is finished, copy over the search results
      //SDL_Log("Search finished.\n");
      if(strlen(state->ss.searchString)>0){
        memcpy(state->ss.results,state->ss.updatedResults,sizeof(state->ss.updatedResults));
        state->ss.numResults = state->ss.numUpdatedResults;
        updateSearchUIState(dat,state,rdat);
      }else{
        //perhaps the user pressed backspace too quickly...
        state->ss.numResults = 0;
      }
      tms->masterThreadState = THREADSTATE_KILL;
      //send a fake SDL event to ensure the dearch results are processed on the next frame
      //regardless of the presence of user input or other events
      SDL_Event evt;
      SDL_PushEvent(&evt); //force frame update even if time has passed, as SDL will wait for an event
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
