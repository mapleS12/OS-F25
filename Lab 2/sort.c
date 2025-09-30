// ROHITA KONJETI
// CSE 3320
// Lab Assignment 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define LINE_MAX_LENGTH 512
#define MAX_NUM_RECORDS 10000 

//extracts lat and compares 
int compare(const char *line1, const char *line2){
    double lat1;
    double lat2;

    // if (sscanf(line1, "%lf", &lat1) != 1 || sscanf(line2, "%lf", &lat2) != 1){
    //     return 0;
    // }
    sscanf(line1, "%*[^,],%lf", &lat1);
    sscanf(line2, "%*[^,],%lf", &lat2);


    //sorts in acending order
    if (lat1<lat2){
         return -1;
    }else if(lat1>lat2){
        return 1;
    } else {
        return 0;
    }


}

//bubble sort in order to reorder
void bubbleSort(char **dataArray, int count){
    int i;
    int j; 
    char *temp;
    for (i = 0; i<count-1; i++){
        for(j=0; j<count-1-i; j++){
            if(compare(dataArray[j], dataArray[j+1])>0){
                temp = dataArray[j];
                dataArray[j]=dataArray[j+1];
                dataArray[j+1]=temp;
            }

        }
    }


}





int main(int argc, char *argv[]){
    if (argc <3){
        fprintf(stderr, "Missing input");
        return 1;
    }
    char *inputFilename = argv[1];
    char *outputFilename = argv[2];

    FILE *fp = fopen(inputFilename, "r");
    if (fp ==NULL){
        perror("File did not open...closing");
        return 1;

    }

    char **data =(char **)malloc(MAX_NUM_RECORDS * sizeof(char *));
    int recordCount = 0;
    char buffer[LINE_MAX_LENGTH];

    while (fgets(buffer, LINE_MAX_LENGTH, fp)&& recordCount<MAX_NUM_RECORDS)
    {
        buffer[strcspn(buffer, "\n")] =0;
        data[recordCount]=strdup(buffer);
        if (data[recordCount] == NULL)
        {
            break;
        }
        recordCount++;
        
    }
    fclose(fp);
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    bubbleSort(data, recordCount);
    gettimeofday(&end, NULL);

    long seconds = end.tv_sec-start.tv_sec;
    long micros = end.tv_usec-start.tv_usec;
    double spentTime= seconds +micros*1e-6;

    FILE *outfp = fopen(outputFilename, "w");
    if (outfp ==NULL){
        perror("File did not open...closing");
        return 1;

    }

    printf("Sorter PID %d | Sorted %d records, from %s in %f seconds.\n", getpid(), recordCount, inputFilename, spentTime);

    for(int i=0; i<recordCount; i++){
        fprintf(outfp, "%s\n", data[i]);
        free(data[i]);

    }
fclose(outfp);

free(data);
exit(0);



}
