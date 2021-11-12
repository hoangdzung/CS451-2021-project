#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

#include "parser.hpp"
#include "udp.hpp"
#include "beb.hpp"
#include "hello.h"
#include <signal.h>

std::ofstream outputFile;
UDPSocket* udpSocket;
BestEffortBroadcast* beb;

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

  for(auto const &output: beb->getLogs()){
    outputFile << output << "\n" ;
  }
  outputFile.close();
  delete beb;  
  // exit directly from signal handler
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;
  unsigned long m,i;

  Parser parser(argc, argv);
  parser.parse();

  hello();
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  auto hosts = parser.hosts();
  for (auto &host : hosts) {
    std::cout << host.id << "\n";
    std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
    std::cout << "Machine-readable IP: " << host.ip << "\n";
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
    std::cout << "\n";
  }
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";
  outputFile.open(parser.outputPath());

  std::cout << "Path to config:\n";
  std::cout << "===============\n";
  std::cout << parser.configPath() << "\n\n";

  std::cout << "Doing some initialization...\n\n";

  std::cout << "Broadcasting and delivering messages...\n\n";

  std::ifstream config_file(parser.configPath());

  // config_file >> m >> i;
  // config_file.close();
  // udpSocket = new UDPSocket(hosts[parser.id()-1]);
  // udpSocket->start(); //if not seperate this, we have 2 untrackable socket
  // if (parser.id() != i) {
  //   for (unsigned int msg=1;msg<=m;msg ++) {
  //     udpSocket->put(hosts[i-1], msg);      
  //   }
  // }

  config_file >> m;
  config_file.close();
  //set attributes instead of assignment operator like UDP in previous version 
  // to avoid perfectLink point to wrong one
  // beb = BestEffortBroadcast(...) the addresses of them are different
  beb = new BestEffortBroadcast(hosts[parser.id()-1], hosts);
  // beb->setAttr(hosts[parser.id()-1], hosts);
  beb->start();
  for (unsigned int msg=1;msg<=m;msg ++) {
    beb->put(msg);      
  }

  // std::cout << "Done" << "\n";
  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }
  delete beb;  

  return 0;
}
