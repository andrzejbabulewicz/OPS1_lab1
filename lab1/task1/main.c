#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_PATH 101

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int scan_dir()
{
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;

    dir = opendir(".");
    if (dir == NULL)
    {
        ERR("opendir");
    }

    int total_size = 0;


    while ((entry = readdir(dir)) != NULL)
    {
        if (stat(entry->d_name, &filestat) == -1)
        {
            perror("stat");
            continue;
        }        
            //printf("%s %ld\n", entry->d_name, filestat.st_size);
            total_size+=filestat.st_size;        
    }


    //printf("total size: %d\n", total_size);
    closedir(dir);

    return total_size;
}



int main(int argc, char **argv)
{
    int fd = open("result.txt", O_WRONLY | O_APPEND | O_CREAT, 0777);
    char path[MAX_PATH]; //current path
    int max_bytes=0;
    int current_size=0;
    if (getcwd(path, MAX_PATH) == NULL)
        ERR("getcwd");
    for (int i = 1; i < argc; i+=2)
    {
        if (chdir(argv[i]))
            printf("Can't access folder: %s", argv[i]);
        //printf("%s:\n", argv[i]);
        max_bytes=atoi(argv[i+1]);
        current_size=scan_dir();
        if(current_size>max_bytes)
        {
            printf("%s\n", argv[i]);
            write(fd, argv[i], 1000);
        }
        if (chdir(path))
            ERR("chdir");
    }
    return EXIT_SUCCESS;
}