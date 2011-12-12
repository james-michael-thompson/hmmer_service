#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>
#include <sstream>

#include "hmmer.pb.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <zmq.hpp>

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

void string_tokenize(
  std::string const & str,
  std::vector< std::string >* lines
) {
  std::stringstream ss(str);
  std::istream_iterator<std::string> it(ss);
  std::istream_iterator<std::string> end;
  while ( it != end ) {
    lines->push_back(*it);
    ++it;
  }
}

double string_to_double(
  std::string const & str
) {
  return std::strtod(str.c_str(),0);
}

std::string remove_newlines( std::string const & str ) {
  std::string copy = str;
  copy.erase(std::remove(copy.begin(), copy.end(), '\n'), copy.end());
  return copy;
}

int string_to_int(
  std::string const & str
) {
  return std::strtol(str.c_str(),0,0);
}

void parse_hmmer_output( const vector<string> & lines, HMMER_Response* resp ) {
  //HMMER_Response resp;
  static string const prefix = "  == domain";
  static string const finished_tag = "Internal pipeline statistics summary:";

  bool parsing = false;
  HMMER_Response_Alignment* current_aln = NULL;
  for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
    if (i->substr(0,prefix.size()) == prefix) {
    current_aln = resp->add_alignments();
      // get the e-value and template_id from this line
    vector<string> tokens;
    string_tokenize(*i,&tokens);

    double const evalue(string_to_double(tokens[8]));
    double log_evalue = -1;
    if ( evalue > 0 ) log_evalue = std::log(evalue);
    current_aln->set_log_evalue(log_evalue);
    current_aln->set_bit_score(string_to_double(tokens[4]));

    parsing = true;
    } else if ( i->substr(0,finished_tag.size()) == finished_tag ) {
      break; // finished parsing
    } else if ( i->substr(0,2) == ">>" || remove_newlines(*i).length() == 0 ) {
      parsing = false;
    } else if ( parsing ) {
      string query_line = remove_newlines(*i); ++i;
      string cons_line  = remove_newlines(*i); ++i;
      string templ_line = remove_newlines(*i); ++i;
      string prob_line  = remove_newlines(*i); ++i;
  
      vector<string> tokens;
      string_tokenize(query_line,&tokens);
      //if ( tokens.size() != 4 ) {
      //  cout << "have " << tokens.size() << " tokens." << endl;
      //  cout << query_line << endl;
      //}
      assert( tokens.size() == 4 );
      // 0 -> id
      // 1 -> start
      // 2 -> sequence
      // 3 -> stop
      if ( current_aln->query_start() == 0 ) {
        current_aln->set_query_start(string_to_int(tokens[1]));
      }
      current_aln->set_aligned_query_seq( current_aln->aligned_query_seq() + tokens[2] );
  
      tokens.clear();
      string_tokenize(templ_line,&tokens);
      assert( tokens.size() == 4 );
      current_aln->set_template_id(tokens[0]);
      if ( current_aln->template_start() == 0 ) {
        current_aln->set_template_start(string_to_int(tokens[1]));
      }
      current_aln->set_aligned_template_seq( current_aln->aligned_template_seq() + tokens[2] );
    }
  } // lines

  cout << "parsed " << resp->alignments_size() << " alignments." << endl;
} // parse_hmmer_output

int main(int argc, char* argv[]) {
  vector<string> lines;
  capture_output("phmmer input/1ubi.fasta ../data/pdb_seqres.txt", &lines);

  HMMER_Response resp;
  parse_hmmer_output(lines,&resp);
  return 0;
}
