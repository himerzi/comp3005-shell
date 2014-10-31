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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

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

void print_string_array(char** str_array) {
    int i = 0;
    while(str_array[i] != NULL) {
        printf("tokens %u: %s\n", i, str_array[i]);
        i++;
    }
}

void free_string_array(char** str_array) {
    int i = 0;
    while (str_array[i] != NULL) {
        free(str_array[i]);
        i++;
    }
    free(str_array);
    
    
}
char ** init_string_array(int array_size) {
    //maximum of 2 entries in this array, one for HOME one for PATH
    //char **file_strings = malloc(array_size * sizeof *file_strings);
    char **file_strings = calloc (array_size+1, sizeof (char*));
    int i;
    for (i = 0; i < array_size; i++) {
        //arbitrary max string size of 300 - could be done dynamically
        file_strings[i] = (char*) malloc(600);
        //enable this later
      //  strncpy(file_strings[i], "", (size_t) strlen("")+1);
       // strlcpy(file_strings[i], NULL, sizeof(file_strings[i]));
        //should init strings to NULL
        //printf("malloc: %u %p\n", i, file_strings[i]);
        if (file_strings[i] == NULL) {
            printf("HERP\n");
        }
    }
    //set last entry to null
   // char *nil = "\0";
    //strlcpy(file_strings[i], nil, 600);
    file_strings[array_size] = (char*)NULL;
    return file_strings;
}
char **  read_profile() {
    FILE *file;
    char * cwd = my_getcwd();
    char *filename = strncat(cwd, "/profile", sizeof("/profile"));
    char **file_strings = init_string_array(2);
    //we need to make sure path gets assigned to 0, and home to 1. program works on that assumption
    //should also raise error if either of home or path are missing.
    /* open the file for writing */
    file = fopen(filename, "r"); if (file == NULL) {
        fprintf(stderr, "File %s could not be opened\n", filename);
        exit(1);
    }
    
    int i = 0;
    while (!feof(file) && i < 3) {
        
        if (fscanf(file, "%s", file_strings[i]) != 1)
            break;
        i++;
    }
    
    /* close the file */
    fclose(file);
    //free cwd
    free(cwd);
    return file_strings;
}

char * remove_trailing_newline(char *string) {
    //remove trailing newline
    size_t len = strlen(string) - 1;
    if (string[len] == '\n')
        string[len] = '\0';
    return string;
}
/*   parse commandline for space separated commands */
char ** parse(char *user_input, char *delim) {
  
    char **tokens;
    char **temp_buffer = init_string_array(20);
    char *word;
    int i = 0;
    
    for (word = strtok(user_input, delim); word; word = strtok(NULL, delim))
    {
        //strncpy(tokens[i], word, (size_t) strlen(word)+1);
        strncpy(temp_buffer[i], word, (size_t) strlen(word)+1);

        i++;
    }
    //take them from temporary into allocated array
    tokens = init_string_array(i);
    for (int k = 0; k < i; k++)
    {
        strncpy(tokens[k], temp_buffer[k], (size_t) strlen(temp_buffer[k])+1);
    }
    //buggy, array should be of correct size. as it is now we have dangling unusued memory
    //tokens[i] = NULL;
    free_string_array(temp_buffer);
    return tokens;
}

int fork_exec(char *args[]) {
    
    int status;
    pid_t pid;
    
    pid = fork();
    
    if (pid == 0)
    {
        /* This is the child process. */
        //_exit (EXIT_FAILURE);
    
    
        //printf ("this is the child process, with id %d, path: %s, home: %s\n", (int) getpid (), getenv("PATH"), getenv("HOME"));
        execv (args[0], args);
        /* The execvp function returns only if an error occurs. */
        perror("execv() error");
        
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

//opendir() to open a directory and readdir() to read an entry from it. You can use these to figure out which directory the executable program is in. Alternatively you might use stat() to test each directory in turn to see which one contains the program.
int find_in_path(char * prog_name, char * path_env) {
    char dirname[160];
    strncpy(dirname, path_env, (size_t) strlen(path_env)+1);
    struct dirent *dp;
    DIR *dirp = opendir(dirname);
    if (dirp == NULL)
        return -1;
    size_t len = strlen(prog_name);
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_namlen == len && strcmp(dp->d_name, prog_name) == 0) {
            (void)closedir(dirp);
            return 1;
        }
    }
    (void)closedir(dirp);
    return 0;
}

int execute(char *args[], char *env[]) {
    //char *path_vars;
    size_t path_string_size = (strlen(env[0]) - 4);
    char stripped_path[path_string_size];
   
    strncpy( stripped_path, &env[0][5], path_string_size);
    
    char **path_vars = parse((char*)stripped_path, ":");
    
    int i = 0;
    while (path_vars[i] != NULL){
        if(find_in_path(args[0], path_vars[i]))
            break;
        i++;
    }
    if(path_vars[i] == NULL){
        i = -1;
        return i;
    }
    
    //255 is name file length limit
    int path_len = 256;
    char path_copy[path_len];
    (void)strncpy(path_copy, path_vars[i], path_len-1);
    (void)strncat(path_copy, "/" , path_len-1);
    (void)strncat(path_copy , args[0], path_len-1);
    strncpy( args[0], path_copy, strlen(path_copy)+1);
    
    //free(args[0]);
    //args[0] = path_copy;
    
    int status = fork_exec(args);
    free_string_array(path_vars);
    return status;
}



void change_working_dir(char * path) {
    chdir(path);
}

char * clean_input(char * input) {
    size_t len = strlen(input);

    return remove_trailing_newline(input);
    
   //
}
int main(int argc, const char * argv[]) {
    
    
    char * cwd = my_getcwd();
    char **profile = read_profile();

    int child_status;
    //init code
    
    //print_string_array(profile, 2);
    while(1) {
        cwd = my_getcwd();
        
        print_string_array(profile);

        //this 80 char should be consistent with allocated string lenths in init str array.
        char input[80];
        
        printf("%s >", cwd);
        fgets(input, 80, stdin);
        
        char **parsed_input = parse(clean_input(input), " ");
        if (! strcmp (parsed_input[0], "cd")) {
            //should change to home by default...
            if(parsed_input[1] == NULL){
                change_working_dir(&profile[1][5]);
            }
            else{
                change_working_dir(parsed_input[1]);
            }
            
        }
        else{
           child_status = execute(parsed_input, profile);//
        }
        
        //print_string_array(parsed_input, 20);
        //20 should be stored as a global constant.
        free_string_array(parsed_input);
        
        free(cwd);
        
        //char *in_line;
        //in_line = readline(">");
        
    }
    //free_string_array(profile, 2);
    return 0;
}



