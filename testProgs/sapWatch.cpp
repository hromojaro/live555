/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2017, Live Networks, Inc.  All rights reserved
// A program that receives and prints SDP/SAP announcements

#include "Groupsock.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"
#include <iostream>
#include <ctime>
#include <iomanip>
using namespace std;

static unsigned const maxPacketSize = 65536;
static unsigned char packet[maxPacketSize+1];

int main(int argc, char** argv) {
  // Argument checking
  char const* sessionAddressStr = "224.2.127.254";	// Default
  if (argc > 1) {
    sessionAddressStr = argv[1];
  }

  // Time
  time_t result;

  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  // Create a 'groupsock' for the input multicast group,port:
  //cout << "* Address: " << sessionAddressStr << endl;
  struct in_addr sessionAddress;
  sessionAddress.s_addr = our_inet_addr(sessionAddressStr);

  const Port port(9875);
  const unsigned char ttl = 0; // we're only reading from this mcast group

  Groupsock inputGroupsock(*env, sessionAddress, port, ttl);

  // Start reading and printing incoming packets
  // (Because this is the only thing we do, we can just do this
  // synchronously, in a loop, so we don't need to set up an asynchronous
  // event handler like we do in most of the other test programs.)
  unsigned packetSize;
  struct sockaddr_in fromAddress;
  while (inputGroupsock.handleRead(packet, maxPacketSize,
				   packetSize, fromAddress)) {
    result = time(nullptr);
    cout << endl << "[packet from" << AddressString(fromAddress).val() << "(" << packetSize << "bytes) on " << put_time(localtime(&result),"%c %Z") << " (Unix time: " << result << ")]" << endl;
    // Ignore the first 8 bytes (SAP header).
    if (packetSize < 8) {
      *env << "Ignoring short packet from " << AddressString(fromAddress).val() << "%s!\n";
      continue;
    }

    // convert "application/sdp\0" -> "application/sdp\0x20"
    // or all other nonprintable characters to blank, except new line
    unsigned idx = 8;
    while (idx < packetSize) {
      if (packet[idx] < 0x20 && packet[idx] != '\n') packet[idx] = 0x20;
      idx++;
    }

    packet[packetSize] = '\0'; // just in case
    cout << (char*)(packet+8);
  }

  return 0; // only to prevent compiler warning
}
