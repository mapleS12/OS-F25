// ROHITA KONJETI
// CSE 3320
// Lab Assignment 2

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#define PROCESS_COUNT 2 //adjustable amount
#define LINE_MAX_LENGTH 512
#define DATA_FILE "all_month.csv" // CHANGE NAME WITH INPUT DATA FILE

int countNumLines(const char *filename){
    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        perror("Error in opening file...exiting");
        return 0;
    }
    int count = 0;
    char buffer[LINE_MAX_LENGTH];
    fgets(buffer, LINE_MAX_LENGTH, fp); // skip header
    while (fgets(buffer, LINE_MAX_LENGTH, fp)){
        count++;
    }
    fclose(fp);
    return count;
}


void writeData(const char *sourceFile, const char *destFile, int firstLineIdx, int lineNum){
    FILE *src = fopen(sourceFile, "r");
    FILE *dst = fopen(destFile, "w");
    if(src==NULL||dst ==NULL){
        perror("Error in making the file");
        exit(1);
    }

    char buffer[LINE_MAX_LENGTH];
    fgets(buffer, LINE_MAX_LENGTH, src); // skip header

    for(int i = 0; i<firstLineIdx; i++){
        if(!fgets(buffer, LINE_MAX_LENGTH, src)){
            break;
        }
    }

    for(int i = 0; i<lineNum; i++){
        if(fgets(buffer, LINE_MAX_LENGTH, src)){
            fputs(buffer, dst);
        }else{
            break;
        }
    }
    fclose(src);
    fclose(dst);
}

void merge(){
    FILE *final = fopen("sortedOutput.csv","w");
    if(!final){
        perror ("Error in opening file");
        return;
    }

    fprintf(final, "time,latitude,longitude,depth,mag,magType,nst,gap,dmin,rms,net,id,updated,place,type,horizontalError,depthError,magError,magNst,status,locationSource,magSource\n");

    // FILE *parent = fopen(DATA_FILE, "r");
    // char header_buffer[LINE_MAX_LENGTH];
    // if (parent && fgets(header_buffer, LINE_MAX_LENGTH, parent)) {
    //     fputs(header_buffer, final);
    // }
    // if(parent != NULL){
    //     fclose(parent);
    // }

    FILE *chunks[PROCESS_COUNT];
    //char buffers[PROCESS_COUNT][LINE_MAX_LENGTH];
    int active[PROCESS_COUNT];
    char **buffers = malloc(PROCESS_COUNT * sizeof(char *));
    if (!buffers){
        perror("Error malloc did not work");
        fclose(final);
        return;
    }

    // char **buffers = malloc(PROCESS_COUNT * sizeof(char *));
    // for (int i = 0; i < PROCESS_COUNT; i++) {
    //     buffers[i] = malloc(LINE_MAX_LENGTH);
    // }


    for(int i=0; i<PROCESS_COUNT; i++){
        buffers[i] = malloc(LINE_MAX_LENGTH);
        if (!buffers[i]){
        perror("Error malloc did not work");
        fclose(final);
        return;
    }
        char fname[50];
        sprintf(fname, "output%d.csv", i + 1);
        chunks[i] = fopen(fname, "r");
        if (chunks[i]) {
            char header[LINE_MAX_LENGTH];
            fgets(header, LINE_MAX_LENGTH, chunks[i]); // SKIP header

            if (fgets(buffers[i], LINE_MAX_LENGTH, chunks[i])) {
                active[i] = 1;
            } else {
                active[i] = 0;
            }
        } else {
            active[i] = 0;
        }
    }

    // for(int i=0; i<PROCESS_COUNT; i++){
    //     char filename[50];
    //     sprintf(filename, "output%d.csv", i+1);
    //     FILE *chunk = fopen(filename, "r");
    //     if(chunk){
    //         char buffer[LINE_MAX_LENGTH];
    //         while(fgets(buffer, LINE_MAX_LENGTH, chunk)){
    //             fputs(buffer, final);
    //         }
    //         fclose(chunk);
    //     }
    //     // remove temporary files
    //     remove(filename);
    //     sprintf(filename, "chunk_in_%d.csv", i+1);
    //     remove(filename);
    // }

    // fclose(final);
    // printf("Merged into sortedOutput.csv\n");

    // int minIdx= -1;
    // double minLat= 0;

    while(1){
        int minIdx= -1;
    double minLat= 1e9;//used to be zero but changed for testing

        for (int i=0; i< PROCESS_COUNT; i++){
            if (active[i]){
                double lat;
                if (sscanf(buffers[i], "%*[^,],%lf", &lat) == 1){
                    if (minIdx == -1 || lat< minLat){
                        minLat = lat;
                        minIdx = i;
                    }
                }
            }
        }
        if (minIdx == -1){
        break;
    }


    fputs(buffers[minIdx], final);

    if (fgets(buffers[minIdx], LINE_MAX_LENGTH, chunks[minIdx])){
        active[minIdx]=1;
    }else{
        active [minIdx] = 0;
    }

    }

    



    for (int i=0; i<PROCESS_COUNT; i++){
        if (chunks[i]) fclose(chunks[i]);
        free(buffers[i]);
        // char fname[50];
        // sprintf(fname, "chunkOutput%d.csv", i+1);
        // remove(fname);
        // sprintf(fname, "chunkInput%d.csv", i+1);
        // remove(fname);
    }
    free(buffers);



    // for (int i = 0; i < PROCESS_COUNT; i++) {
    //     free(buffers[i]);
    // }
    // free(buffers);


    fclose(final);
    printf("Output is in file : sortedOutput.csv\n");




}




int main(void){
    int totalLines = countNumLines(DATA_FILE);

    if (totalLines< PROCESS_COUNT || totalLines ==0){
        fprintf(stderr, "Error with file\n");
        return 1;
    }

    //to get the size of the chunks
    int size = totalLines/PROCESS_COUNT;
    int remaning = totalLines%PROCESS_COUNT;
    int currLine = 0;

    time_t t_start = time(NULL);
    printf("Driver starts at %s\n", ctime(&t_start));

    pid_t parent_pid = getpid();
    printf("Driver PID: %d\n", parent_pid);

    for(int i=0; i<PROCESS_COUNT; i++){
        int linesForThis = size + (i == PROCESS_COUNT-1 ? remaning : 0);

        char inFile[50];
        char outFile[50];
        sprintf(inFile, "input%d.csv", i+1);
        sprintf(outFile, "output%d.csv", i+1);

        writeData(DATA_FILE, inFile, currLine, linesForThis);
        currLine += linesForThis;

        pid_t p = fork();
        if(p==0){
            // child process
            char *args[] = {"./u", inFile, outFile, NULL};
            execv("./u", args);
            perror("execv failed");
            exit(1);
        } else if(p>0){
            // parent process continues
            printf("Launched child %d (PID=%d)\n", i+1, p);
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    for(int i=0; i<PROCESS_COUNT; i++){
        wait(NULL);
    }

    printf("All children finished. Now merging...\n");
    merge();

    time_t t_end = time(NULL);
    printf("Driver finished at %s\n", ctime(&t_end));
    printf("Total time: %ld seconds\n", (long)(t_end - t_start));

    return 0;
}
