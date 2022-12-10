#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*
  Function Declarations for builtin shell commands:
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("huahua's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}

//lsh_launch
//int execvp(const char *file, char *const argv[]);
int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);  //wait for the process’s state to change
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

//Parsing the line
//char *strtok(char *str, const char *delim) //search it
#define LSH_TOK_BUFSIZE 64        //buffer 
#define LSH_TOK_DELIM " \t\r\n\a"    
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));  //pointer array
  char *token;

  if (!tokens) {                    //check error
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);     //input , in " \t\r\n\a" and get first child string
  while (token != NULL) {
    tokens[position] = token;             //the rest of child string
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;      //expand
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {            //null
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM); //break while
  }
  tokens[position] = NULL;   //null end
  return tokens;        //get a pointer array and it's store every child string's pointer  
}

//lsh_read_line
#define LSH_RL_BUFSIZE 1024        //buf size
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;                //position 
  char *buffer = malloc(sizeof(char) * bufsize);    //malloc buffer
  int c;     //get char ascii,EOF is an integer, not a character

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");      //check error
    exit(EXIT_FAILURE);
  }

  while (1) {           //loop
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;           //expand buffer
      buffer = realloc(buffer, bufsize);   //false return null
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}
/*
//another methods in using getline
char *lsh_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}
*/


//lsh_loop
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("#>>> ");                         // print a prompt
    line = lsh_read_line();               // call a function to read a line
    args = lsh_split_line(line);          // call a function to split the line into args
    status = lsh_execute(args);           // execute the args

    free(line);                           // free the line and arguments that we created earlier.
    free(args);
  } while (status);                       // using a status variable returned by lsh_execute() to determine when to exit.
}

//main
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();       

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}