#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

// Include shared struct?


#define PORT 8080

bool terminated = false;

// handle clients
void startServer() {
	// Set up rpc server and listen to PORT
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;

	// handlers here
	srv.bind("test", []() {
		std::cout << "here" << std::endl;
		return "received\n";
	});

	srv.run();
}

void updateLoop() {
	auto dt = std::chrono::milliseconds(30);
	while (!terminated) {
		// data update

		// sleep
		std::this_thread::sleep_for(dt);
	}
}


using std::string;

int main()
{
	std::thread reqHandleThread(startServer);
	std::thread updateLoopThread(updateLoop);
	
	// Blocking call to start the server: non-blocking call is srv.async_run(threadsCount);


	reqHandleThread.join();
	updateLoopThread.join();
	return 0;
}

