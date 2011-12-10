// C headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  pid_t pid = fork();

  if (pid == 0) {  // child process
    char executable[] = "/usr/local/bin/phmmer";
    char query[] = "/home/hmmer/devel/input/1ubi.fasta";
    char database[] = "/home/hmmer/data/uniprot_sprot.fasta";

    char* const args[] = { executable, query, database, NULL };
    execvp(executable, args);
    exit(0);
  } else {  // parent process
    int status;
    wait(&status);
  }
  
  return 0;
}
