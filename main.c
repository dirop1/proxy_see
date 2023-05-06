#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define IP_API "https://api.ipify.org"

typedef struct {
    char name[50];
    char url[250];
    char ip[50];
    double time;
} Proxy;

int countlines(char filename[])
{
    // count the number of lines in the file called filename
    FILE *fp = fopen(filename,"r");
    int ch=0;
    int lines=0;

    if (fp == NULL)
        return 0;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if(strlen(line) > 3 && line[0] != '#'){
            lines++;
        }
    }
    fclose(fp);
    return lines;
}

int readProxyFile(Proxy proxies[], char filename[]){
    FILE* ptr;

    Proxy nProxy;

    // Opening file in reading mode
    ptr = fopen(filename, "r");

    if (NULL == ptr) {
        printf("file can't be opened \n");
        return 100;
    }
    char hostname[40], url[150];
    int pos = 1;
    int posArray = 0;

    char line[512];
    char delim[] = "|";

    while (fgets(line, sizeof(line), ptr)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        char b = '#';

        if((strlen(line) > 3) && line[0] != b){
            char *splited = strtok(line, delim);

            while(splited != NULL)
            {
                if(pos++ == 1) {
                    strcpy(hostname, splited);
                }else{
                    pos = 1;
                    strcpy(url, strtok(splited, "\n"));
                }
                splited = strtok(NULL, delim);
            }

            strcpy(nProxy.name, hostname);
            strcpy(nProxy.url, url);
            strcpy(nProxy.ip, "unknown");
            nProxy.time = 10.12;

            proxies[posArray++] = nProxy;
        }
    }
    fclose(ptr);
    return 0;
}


void executeCmdAndWait(char prepend[], char cmd[], char ip[], double *time){
    clock_t t;
    t = clock();
    FILE *fp;
    fp = popen(cmd, "r");
    char buffer[100];
    if (fp == NULL) {
        printf("Failed to run command\n" );
        strcpy(ip, "error");
    }
    while (fgets(buffer, 100, fp) != NULL) {
        strcpy(ip, buffer);
    }

    t = clock() - t;
    *time = ((double)t) / CLOCKS_PER_SEC;;

    /* close */
    pclose(fp);
}

void printIpWithoutProxy(char ip[], double *time){

    char cmd[150] = "curl -s ";
    strcat(cmd, IP_API);
    executeCmdAndWait("No proxy", cmd, ip, time);
}
void printIpWithProxy(Proxy proxy, char url[], char ip[], double *time){
    char cmd[500] = "curl -s -x ";

    strcat(cmd, proxy.url);
    strcat(cmd, " ");
    strcat(cmd, url);

    executeCmdAndWait(proxy.name, cmd, ip, time);
}

void* process_proxy(void* arg) {
    Proxy* proxy = (Proxy*) arg;
    if(strncmp(proxy->name, "localhost",6) != 0  ||  strncmp(proxy->url, "none",4) != 0){
        printIpWithProxy(*proxy, IP_API, proxy->ip, &proxy->time);
    }else{
        printIpWithoutProxy(proxy->ip, &proxy->time);
    }
    printf("| %15.15s| %15.15s|\t%f|\n", proxy->name , proxy->ip, proxy->time);
    return NULL;
}


void scanInput(){
    char c;
    scanf(" %c",&c);
    printf("%c",c);
}

void execProxiesIpFromFileMultiThreaded(char filename[]) {
    clock_t tic;
    tic = clock();
    int maxProxies = countlines(filename);
    printf("Reading %s with %d lines:\n", filename, maxProxies);
    Proxy proxies[maxProxies];
    pthread_t threads[maxProxies];

    printf("\n--------------  -----Results----  ---------------\n");
    printf("._______________________________________________.\n");
    printf("| %15.15s| %15.15s|%13.13s|\n", "NAME", "IP", "      Time");
    printf("|________________|________________|_____________|\n");

    readProxyFile(proxies, filename);

    for (int i = 0; i < maxProxies; ++i) {
        pthread_create(&threads[i], NULL, process_proxy, &proxies[i]);
    }

    // wait for all threads to finish
    for (int i = 0; i < maxProxies; ++i) {
        pthread_join(threads[i], NULL);
    }


    tic = clock() - tic;
    double total_time = ((double) tic) / CLOCKS_PER_SEC;

    printf("|-----------------------------------------------|\n");
    printf("|  %15.15s%15.15d\t%f|\n","TOTAL:|", maxProxies, total_time);
    printf("|_______________________________________________|\n");
}


int main( int argc, char *argv[] ) {

    char filelocation[100];


    if( argc > 1 ) {
        strcpy(filelocation,argv[1]);
    }else{
        strcpy(filelocation,"proxies.psv");
    }
    
    if(access(filelocation, F_OK) == 0){
        execProxiesIpFromFileMultiThreaded(filelocation);
        exit(0);
    }else{
        printf("\nError\nCouldn't find the proxies.psv file create one in this location or pass it as an argument\n\n");
    }

    
    
    return 100;
}



