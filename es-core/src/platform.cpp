#include "platform.h"
#include <SDL_events.h>

#include <unistd.h>

#include <fcntl.h>

#include "Window.h"
#include "Log.h"

#include "GuiComponent.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"

int runShutdownCommand()
{
	return system("sudo shutdown -h now");
}

int runRestartCommand()
{
	return system("sudo shutdown -r now");
}

void splitCommand(std::string cmd, std::string* executable, std::string* parameters)
{
	std::string c = Utils::String::trim(cmd);
	size_t exec_end;

	if (c[0] == '\"')
	{
		exec_end = c.find_first_of('\"', 1);
		if (std::string::npos != exec_end)
		{
			*executable = c.substr(1, exec_end - 1);
			*parameters = c.substr(exec_end + 1);
		}
		else
		{
			*executable = c.substr(1, exec_end);
			std::string().swap(*parameters);
		}
	}
	else
	{
		exec_end = c.find_first_of(' ', 0);
		if (std::string::npos != exec_end)
		{
			*executable = c.substr(0, exec_end);
			*parameters = c.substr(exec_end + 1);
		}
		else
		{
			*executable = c.substr(0, exec_end);
			std::string().swap(*parameters);
		}
	}
}

int runSystemCommand(const std::string& cmd_utf8, const std::string& name, Window* window)
{
	return system(cmd_utf8.c_str());
}

QuitMode quitMode = QuitMode::QUIT;

int quitES(QuitMode mode)
{
	quitMode = mode;

	SDL_Event *quit = new SDL_Event();
	quit->type = SDL_QUIT;
	SDL_PushEvent(quit);
	return 0;
}

void touch(const std::string& filename)
{
	int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
	if (fd >= 0)
		close(fd);
}

void processQuitMode()
{
	switch (quitMode)
	{
	case QuitMode::RESTART:
		LOG(LogInfo) << "Restarting EmulationStation";
		touch("/tmp/es-restart");
		break;
	case QuitMode::REBOOT:
		LOG(LogInfo) << "Rebooting system";
		touch("/tmp/es-sysrestart");
		runRestartCommand();
		break;
	case QuitMode::SHUTDOWN:
		LOG(LogInfo) << "Shutting system down";
		touch("/tmp/es-shutdown");
		runShutdownCommand();
		break;
	}
}

// Adapted from emuelec
std::string getShOutput(const std::string& mStr) 
{
    std::string result, file;
    FILE* pipe{popen(mStr.c_str(), "r")};
    char buffer[256];

    while(fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        file = buffer;
        result += file.substr(0, file.size() - 1);
    }

    pclose(pipe);
    return result;
}
