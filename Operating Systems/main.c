//
//  main.c
//  Operating Systems
//
//  Created by Michael Detmold on 27/10/2014.
//  Copyright (c) 2014 Michael Detmold. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
//execve
#include <unistd.h>

char* my_getcwd ()
{
    int size = 100;
    char *buffer = (char *) malloc (size);
    if (getcwd(buffer, size) == NULL)
        perror("getcwd() error");
    //printf("curdir %s\n", buffer);
    return buffer;
}

/* used for debugging*/

void print_string_array(char** str_array, int size) {
    for (int i = 0; i<size; i++) {
        printf("tokens %u: %s\n", i, str_array[i]);
    }
}

void free_string_array(char** str_array, int size) {
    //free(str_array);
    for (int i = 0; i < size; i++) {
        //printf("freeing %u = %s\n", i, str_array[i]);
        free(str_array[i]);
    }
    free(str_array);
    
    
}
char ** init_string_array(int array_size) {
    //maximum of 2 entries in this array, one for HOME one for PATH
    //char **file_strings = malloc(array_size * sizeof *file_strings);
    char **file_strings = calloc (array_size, sizeof (char*));
    int i;
    for (i = 0; i < array_size-1; i++) {
        //arbitrary max string size of 300 - could be done dynamically
        file_strings[i] = (char*) malloc(300 * sizeof(char));
       // strlcpy(file_strings[i], NULL, sizeof(file_strings[i]));
        //should init strings to NULL
        //printf("malloc: %u %p\n", i, file_strings[i]);
        if (file_strings[i] == NULL) {
            printf("HERP\n");
        }
    }
    //set last entry to null
    file_strings[array_size] = NULL;
    return file_strings;
}
char **  read_profile() {
    FILE *file;
    char *filename = "/Users/md/Documents/UCL 3/3005 OS/cwk/Operating Systems/profile";
    char **file_strings = init_string_array(3);
    
    /* open the file for writing */
    file = fopen(filename, "r"); if (file == NULL) {
        fprintf(stderr, "File %s could not be opened\n", filename);
        exit(1);
    }
    
    int i = 0;
    while (!feof(file)) {
        
        if (fscanf(file, "%s", file_strings[i]) != 1)
            break;
        i++;
    }
    
    /* close the file */
    fclose(file);
    return file_strings;
}

void remove_trailing_newline(char *string) {
    //remove trailing newline
    size_t len = strlen(string) - 1;
    if (string[len] == '\n')
        string[len] = '\0';
}
/*   parse commandline for space separated commands */
char ** parse(char *user_input) {
  
    char **tokens = init_string_array(20);
    char *delim = " ";
    char *word;
    int i = 0;
    
    remove_trailing_newline(user_input);
    
    for (word = strtok(user_input, delim); word; word = strtok(NULL, delim))
    {
        strlcpy(tokens[i], word, sizeof(tokens[i]));
        i++;
    }
    //buggy, array should be of correct size. as it is now we have dangling unusued memory
    tokens[i] = NULL;
    return tokens;
}
int execute(char *args[], char *env[]) {
    int status;
    pid_t pid;
    
    pid = fork();
    if (pid == 0)
    {
        /* This is the child process. */
        //_exit (EXIT_FAILURE);
        putenv(env[0]);//path
        putenv(env[1]);//home
        printf ("this is the child process, with id %d, path: %s, home: %s\n", (int) getpid (), getenv("PATH"), getenv("HOME"));
        execvp (args[0], args);
        /* The execvp function returns only if an error occurs. */
        perror("execvp() error");
        
        abort ();
    }
    else if (pid < 0)
    /* The fork failed.  Report failure.  */
        status = -1;
    else
    /* This is the parent process.  Wait for the child to complete.  */
        if (waitpid (pid, &status, 0) != pid)
            status = -1;
    return status;
    

}

int main(int argc, const char * argv[]) {
    
    
    //gnu_getcwd();
    char **profile = read_profile();
    print_string_array(profile, 2);
    int child_status;
    char* cwd = my_getcwd();
    //print_string_array(profile, 2);
    while(1) {
        //this 80 char should be consistent with allocated string lenths in init str array.
        char input[80];
        
        printf("%s >", cwd);
        fgets(input, 80, stdin);
        printf("input: %s\n", input);
        char **parsed_input = parse(input);
        child_status = execute(parsed_input, profile);
        //print_string_array(parsed_input, 20);
        //20 should be stored as a global constant.
        free_string_array(parsed_input, 20);
        
        //char *in_line;
        //in_line = readline(">");
        
    }
    //free_string_array(profile, 2);
    return 0;
}



