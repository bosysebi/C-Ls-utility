#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdint.h>


char *getcwd(char *buf, size_t size);
char *strdup (const char *s);

void mode_to_string(mode_t mode, char str[11])
{
    strcpy(str, "----------"); // 10 x -
    char *p = str;


    if (S_ISDIR(mode))
        *p = 'd';
    p++;

    // Owner

    if (mode & S_IRUSR)
        *p = 'r';
    p++;
    if (mode & S_IWUSR)
        *p = 'w';
    p++;
    if (mode & S_IXUSR)
        *p = 'x';
    p++;

    // Group

    if (mode & S_IRGRP)
        *p = 'r';
    p++;
    if (mode & S_IWGRP)
        *p = 'w';
    p++;
    if (mode & S_IXGRP)
        *p = 'x';
    p++;

    // Others

    if (mode & S_IROTH)
        *p = 'r';
    p++;
    if (mode & S_IWOTH)
        *p = 'w';
    p++;
    if (mode & S_IXOTH)
        *p = 'x';
    p++;

}

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
    if(shower == 1){
        if(strcmp(entry->name, ".")==0){
            return;
        }
        if(strcmp(entry->name, "..")==0){
            return;
        }
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

void getMaxSizes(fileEntry *entries[], int size, struct maxSizes *sizes, int mode){
    resetSizes(sizes);
    for(int i = 0; i < size; i++){
        if(mode == 0){
            if(entries[i]->name[0] != '.'){
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
        } else if(mode == 1){
            if(strcmp(entries[i]->name, "..") != 0){
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
        } else {
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
}

void getUserName(struct stat stat, fileEntry *entry){
    struct passwd *pwd;
    pwd = getpwuid(stat.st_uid);
    if(pwd==NULL){
        entry->username = NULL;
        return;
    }
    entry->username = strdup(pwd->pw_name);
}

void getGroupName(struct stat stat, fileEntry *entry){
    struct group *grp;
    grp = getgrgid(stat.st_gid);
    if(grp==NULL){
        entry->groupname = NULL;
        return;
    }
    entry->groupname = strdup(grp->gr_name);
}

void readEntry(struct dirent *dir, fileEntry *entry){
    struct stat test = {0};
    if(stat(dir->d_name, &test)!=0){
        return;
    }
    entry->filesize = test.st_size;
    getUserName(test, entry);
    getGroupName(test, entry);
    strftime(entry->lastmod, 64, "%b %e %Y %R", localtime(&test.st_mtime));
    entry->name = dir->d_name;
    mode_to_string(test.st_mode, entry->rights);
    entry->inode = (long) test.st_ino;
    entry->hardLinks = test.st_nlink;
}

int compare(const void *p1, const void *p2){
    fileEntry* a = * (fileEntry * * ) p1;
    fileEntry* b = * (fileEntry * * ) p2;
    if(a->name[0] != '.' && b->name[0] != '.'){
        int u = 0;
        while(a->name[u] && b->name[u]){
            char m = a->name[u];
            char n = b->name[u];
            if(a->name[u] < 91 && a->name[u] > 64 && b->name[u] < 123 && b->name[u] > 96){
                return -1;
            }
            if(b->name[u] < 91 && b->name[u] > 64 && a->name[u] < 123 && a->name[u] > 96){
                return 1;
            }
            if(m>n){
                return 1;
            }
            if(m<n){
                return -1;
            }
            u++;
        }
        if(strlen(a->name) > strlen(b->name)){
            return 1;
        }
        if(strlen(a->name) < strlen(b->name)){
            return -1;
        }
        return 0;
    }
    if(a->name[0] == '.' && b->name[0] != '.'){
        int u = 1;
        while(a->name[u] && b->name[u-1]){
            char m = a->name[u];
            char n = b->name[u-1];
            if(a->name[u] < 91 && a->name[u] > 64 && b->name[u-1] < 123 && b->name[u-1] > 96){
                return -1;
            }
            if(b->name[u-1] < 91 && b->name[u-1] > 64 && a->name[u] < 123 && a->name[u] > 96){
                return 1;
            }
            if(m>n){
                return 1;
            }
            if(m<n){
                return -1;
            }
            u++;
        }
        if(strlen(a->name) > strlen(b->name)){
            return 1;
        }
        if(strlen(a->name) < strlen(b->name)){
            return -1;
        }
        return -1;
    }
    if(a->name[0] != '.' && b->name[0] == '.'){
        int u = 1;
        while(a->name[u-1] && b->name[u]){
            char m = a->name[u-1];
            char n = b->name[u];
            if(a->name[u-1] < 91 && a->name[u-1] > 64 && b->name[u] < 123 && b->name[u] > 96){
                return -1;
            }
            if(b->name[u] < 91 && b->name[u] > 64 && a->name[u-1] < 123 && a->name[u-1] > 96){
                return 1;
            }
            if(m>n){
                return 1;
            }
            if(m<n){
                return -1;
            }
            u++;
        }
        if(strlen(a->name) > strlen(b->name)){
            return 1;
        }
        if(strlen(a->name) < strlen(b->name)){
            return -1;
        }
        return 1;
    }
    return strcmp(a->name, b->name);
}

int main(int argc, char *argv[])
{
    int shower = 0;
    bool string = false;
    char identifier[8] = {0};
    for(int k = 1; k < argc; k++){
        if(strcmp(argv[k], "-a")==0){
            shower = 2;
        }
        if(strcmp(argv[k], "-A")==0){
            shower = 1;
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
    if(!sizes){
        fprintf(stderr,"error");
        free(sizes);
        closedir(dir);
        return 45;
    }
    int entries = 0;
    while ((dirEntry = readdir(dir)) != NULL){
        fileEntry *entry = malloc(sizeof(fileEntry));
        if(!entry){
            for(int h = 0; h < entries; h++){
                free(holder[h]);
            }
            free(sizes);
            fprintf(stderr, "Failed to allocate");
            return 7;
        }
        readEntry(dirEntry, entry);
        holder[entries] = entry;
        entries++;
    }
    qsort(holder, entries, sizeof(fileEntry *), compare);
    getMaxSizes(holder, entries, sizes, shower);
    for(int j = 0; j < entries; j++){
        printID(holder[j], sizes, identifier, shower);
    }
    for(int j = 0; j < entries; j++){
        if(holder[j]->username) free(holder[j]->username);
        if(holder[j]->groupname) free(holder[j]->groupname);
        free(holder[j]);
    }
    free(sizes);
    closedir(dir);
    return 0;
}



