#include "../compiler/library/linux_compat.h"
#include <unordered_map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <string.h>
#include <queue>
#include <set>
#include <unistd.h>
#include <map>
#include <sys/signal.h>
#include <mutex>

/* Stores the data of the app to be started */
typedef struct App_data
{
    char appName[256];
    char mode[256];
    char fileName[256];
    long numOfPackets;
    char dev[256];
    bool debug_mode;
}App_data;

pthread_mutex_t lock;
pthread_t handle;

/* Queue of all requested apps to be started */
std::queue<App_data*> q;
/* Map that stores the pids of all running apps */
std::map<std::string, pid_t> runningApps;
/* Mutex for the map that stores the info of all running apps */
std::mutex mapMutex;

/* Function to free app data */
void free_appData(App_data ** ad)
{
    if(NULL != ad && NULL != *ad)
    {
        free(*ad);
        *ad = NULL;
    }
}

/* Thread that listens to user requests and add them to the queue */
void * listener_thread(void *)
{
    char input[256];
    App_data * ad;
    bool is_offline = true;
    int iStrLen = 0;
    while(true)
    {
        ad = (App_data *) malloc(sizeof(App_data));
        strcpy(input, "");
        printf("\nPlease enter the application that needs to be started:\n");

        /* Accept the input from user */
take_input: fgets(input, 256, stdin);
        if(!strcmp(input, "\n"))
        {
            strcpy(input, "");
            goto take_input;
        }
        iStrLen = strlen(input);
        input[iStrLen - 1] = '\0';

        /* Get app name */
        char *token = std::strtok(input, " ");
        if(token == NULL)
        {
            printf("\nIncorrect number of arguments supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }

        /* If the command is to stop an already running process */
        if(!strcmp(token, "stop"))
        {
            free_appData(&ad);
            token = std::strtok(NULL, " ");
            if(token == NULL)
            {
                /* If only stop is called, clean the process and exit the runtime */
                pthread_mutex_lock(&lock);
                while(!q.empty())
                {
                    ad = q.front();
                    free_appData(&ad);
                    q.pop();
                }
                pthread_mutex_unlock(&lock);
                mapMutex.lock();
                for(std::map<std::string, pid_t>::iterator iter = runningApps.begin();
                        iter != runningApps.end(); iter++)
                {
                    kill(iter->second, SIGTERM);
                }
                mapMutex.unlock();
                exit(0);
            }

            /* If stop is called with an app name, stop the app */
            mapMutex.lock();
            std::map<std::string, pid_t>::iterator i = runningApps.find(token);
            if(i != runningApps.end())
            {
                kill(i->second, SIGTERM);
                runningApps.erase(token);
                fprintf(stdout, "The requested process is stopped\n");
            }
            else
            {
                fprintf(stdout, "The requested app is not running\n");
            }
            mapMutex.unlock();
            continue;
        }

        strcpy(ad->appName, token);

        /* Get mode */
        token = std::strtok(NULL, " ");
        if(token == NULL)
        {
            printf("\nIncorrect number of arguments supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }
        if(!strcmp(token, "live") || !strcmp(token, "offline"))
        {
            strcpy(ad->mode, token);
            if(!strcmp(token, "live"))
                is_offline = false;
            else is_offline = true;
        }
        else
        {
            printf("\nIncorrect mode supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }

        /* Get dev name if live, else get file name */
        token = std::strtok(NULL, " ");
        if(token == NULL)
        {
            printf("\nIncorrect number of arguments supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }
        if(is_offline)
            strcpy(ad->fileName, token);
        else
            strcpy(ad->dev, token);

        /* Get loop number if offline */
        token = std::strtok(NULL, " ");

        if( (!is_offline && token != NULL) ||
                (token == NULL && is_offline) )
        {
            printf("\nIncorrect number of arguments supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }

        if(is_offline && token != NULL)
        {
            ad->numOfPackets = strtol(token, NULL, 10);
        }

        token = std::strtok(NULL, " ");
        if(token != NULL && !(strcmp(token, "debug")))
        {
            printf("\nIncorrect number of arguments supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }

        if(token != NULL && strcmp(token, "debug"))
        {
            ad->debug_mode = true;
        }

        token = std::strtok(NULL, " ");
        if(token != NULL)
        {
            printf("\nIncorrect number of arguments supplied. Please retry\n");
            free_appData(&ad);
            continue;
        }

        //printf("%s, %s, %s, %ld\n", ad->appName, ad->mode, ad->fileName, ad->numOfPackets);
        /* Push the app requested to the queue so that the main thread spawns
         * it */
        pthread_mutex_lock(&lock);
        q.push(ad);
        pthread_mutex_unlock(&lock);
    }
}

int main()
{
    int iErr;
    int flag = 1;
    int childPid = 0, retVal = 0;
    int iAppCount = 1;
    std::string sBinaryName;
    
    App_data * appData;

    if(pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\nMutex init failed. Please retry\n");
        exit(1);
    }

    /* Start the listener thread */
    iErr = pthread_create(&handle, NULL, &listener_thread, NULL);

    if (iErr != 0)
            printf("\nUnable to create thread. Please retry\n");

    /* Start processing all the user requests */
    while(flag)
    {
        appData = NULL;
        /* Pop from the queue to obtain the app request */
        pthread_mutex_lock(&lock);
        if(!q.empty())
        {
            appData = q.front();
            q.pop();
        }
        pthread_mutex_unlock(&lock);

        /* If there is any request */
        if(appData != NULL)
        {
            if(NULL == appData->appName)
            {
                printf("App not found. Please retry\n");
                free_appData(&appData);
                continue;
            }
            std::string appName(appData->appName);

            /* If the input file is not c++ file, throw an error */
            if(appName.substr(appName.find_last_of(".") + 1) != "cpp")
            {
                printf("Invalid app name. Please retry\n");
                free_appData(&appData);
                continue;
            }

            /* Create the app binary */
            size_t lastindex = appName.find_last_of("."); 

            std::string appNameRaw = appName.substr(0, lastindex);
            
            sBinaryName = std::string("app_") + std::to_string(iAppCount) +
                std::string("_") + appNameRaw;

            std::string comm = "g++ pcap_main.cpp " + appName + " -o " +
                sBinaryName + " -lpcap -std=gnu++11";
            
            if(system(comm.c_str()) != 0 || errno != 0)
            {
                printf("Error with the input file. Please retry\n");
                free_appData(&appData);
                continue;
            }

            /* Fork a new process for the app */
            childPid = fork();

            if(childPid <0)
            {
                printf("Line 255: Error while creating the app. Please retry\n");
                free_appData(&appData);
                continue;
            }
            else if(childPid == 0)
            {
                /* If child process, execute the binary created */

                if(strcmp(appData->mode, "live"))
                    retVal = execl(sBinaryName.c_str(), sBinaryName.c_str(), appData->mode,
                            appData->fileName, std::to_string(appData->numOfPackets).c_str(),
                            (char *) NULL);
                else
                    retVal = execl(sBinaryName.c_str(), sBinaryName.c_str(), appData->mode,
                            appData->dev, (char *) NULL);
                if(retVal)
                {
                    printf("Error while creating the app. Please retry\n");
                    free_appData(&appData);
                    continue;
                }

            }
            /* If the process is runtime, the child process id in the map */
            else if(childPid > 0)
            {
                mapMutex.lock();
                runningApps.insert(std::pair<std::string, pid_t>(sBinaryName, childPid));
                mapMutex.unlock();
                fprintf(stdout, "The requested app: %s has been started!\n", sBinaryName.c_str());
                iAppCount++;
            }
        }
    }
}
