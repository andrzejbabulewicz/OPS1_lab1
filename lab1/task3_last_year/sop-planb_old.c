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
#include <sys/stat.h>
#include <regex.h>
#include <ctype.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_GROUPS 20
#define MAX_PATH 101
#define MAX_COMMAND 10

int groups_count[MAX_GROUPS + 1];

void usage(char* name)
{
    fprintf(stderr, "USAGE: %s path\n", name);
    exit(EXIT_FAILURE);
}



void get_groups_count(const char* dirpath, int counts[MAX_GROUPS + 1]) {
    DIR* dir = opendir(dirpath);
    if (dir == NULL) {
        ERR("opendir");
    }

    struct dirent* entry;
    regex_t regex;
    int ret;

    // Compile the regular expression pattern
    ret = regcomp(&regex, "^grupa([0-9]+)$", REG_EXTENDED);
    if (ret != 0) {
        char error_message[100];
        regerror(ret, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex compilation failed: %s\n", error_message);
        exit(EXIT_FAILURE);
    }

    // Iterate over the directory entries
    while ((entry = readdir(dir)) != NULL) {
        char filepath[MAX_PATH * 2]; // Increase buffer size to avoid truncation
        strcpy(filepath, dirpath);
        strcat(filepath, "/");
        strcat(filepath, entry->d_name);

        struct stat file_stat;
        if (stat(filepath, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISREG(file_stat.st_mode)) { // Check if it's a regular file
            // Match the file name against the regular expression
            regmatch_t matches[2];
            ret = regexec(&regex, entry->d_name, 2, matches, 0);
            if (ret == 0) {
                // Extract the group number from the file name
                char group_number[10];
                int length = matches[1].rm_eo - matches[1].rm_so;
                strncpy(group_number, entry->d_name + matches[1].rm_so, length);
                group_number[length] = '\0';

                // Open the file
                FILE* file = fopen(filepath, "r");
                if (file == NULL) {
                    ERR("fopen");
                }

                // Count the number of lines (members) in the file
                int member_count = 0;
                char buffer[MAX_PATH];
                while (fgets(buffer, sizeof(buffer), file) != NULL) {
                    member_count++;
                }

                if(buffer[0]=='-')
                {
                    printf("%s: link encountered\n", entry->d_name);
                }
                else
                {
                    counts[atoi(group_number)] = atoi(buffer);
                    printf("Group %s has %s members\n", group_number, buffer);
                    printf("just to check: %d\n", counts[atoi(group_number)]);
                }
                // Print the result
                

                // Close the file
                fclose(file);
            }
        }
    }

    // Clean up
    closedir(dir);
    regfree(&regex);
}

void process_file(const char* filepath, int group_count) 
{
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Determine the file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("lseek");
        close(fd);
        exit(EXIT_FAILURE);
    }
    lseek(fd, 0, SEEK_SET);

    // Calculate the number of grades
    int num_grades = file_size / 4;
    if (file_size % 4 != 0) {
        fprintf(stderr, "File size is not a multiple of 4 bytes\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Allocate buffers for each grade
    struct iovec* iov = malloc(num_grades * sizeof(struct iovec));
    if (iov == NULL) {
        perror("malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int* grades = malloc(num_grades * sizeof(int));
    if (grades == NULL) {
        perror("malloc");
        free(iov);
        close(fd);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_grades; i++) {
        iov[i].iov_base = &grades[i];
        iov[i].iov_len = sizeof(int);
    }

    // Read the grades using readv
    ssize_t bytes_read = readv(fd, iov, num_grades);
    if (bytes_read == -1) {
        perror("readv");
        free(iov);
        free(grades);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Print the grades
    for (int i = 0; i < num_grades; i++) {
        printf("Student %d: %d\n", i, grades[i]);
    }

    // Clean up
    free(iov);
    free(grades);
    close(fd);
}



int type_file(const char* path) //1 - file, 0- directory, -1 - error, 2-link
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
    else if (S_ISLNK(path_stat.st_mode)) 
    {
        printf("The path leads to a link.\n");
        return 2;
    }   
    

    return -1;
}

int is_valid_filename(const char* filename) {
    // Check if the filename starts with 'z'
    if (filename[0] != 'z') return 0;

    // Check if there is at least one digit after 'z'
    int i = 1;
    while (isdigit(filename[i])) i++;
    if (i == 1) return 0; // No digits after 'z'

    // Check if the next character is 'g'
    if (filename[i] != 'g') return 0;
    i++;

    // Check if there is at least one digit after 'g'
    if (!isdigit(filename[i])) return 0;

    // Check if the rest of the string are digits
    while (isdigit(filename[i])) i++;

    // Ensure the filename ends after the digits
    return filename[i] == '\0';
}

int callback(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    // Check if the current item is a regular file
    if (typeflag == FTW_F) {
        // Extract the filename from the path
        const char* filename = fpath + ftwbuf->base;

        // Check if the filename matches the pattern zMgN
        if (is_valid_filename(filename)) {
            // Call the process_file function
            printf("%s\n", fpath);
            process_file(fpath, *groups_count);
        }
    }
    return 0; // Continue the tree walk
}

void batch_process(const char* dirpath) {
    printf("Przetworzenie ocen w drzewie katalogow\n");

    // Use nftw to traverse the directory tree
    if (nftw(dirpath, callback, 20, FTW_PHYS) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) 
{
    if (argc != 2) {
        ERR("path");
        return EXIT_FAILURE;
    }

    struct stat path_stat;
    if (stat(argv[1], &path_stat) == -1) 
    {
        ERR("stat");
        return EXIT_FAILURE;
    }
    if (S_ISLNK(path_stat.st_mode)) 
    {
        printf("The path leads to a link.\n");
    }   

    int pathtype = type_file(argv[1]);

    if(pathtype==2)
    {
        printf("Link encountered\n");
    }

    char command[MAX_COMMAND];

    while(1)
    {
        scanf("%s", command);
        printf("we are after scanning\n");
        if(strcmp(command, "groups") == 0)
        {
            if(pathtype == 0)
            {
                get_groups_count(argv[1], groups_count);
            }
            else
            {
                ERR("path doesn't lead to a directory");
            }
        }
        else if(strcmp(command, "process") == 0)
        {
            if(pathtype == 1)
            {
                process_file(argv[1], *groups_count);
            }
            else
            {
                ERR("path doesn't lead to a file");
            }
        }
        else if(strcmp(command, "batch") == 0)
        {
            if(pathtype == 0)
            {
                batch_process(argv[1]);
            }
            else
            {
                ERR("path doesn't lead to a directory");
            }
        }        
        else
        {
            ERR("Unknown command");
        }
    }   

    

    return EXIT_SUCCESS; 
}
