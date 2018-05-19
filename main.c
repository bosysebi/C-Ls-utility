#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include "common.h"
#include "mode_to_string.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <string.h>

#define UNUSED(VAR) ((void) VAR)


char *getcwd(char *buf, size_t size);

struct maxSizes{
    size_t filesize;
    size_t username;
    size_t groupname;
    size_t lastmod;
    size_t name;
    size_t inode;
    size_t hardLinks;
};

typedef struct{
    char rights[11];
    size_t filesize;
    char *username;
    char *groupname;
    char lastmod[64];
    char *name;
    long inode;
    unsigned long hardLinks;
    int hidden;
} fileEntry;


size_t digitCount(long num)
{
    size_t n = 0;
    if(!num){
        return 1;
    }
    while(num) {
        num /= 10;
        n++;
    }
    return n;
}

void printID(fileEntry *entry, struct maxSizes *sizes, char *identifier, int shower){
    int i = 0;
    if(shower == 0 && entry->name[0] == '.'){
        return;
    }
    if(shower == 1 && entry->name[0] == '.' && entry->name[1] == '.' && !entry->name[3]){
        return;
    }
    while(identifier[i]){
        if(i!=0){
            printf(" ");
        }
        switch(identifier[i]){
            case 'R':
                printf("%s", entry->rights);
                break;
            case 'L':
                if(digitCount(entry->hardLinks)<sizes->hardLinks){
                    size_t spaces = sizes->hardLinks - digitCount(entry->hardLinks);
                    for(size_t j = 0; j < spaces; j++){
                        printf(" ");
                    }
                }
                printf("%lu", entry->hardLinks);
                break;
            case 'U':
                printf("%s", entry->username);
                if(!identifier[i+1]){
                    break;
                }
                if(strlen(entry->username) < sizes->username){
                    size_t spaces = sizes->username - strlen(entry->username);
                    for(size_t j = 0; j < spaces; j++){
                        printf(" ");
                    }
                }
                break;
            case 'G':
                printf("%s", entry->groupname);
                if(!identifier[i+1]){
                    break;
                }
                if(strlen(entry->groupname) < sizes->groupname){
                    size_t spaces = sizes->groupname - strlen(entry->groupname);
                    for(size_t j = 0; j < spaces; j++){
                        printf(" ");
                    }
                }
                break;
            case 'S':
                if(digitCount(entry->filesize)<sizes->filesize){
                    size_t spaces = sizes->filesize - digitCount(entry->filesize);
                    for(size_t j = 0; j < spaces; j++){
                        printf(" ");
                    }
                }
                printf("%zu", entry->filesize);
                break;
            case 'M':
                printf("%s", entry->lastmod);
                break;
            case 'N':
                printf("%s", entry->name);
                if(!identifier[i+1]){
                    break;
                }
                if(strlen(entry->name) < sizes->name){
                    size_t spaces = sizes->name - strlen(entry->name);
                    for(size_t j = 0; j < spaces; j++){
                        printf(" ");
                    }
                }
                break;
            case 'I':
                if(digitCount(entry->inode)<sizes->inode){
                    size_t spaces = sizes->inode - digitCount(entry->inode);
                    for(size_t j = 0; j < spaces; j++){
                        printf(" ");
                    }
                }
                printf("%zu", entry->inode);
                break;
        }
        i++;
    }
    printf("\n");
}

void resetSizes(struct maxSizes *sizes){
    sizes->filesize = 0;
    sizes->groupname = 0;
    sizes->hardLinks = 0;
    sizes->inode = 0;
    sizes->lastmod = 0;
    sizes->name = 0;
    sizes->username = 0;
}

void getMaxSizes(fileEntry *entries[], int size, struct maxSizes *sizes){
    resetSizes(sizes);
    for(int i = 0; i < size; i++){
        if(strlen(entries[i]->username)>sizes->username){
            sizes->username = strlen(entries[i]->username);
        }
        if(strlen(entries[i]->name)>sizes->name){
            sizes->name = strlen(entries[i]->name);
        }
        if(strlen(entries[i]->groupname)>sizes->groupname){
            sizes->groupname = strlen(entries[i]->groupname);
        }
        if(strlen(entries[i]->lastmod)>sizes->lastmod){
            sizes->lastmod = strlen(entries[i]->lastmod);
        }
        if(digitCount(entries[i]->filesize) > sizes->filesize){
            sizes->filesize = digitCount(entries[i]->filesize);
        }
        if(digitCount(entries[i]->hardLinks) > sizes->hardLinks){
            sizes->hardLinks = digitCount(entries[i]->hardLinks);
        }
        if(digitCount(entries[i]->inode) > sizes->inode){
            sizes->inode = digitCount(entries[i]->inode);
        }
    }
}

