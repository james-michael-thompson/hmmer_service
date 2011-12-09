#include "hmmer.pb.h"

#include <cstdio>
#include <iostream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <zmq.hpp>

DEFINE_string(send_socket, "tcp://localhost:8001", "Communication channel to client");
DEFINE_string(recv_socket, "tcp://*:8000", "Communication channel from client");

using namespace std;

// Writes a HMMER_Response to the client
void send_response(const HMMER_Response& resp, zmq::socket_t* sender) {
  CHECK_NOTNULL(sender);
  zmq::message_t msg(resp.ByteSize());

  string m;
  resp.SerializeToString(&m);
  sprintf((char *) msg.data(), "%s", m.c_str());

  sender->send(msg);
}

// Receives a HMMER_Request from the client
void receive_request(HMMER_Request* req, zmq::socket_t* receiver) {
  CHECK_NOTNULL(req);
  CHECK_NOTNULL(receiver);

  zmq::message_t msg;
  receiver->recv(&msg);
  req->ParseFromString(static_cast<char*>(msg.data()));
}

int main(int argc, char* argv[]) {
  using zmq::context_t;
  using zmq::socket_t;

  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);

  // Verify that the version of the protocol buffer library that we linked
  // against is compatible with the version of the headers we compiled against
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // Setup communication channels
  context_t ctx(1);
  socket_t sender(ctx, ZMQ_PUSH);
  socket_t receiver(ctx, ZMQ_PULL);
  sender.connect(FLAGS_send_socket.c_str());
  receiver.bind(FLAGS_recv_socket.c_str());

  while (1) {
    HMMER_Request req;
    receive_request(&req, &receiver);
    cout << "Received request: " << req.fasta_sequence() << endl;

    HMMER_Response resp;
    resp.set_job_id(1);
    send_response(resp, &sender);
  }

  return 0;
}
