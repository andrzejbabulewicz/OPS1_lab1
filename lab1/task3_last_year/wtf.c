
void write_to_file(const char* path)
{
    char buffer[100];
    
    FILE* file = fopen(path, "w+");
    if (file == NULL)
    {
        ERR("Error opening the file");
    }
    printf("Podaj dane do zapisania: ");
    while(1)
    {
        buffer[0] = '\0';
        fscanf(stdin, "%s", buffer);
        fprintf(file, "%s\n", buffer);
        printf("buffer: %s\n", buffer);
        if (buffer[0] == '\n')
        {
            break;
        }

    }
    fclose(file);
}

//-----------copilot and gpt---------------
void write_to_file(const char* path)
{
    char buffer[100];
    
    FILE* file = fopen(path, "w+");
    if (file == NULL)
    {
        perror("Error opening the file");
        return;
    }
    printf("Podaj dane do zapisania: ");
    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            perror("Error reading input");
            break;
        }
        // Remove the newline character if present
        buffer[strcspn(buffer, "\n")] = 0;
        if (buffer[0] == '\0')
        {
            break;
        }
        fprintf(file, "%s\n", buffer);
        printf("buffer: %s\n", buffer);
    }
    fclose(file);
}