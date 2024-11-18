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

#define MAX_PATH 101

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
    ({ long int __result; \
       do __result = (long int) (expression); \
       while (__result == -1L && errno == EINTR); \
       __result; })
#endif

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int depth=1; //----------- Can it be done without global variables?----------------------
int e_indicator = 0;
int o_indicator = 0;
char ext[5]="";
FILE *output_file = NULL;
//---------------------------------------------------------------------

void usage(char* name)
{
    fprintf(stderr, "USAGE: %s path\n", name);
    exit(EXIT_FAILURE);
}


int browse_dir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    if(ftwbuf->level<=depth)
    {
        switch (typeflag)
        {
            case FTW_DNR:
            case FTW_D:
                fprintf(output_file, "path: %s\n", fpath);                       
                break;
            case FTW_F:
                if(e_indicator==1)
                {
                    if(strstr(fpath, ext)!=NULL)
                    {
                        fprintf(output_file, "%s %ld\n", basename((char *)fpath), sb->st_size);
                    }
                }
                else
                fprintf(output_file, "%s %ld\n", basename((char *)fpath), sb->st_size);
                break;
        }
    // Print the name of the file (basename) and the size of the file
    }
    
    return 0;
}



int main(int argc, char **argv)
{
    
    
    for(int i=1;i<argc;i++)
    {
        if(strcmp(argv[i],"-d")==0)
        {
            depth=atoi(argv[i+1]);
            printf("depth: %d\n", depth);
        }
        if(strcmp(argv[i],"-e")==0)
        {
            e_indicator=1;
            strcpy(ext, argv[i+1]);
        }
        if(strcmp(argv[i],"-o")==0)
        {
            o_indicator=1;
        }
    }

    if (o_indicator)
    {
        char *output_filename = getenv("L1_OUTPUTFILE");
        if (output_filename && strlen(output_filename) > 0)
        {
            output_file = fopen(output_filename, "w");
            if (!output_file)
            {
                perror("fopen");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            output_file = stdout;
        }
    }
    else
    {
        output_file = stdout;
    }

    for (int i = 1; i < argc; i++) 
    {
        //printf("iteration %d, argument %s\n", i, argv[i]);
        if (strcmp(argv[i], "-p") == 0) 
        {
            //printf("flag p encountered\n");
            if (nftw(argv[i+1], browse_dir, 20, FTW_PHYS) == -1)
            {
                perror("nftw");
                exit(EXIT_FAILURE);
            }
        } 
    }

    if (output_file != stdout)
    {
        fclose(output_file);
    }

    return EXIT_SUCCESS;
}