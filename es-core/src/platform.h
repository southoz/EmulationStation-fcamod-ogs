#pragma once
#ifndef ES_CORE_PLATFORM_H
#define ES_CORE_PLATFORM_H

#include <string>

class Window;

enum QuitMode
{
	QUIT = 0,
	RESTART = 1,
	SHUTDOWN = 2,
	REBOOT = 3
};

int runSystemCommand(const std::string& cmd_utf8, const std::string& name, Window* window); // run a utf-8 encoded in the shell (requires wstring conversion on Windows)
int quitES(QuitMode mode = QuitMode::QUIT);
void processQuitMode();

#if !defined(TRACE)
	#define TRACE(s) 
#endif

class StopWatch
{
public:
	StopWatch(std::string name)
	{

	}

	~StopWatch()
	{
	}

private:
	int mTicks;
	std::string mName;
};

std::string getShOutput(const std::string& mStr);

#endif // ES_CORE_PLATFORM_H
