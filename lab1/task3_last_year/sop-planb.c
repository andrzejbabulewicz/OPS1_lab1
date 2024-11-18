#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <libgen.h>

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
    ({ long int __result; \
       do __result = (long int) (expression); \
       while (__result == -1L && errno == EINTR); \
       __result; })
#endif

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_GROUPS 20
#define MAX_PATH 100
#define MAX_LINE 50
#define FILE_BUF_LEN 256

int groups_count[MAX_GROUPS + 1];

void usage(char* name)
{
    fprintf(stderr, "USAGE: %s path\n", name);
    exit(EXIT_FAILURE);
}

int get_depth(struct FTW *ftwbuf)
{
    return ftwbuf->level;
}

int display_info(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    switch (typeflag)
    {
        case FTW_DNR:
        case FTW_D:
            for(int i=0;i<get_depth(ftwbuf);i++)
            {
                printf(" ");
            }
            printf("+");
            break;
        case FTW_F:
            for(int i=0;i<get_depth(ftwbuf);i++)
            {
                printf(" ");
            }
            printf(" ");
            break;
    }
    // Print the name of the file (basename) and the depth of the search
    printf("%s\n", basename((char *)fpath));
    return 0; // Continue the tree walk
}

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;  // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void scan_dir()
{
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;

    dir = opendir(".");
    if (dir == NULL)
    {
        ERR("opendir");
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (lstat(entry->d_name, &filestat))
        {
            printf("oops\n");
            perror("stat");
            continue;
        }      
        else if (S_ISREG(filestat.st_mode)) 
        {
            printf("%s\n", entry->d_name);
        }         
    }


    //printf("total size: %d\n", total_size);
    closedir(dir);

    //return total_size;
}

void file_handle(const char* path)
{
    const int fd = open(path, O_RDONLY);
    if (fd == -1)
        ERR("open");
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) 
    {
        ERR("fstat");
    }

    char file_buf[FILE_BUF_LEN];
    for (;;)
    {
        const ssize_t read_size = bulk_read(fd, file_buf, FILE_BUF_LEN);
        if (read_size == -1)
            ERR("bulk_read");

        if (read_size == 0)
            break;
        printf("%s", file_buf);
    }

    printf("File size: %ld\n", file_stat.st_size);
    printf("User ID of the owner: %d\n", file_stat.st_uid);
    printf("Group ID of the owner: %d\n", file_stat.st_gid);

}

int type_file(const char* path) //1 - file, 0- directory, -1 - error, 2-unknown
{
    struct stat path_stat;
    if (stat(path, &path_stat) == -1) 
    {
        ERR("stat");
        return EXIT_FAILURE;
    }
    if (S_ISREG(path_stat.st_mode)) 
    {
        printf("The path leads to a file.\n");
        return 1;
    }

    else if (S_ISDIR(path_stat.st_mode)) 
    {
        printf("The path leads to a directory.\n");
        return 0;
    } 
    else
    {
        printf("Unknown type of file\n");
        return 2;
    }   
    

    return -1;
}

void write_to_file(const char* path)
{
    char buffer[100];
    //char character='\0';
    FILE* file = fopen(path, "w+");
    printf("opened\n");
    if (file == NULL)
    {
        ERR("Error opening the file");
    }
    int sth=0;
    int sth1=0;
    while(1)
    {
        fgets(buffer, sizeof(buffer), stdin);
        if (strcmp(buffer, "\n") == 0)
        {
            if(sth==0)
            {
                sth=1;
            }
            else
            {
                break;
            }
        }

        if(sth1==0)
        {
            sth1=1;

        }
        else
        {
            fprintf(file, "%s", buffer);
        }
    }
    fclose(file);
}

void _write(const char* filepath, int counts[MAX_GROUPS + 1]) 
{ 
    printf("write\n");
    struct stat file_stat;
    printf("1\n");
    if (stat(filepath, &file_stat) == -1) 
    {
        printf("2\n");
        write_to_file(filepath);
    }


    if (S_ISREG(file_stat.st_mode))
    {
        printf("3\n");
        if (remove(filepath) == 0)
        {
            printf("File deleted successfully.\n");
        }
        else
        {
            ERR("Error deleting the file");
        }
        write_to_file(filepath);
    }
}


void _show(const char* filepath, int group_count) 
{ 
    printf("show\n");

    int type_of_file=type_file(filepath);
    char path[MAX_PATH];

    switch(type_of_file)
    {
        case 0: //dir
        
        if (getcwd(path, MAX_PATH) == NULL)
        ERR("getcwd");
        chdir(filepath);
        scan_dir();
        chdir(path);
        break;

        case 1: //file
        file_handle(filepath);
        break;

        case 2: //unknown

        break;
    }
}

void _walk(const char* dirpath)
{  
    printf("walk\n"); 
    if (nftw(dirpath, display_info, 20, 0) == -1)
    {
        perror("nftw");
        exit(EXIT_FAILURE);
    }

}

void leave()
{
    printf("Zakonczenie programu\n");
    exit(EXIT_SUCCESS);
}



int main(int argc, char** argv) 
{ 
    char path[MAX_PATH]="";
    char choice;
    printf("A. Write\nB. Show\nC. Walk\nD. Exit\n");
    choice=getchar();
    if(choice<'A' || choice>'D') 
    {
        choice+='A'-'a';
    }
    switch (choice)
    {
    case 'A':
        printf("path:");
        scanf("%s", path);
        _write(path, groups_count);
        break;
    case 'B':
        printf("path:");
        scanf("%s", path);
        _show(path, groups_count[0]);
        break;
    case 'C':
        printf("path:");
        scanf("%s", path);
        _walk(path);
        break;
    case 'D':
        leave();
        break;
    default:
        ERR("Invalid choice");
        break;
    }
    
    return EXIT_SUCCESS; 
}
