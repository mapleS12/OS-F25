#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>


#define MAX_ITEMS 1024
#define MAX_NAME_LENGTH 2048
#define PAGE_LENGTH 5

typedef struct {
    char name[MAX_NAME_LENGTH];
    int isDirectory;
} Item;

// Item file[MAX_ITEMS];
// int itemCount = 0;

int readDirectory(Item items[], int *count){
    DIR *dp = opendir(".");
    if (!dp){
        return -1;
    }
    struct dirent *entry;
    int total =0;

    while ((entry = readdir(dp)) != NULL && total < MAX_ITEMS){
        if (entry->d_name[0]=='.') continue;
        strncpy(items[total].name, entry->d_name, MAX_NAME_LENGTH-1);
        items[total].name[MAX_NAME_LENGTH-1]='\0';
        struct stat st;
        if (stat(items[total].name, &st) == 0 && S_ISDIR(st.st_mode)) {
            items[total].isDirectory = 1;
        } else {
            items[total].isDirectory = 0;
        }

        total ++;
    }

    closedir(dp);
    *count=total;
    return 0;
}


void printHead(){
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd))==NULL){
        strcpy(cwd, "Unknown");
    }

    time_t now = time(NULL);
    char *timestr = ctime(&now);
    if(timestr) timestr[strlen(timestr)-1] = '\0';
    
    printf("Current Working Directory: %s\n", cwd);
    printf("It is now: %s\n", timestr);
}


void printMenu(){
    printf("\nCommans:\n");
    printf("D - Display the Menu Again\n");
    printf("E - Edit File\n");
    printf("R - Run File\n");
    printf("C - Change Directory\n");
    printf("S - Sort Files\n");
    printf("X - Remove File\n");
    printf("N - Next Page\n");
    printf("P - Previous Page\n");
    printf("Q - Quit\n");
}

void printItems(Item items[], int count, int page){
    int start = page*PAGE_LENGTH;
    int end = start + PAGE_LENGTH;
    if(end>count) end = count;
    printf("\nFiles & DIrectories: \n");
    for(int i =start; i<end; i++){
        printf("%d. %s", i, items[i].name);
        if (items[i].isDirectory == 1) {
            printf("/");
        }
        printf("\n");
    }
}

int cmpName(const void *a, const void *b){
    return strcmp(((Item*)a)->name, ((Item*)b)->name);
}

void editFile(Item items[], int index, int count){
    if(index<0 || index>=count || items[index].isDirectory){
        printf("Invalid file\n");
        return;
    }
    pid_t pid = fork();
    if(pid==0){
        execlp("nano", "nano", items[index].name, NULL);
        perror("execlp");
        exit(1);
    } else {
        wait(NULL);
    }
}


void changeDirectory(Item items[], int index, int count){
    if(index<0 || index>=count || !items[index].isDirectory){
        return;
    }
    if(chdir(items[index].name)!=0) perror("chdir");
}



void removeFile(Item items[], int index, int count){
    if(index<0 || index>=count || items[index].isDirectory){
        return;
    }
    if(remove(items[index].name)!=0) perror("remove");
    else printf("File %s removed.\n", items[index].name);
}


void runFile(Item items[], int index, int count){
    if(index<0 || index>=count || items[index].isDirectory){
        printf("Invalid file\n");
        return;
    }
    pid_t pid = fork();
    if(pid==0){
        execlp("./a.out", items[index].name, NULL);
        perror("execlp");
        exit(1);
    } else {
        wait(NULL);
    }
}


int main(void){
    Item items[MAX_ITEMS];
    int totalItems=0;
    int page = 0;

    while (1)
    {
        if(readDirectory(items,&totalItems)<0){
            break;
        }
        qsort(items,totalItems,sizeof(Item),cmpName);

        printHead();
        printItems(items,totalItems,page);
        printMenu();
        printf("\nEnter command: ");
        char cmd;
        scanf(" %c", &cmd);
        while(getchar()!='\n'); 

        if(cmd=='Q'||cmd=='q'){
            break;

        }else if(cmd=='N'||cmd=='n'){
            if((page+1)*PAGE_LENGTH < totalItems) page++;
        }else if(cmd=='P'||cmd=='p'){
            if(page>0) page--;
        }else if(cmd=='S'||cmd=='s'){
            qsort(items,totalItems,sizeof(Item),cmpName);
            printf("Sorted.. Enter to continue...");
            getchar();
        }else if(cmd=='E'||cmd=='e'){
            int idx;
            printf("Enter file index to edit: ");
            scanf("%d",&idx);
            while(getchar()!='\n');
            editFile(items,idx,totalItems);
        }else if(cmd=='R'||cmd=='r'){
            int idx;
            printf("Enter index to run: ");
            scanf("%d",&idx);
            while(getchar()!='\n');
            runFile(items,idx,totalItems);
        }else if(cmd=='C'||cmd=='c'){
            int idx;
            printf("Enter directory index to change: ");
            scanf("%d",&idx);
            while(getchar()!='\n');
            changeDirectory(items,idx,totalItems);
            page=0;
        }else if(cmd=='X'||cmd=='x'){
            int idx;
            printf("Enter file index to remove: ");
            scanf("%d",&idx);
            while(getchar()!='\n');
            removeFile(items,idx,totalItems);
        }else if(cmd=='D'||cmd=='d'){
            continue;
        }else {
            printf("Invalid.. Press Enter to continue..");
            getchar();
        }







    }

    printf("Closing...\n");
    return 0;

    
}
