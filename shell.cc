#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>

using namespace std;

void capture_output(const string& cmd, vector<string>* lines) {
  FILE* stream = popen(cmd.c_str(), "r");

  char* line = NULL;
  size_t len = 0;
  ssize_t retval;

  while ((retval = getline(&line, &len, stream)) != -1) {
    lines->push_back(string(line));
  }

  free(line);
  pclose(stream);  
}

void parse_hmmer_output( const vector<string> & lines ) {

  static string const prefix("  == domain");
  for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
    if (i->substr(0,prefix.size()) == prefix) {
      cout << *i;
      // get the e-value and template_id from this line
    }
  }
}

int main(int argc, char* argv[]) {
  vector<string> lines;
  capture_output("phmmer input/1ubi.fasta /home/hmmer/data/uniprot_sprot.fasta", &lines);

  parse_hmmer_output(lines);
  //for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
  //  cout << *i;

  

  return 0;
}
