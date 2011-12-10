#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

char sequence[] = ">1UBI:A|PDBID|CHAIN|SEQUENCE\nMQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQRLIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG";

int main(int argc, char* argv[]) {
  int descriptors[2];
  pipe(descriptors);

  pid_t pid = fork();

  if (pid == 0) {  // child process
    // Replace stdin with the input part of the pipe
    dup2(descriptors[0], 0);

    // Close the unused half of the pipe
    close(descriptors[1]);

    char executable[] = "/usr/local/bin/phmmer";
    char database[] = "/home/hmmer/data/uniprot_sprot.fasta";
    char source[] = "-";

    char* const args[] = { executable, source, database, NULL };
    execvp(executable, args);
    exit(0);
  } else {  // parent process
    // Replace stdout with the output part of the pipe
    dup2(descriptors[1], 1);

    // Close the unused part of the pipe
    close(descriptors[0]);

    char executable[] = "/bin/echo";
    char* const args[] = { executable, sequence, NULL };
    execvp(executable, args);

    int status;
    wait(&status);
    close(descriptors[0]);
  }
  
  return 0;
}
