#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <utility> // pair

#include <glm/glm.hpp>

#include "../Minimal/serverData.h"

#include <time.h> // for countdown function

#define PORT 8080
#define GAMETIME 10  // 10 seconds now for test purpose

bool terminated = false;
bool gameStarted = false;
time_t start_time;
int remain_time;

void startGame() {
	gameStarted = true;
	start_time = time(NULL);
	remain_time = GAMETIME;
}


/* for testing purpose for mirror Avatar, might be deleted */
static glm::mat4 _computeReflectionMatrix(const glm::vec4& plane)
{
	return glm::mat4(
		1.0f - 2.0f * plane.x * plane.x,
		-2.0f * plane.x * plane.y,
		-2.0f * plane.x * plane.z,
		-2.0f * plane.w * plane.x,

		-2.0f * plane.y * plane.x,
		1.0f - 2.0f * plane.y * plane.y,
		-2.0f * plane.y * plane.z,
		-2.0f * plane.w * plane.y,

		-2.0f * plane.z * plane.x,
		-2.0f * plane.z * plane.y,
		1.0f - 2.0f * plane.z * plane.z,
		-2.0f * plane.w * plane.z,

		0.0f,
		0.0f,
		0.0f,
		1.0f
	);
}

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

	// update one client's avatar position to another client
	// need to get new view and viewPos
	srv.bind("mirrorPos", [&](avatarPos p) {
		// render mirror head (from sample code)
		glm::vec4 reflectionPlane = glm::vec4(0.0, 0.0, -1.0, 0.0);
		glm::mat4 reflection = _computeReflectionMatrix(reflectionPlane);
		avatarPos newp;
		newp.view = p.view * reflection;
		newp.viewPos = glm::vec3(reflection * glm::vec4(p.viewPos, 1.0f));
		return newp;
	});


	// return result of game to client. -1 is game not ended, 0 is loss, 1 is win.
	// assume that the client does not call this function before game starts.
	srv.bind("result", []() {
		if (gameStarted) {
			return -1;
		}
		else {
			//TODO
		}
	});

	srv.bind("remainTime", []() {
		return remain_time;
	});

	srv.bind("startgame", []() {
		std::cout << "start game" << std::endl;
		if (!gameStarted) {
			startGame();
		}
	});

	srv.run();
}

void updateLoop() {
	auto dt = std::chrono::milliseconds(30);
	while (!terminated) {
		// data update

		// end the game when time is up
		remain_time = GAMETIME - (time(NULL) - start_time);
		if (remain_time <= 0) {
			gameStarted = false;
		}

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

