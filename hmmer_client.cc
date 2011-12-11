#include "hmmer.pb.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <zmq.hpp>

DEFINE_string(send_socket, "tcp://localhost:8000", "Communication channel to server");
DEFINE_string(recv_socket, "tcp://*:8001", "Communication channel from server");
DEFINE_string(fasta, "", "File containing the query sequence in FASTA format");

using namespace std;

// Reads the contents of filename into contents, aborting if any errors occur
void read_sequence_or_die(const string& filename, HMMER_Request* req) {
  CHECK_NOTNULL(req);

  string sequence;
  string line;

  ifstream in(filename.c_str());
  while(in.good()) {
    getline(in, line);
    sequence += line + "\n";
  }

  req->set_fasta_sequence(sequence);
}

// Writes a HMMER_Request to the server's input queue
void send_request(const HMMER_Request& req, zmq::socket_t* sender) {
  CHECK_NOTNULL(sender);

  int num_bytes = req.ByteSize();

  string m;
  req.SerializeToString(&m);

  zmq::message_t msg(num_bytes);
  snprintf((char *) msg.data(), num_bytes, "%s", m.c_str());

  sender->send(msg);
}

// Receives a HMMER_Response from the server's output queue
void receive_response(HMMER_Response* resp, zmq::socket_t* receiver) {
  CHECK_NOTNULL(resp);
  CHECK_NOTNULL(receiver);

  zmq::message_t msg;
  receiver->recv(&msg);

  string m;
  istringstream iss(static_cast<char*>(msg.data()));
  iss >> m;

  resp->ParseFromString(m);
}

void emit(const HMMER_Response& resp) {
  cout << "job_id: " << resp.job_id() << endl;
  cout << "num_aln: " << resp.alignments_size() << endl;

  for (int i = 0; i < resp.alignments_size(); ++i) {
    const HMMER_Response_Alignment& aln = resp.alignments(i);

    cout << "\t" << "template_id: " << aln.template_id() << endl;
    cout << "\t" << "query_start: " << aln.query_start() << endl;
    cout << "\t" << "template_start: " << aln.template_start() << endl;
    cout << "\t" << "aligned_query_seq: " << aln.aligned_query_seq() << endl;
    cout << "\t" << "aligned_template_seq: " << aln.aligned_template_seq() << endl;

    if (aln.has_log_evalue()) {
      cout << "\t" << "log(evalue): " << aln.log_evalue() << endl;
    }

    if (aln.has_score()) {
      cout << "\t" << "score: " << aln.score() << endl;
    }
  }
}

int main(int argc, char* argv[]) {
  using zmq::context_t;
  using zmq::socket_t;

  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);

  CHECK(!FLAGS_fasta.empty()) << "Failed to provide required argument --fasta";

  // Verify that the version of the protocol buffer library that we linked
  // against is compatible with the version of the headers we compiled against
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  context_t ctx(1);
  socket_t sender(ctx, ZMQ_PUSH);
  socket_t receiver(ctx, ZMQ_PULL);
  sender.connect(FLAGS_send_socket.c_str());
  receiver.bind(FLAGS_recv_socket.c_str());

  HMMER_Request req;
  read_sequence_or_die(FLAGS_fasta, &req);
  send_request(req, &sender);

  HMMER_Response resp;
  receive_response(&resp, &receiver);
  emit(resp);

  return 0;
}