char* getUserName(struct stat *stat){
    struct passwd *pwd;
    pwd = getpwuid(stat->st_uid);
    return pwd->pw_name;
}

char* getGroupName(struct stat *stat){
    struct group *grp;
    grp = getgrgid(stat->st_uid);
    return grp->gr_name;
}

void readEntry(struct dirent *dir, fileEntry *entry){
    int desc = open(dir->d_name, O_RDONLY);
    if (desc == -1) {
        perror(dir->d_name);
        return;
    }
    struct stat *test = malloc(sizeof(struct stat));
    int statdesc = fstat(desc, test);
    if(statdesc == -1){
        perror(dir->d_name);
        return;
    }
    entry->filesize = test->st_size;
    entry->username = getUserName(test);
    entry->groupname = getGroupName(test);
    strftime(entry->lastmod, 64, "%b %e %Y %R", localtime(&test->st_mtime));
    entry->name = dir->d_name;
    mode_to_string(test->st_mode, entry->rights);
    entry->inode = (long) test->st_ino;
    entry->hardLinks = test->st_nlink;
    if(entry->name[0] == '.'){
        entry->hidden = 0;
    } else {
        entry->hidden = 1;
    }
    free(test);
}

int comparator(const void *p1, const void *p2){
    const fileEntry **a = p1;
    //const fileEntry * b = (fileEntry * )p2;
    printf("%s name\n",(*a)->name);
    //printf("%s name\n", b->name);
    printf("%s name \n", ((fileEntry *)p2)->name);
    return 0;
}

int main(int argc, char *argv[])
{
    int shower = 0;
    bool string = false;
    char identifier[8];
    for(int k = 1; k < argc; k++){
        if(strcmp(argv[k], "-a")){
            shower = 1;
        }
        if(strcmp(argv[k], "-A")){
            shower = 2;
        }
        if(strstr(argv[k], "--show=")){
            int in1 = 7;
            int in2 = 0;
            string = true;
            while(argv[k][in1]){
                switch(argv[k][in1]){
                    case 'R':
                        identifier[in2] = 'R';
                        in2++;
                        break;
                    case 'L':
                        identifier[in2] = 'L';
                        in2++;
                        break;
                    case 'U':
                        identifier[in2] = 'U';
                        in2++;
                        break;
                    case 'G':
                        identifier[in2] = 'G';
                        in2++;
                        break;
                    case 'S':
                        identifier[in2] = 'S';
                        in2++;
                        break;
                    case 'M':
                        identifier[in2] = 'M';
                        in2++;
                        break;
                    case 'N':
                        identifier[in2] = 'N';
                        in2++;
                        break;
                    case 'I':
                        identifier[in2] = 'I';
                        in2++;
                        break;
                    default:
                        fprintf(stderr, "wrong identifier");
                        return 45;
                }
                in1++;
            }
        }
    }
    if(!string){
        identifier[0] = 'R'; identifier[1] = 'L'; identifier[2] = 'U'; identifier[3] = 'G';
        identifier[4] = 'S'; identifier[5] = 'M'; identifier[6] = 'N';
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        fprintf(stderr, "Cannot open current directory");
        return 45;
    }
    DIR *dir = opendir(cwd);
    struct dirent *dirEntry = NULL;
    fileEntry *holder[500];
    struct maxSizes *sizes = malloc(sizeof(struct maxSizes));
    int entries = 0;
    while ((dirEntry = readdir(dir)) != NULL){
        fileEntry *entry = malloc(sizeof(fileEntry));
        readEntry(dirEntry, entry);
        holder[entries] = entry;
        entries++;
    }
    //printf("Entries: %d", entries);
    qsort(&holder, entries, sizeof(fileEntry), comparator);
    getMaxSizes(holder, entries, sizes);
    for(int j = 0; j < entries; j++){
        printID(holder[j], sizes, identifier, shower);
    }
    for(int j = 0; j < entries; j++){
        free(holder[j]);
    }
    free(sizes);
    return 0;
}



