#include "ApiSystem.h"
#include <stdlib.h>
#include <sys/statvfs.h>
#include "HttpReq.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include <thread>
#include <codecvt> 
#include <locale> 
#include "Log.h"
#include "Window.h"
#include "components/AsyncNotificationComponent.h"
#include "VolumeControl.h"

UpdateState::State ApiSystem::state = UpdateState::State::NO_UPDATE;

class ThreadedUpdater
{
public:
	ThreadedUpdater(Window* window) : mWindow(window)
	{
		ApiSystem::state = UpdateState::State::UPDATER_RUNNING;

		mWndNotification = new AsyncNotificationComponent(window, false);
		mWndNotification->updateTitle(_U("\uF019 ") + _("EMULATIONSTATION"));

		mWindow->registerNotificationComponent(mWndNotification);
		mHandle = new std::thread(&ThreadedUpdater::threadUpdate, this);
	}

	~ThreadedUpdater()
	{
		mWindow->unRegisterNotificationComponent(mWndNotification);
		delete mWndNotification;
	}

	void threadUpdate()
	{
		std::pair<std::string, int> updateStatus = ApiSystem::updateSystem([this](const std::string info)
		{
			auto pos = info.find(">>>");
			if (pos != std::string::npos)
			{
				std::string percent(info.substr(pos));
				percent = Utils::String::replace(percent, ">", "");
				percent = Utils::String::replace(percent, "%", "");
				percent = Utils::String::replace(percent, " ", "");

				int value = atoi(percent.c_str());

				std::string text(info.substr(0, pos));
				text = Utils::String::trim(text);

				mWndNotification->updatePercent(value);
				mWndNotification->updateText(text);
			}
			else
			{
				mWndNotification->updatePercent(-1);
				mWndNotification->updateText(info);
			}
		});

		if (updateStatus.second == 0)
		{
			ApiSystem::state = UpdateState::State::UPDATE_READY;

			mWndNotification->updateTitle(_U("\uF019 ") + _("UPDATE IS READY"));
			mWndNotification->updateText(_("RESTART EMULATIONSTATION TO APPLY"));

			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::hours(12));
		}
		else
		{
			ApiSystem::state = UpdateState::State::NO_UPDATE;

			std::string error = _("AN ERROR OCCURED") + std::string(": ") + updateStatus.first;
			mWindow->displayNotificationMessage(error);
		}

		delete this;
	}

private:
	std::thread*				mHandle;
	AsyncNotificationComponent* mWndNotification;
	Window*						mWindow;
};

void ApiSystem::startUpdate(Window* c)
{
}

std::string ApiSystem::checkUpdateVersion()
{
	return "";
}

std::pair<std::string, int> ApiSystem::updateSystem(const std::function<void(const std::string)>& func)
{
	return std::pair<std::string, int>("error.", 1);
}

std::vector<ThemeDownloadInfo> ApiSystem::getThemesList()
{
	LOG(LogDebug) << "ApiSystem::getThemesList";

	std::vector<ThemeDownloadInfo> res;

	HttpReq httpreq("https://batocera.org/upgrades/themes.txt");
	if (httpreq.wait())
	{
		auto lines = Utils::String::split(httpreq.getContent(), '\n');
		for (auto line : lines)
		{
			auto parts = Utils::String::splitAny(line, " \t");
			if (parts.size() > 1)
			{
				auto themeName = parts[0];

				std::string themeUrl = parts[1];
				std::string themeFolder = Utils::FileSystem::getFileName(themeUrl);

				bool themeExists = false;

				std::vector<std::string> paths{
					Utils::FileSystem::getHomePath() + "/.emulationstation/themes",
					"/etc/emulationstation/themes",
					"/userdata/themes"
				};

				for (auto path : paths)
				{
					themeExists = Utils::FileSystem::isDirectory(path + "/" + themeName) ||
						Utils::FileSystem::isDirectory(path + "/" + themeFolder) ||
						Utils::FileSystem::isDirectory(path + "/" + themeFolder + "-master");
					
					if (themeExists)
						break;
				}

				ThemeDownloadInfo info;
				info.installed = themeExists;
				info.name = themeName;
				info.url = themeUrl;

				res.push_back(info);
			}
		}
	}

	return res;
}

bool downloadGitRepository(const std::string url, const std::string fileName, const std::string label, const std::function<void(const std::string)>& func)
{
	if (func != nullptr)
		func(_("Downloading") + " " + label);

	long downloadSize = 0;

	std::string statUrl = Utils::String::replace(url, "https://github.com/", "https://api.github.com/repos/");
	if (statUrl != url)
	{
		HttpReq statreq(statUrl);
		if (statreq.wait())
		{
			std::string content = statreq.getContent();
			auto pos = content.find("\"size\": ");
			if (pos != std::string::npos)
			{
				auto end = content.find(",", pos);
				if (end != std::string::npos)
					downloadSize = atoi(content.substr(pos + 8, end - pos - 8).c_str()) * 1024;
			}
		}
	}

	HttpReq httpreq(url + "/archive/master.zip", fileName);

	int curPos = -1;
	while (httpreq.status() == HttpReq::REQ_IN_PROGRESS)
	{
		if (downloadSize > 0)
		{
			double pos = httpreq.getPosition();
			if (pos > 0 && curPos != pos)
			{
				if (func != nullptr)
				{
					std::string pc = std::to_string((int)(pos * 100.0 / downloadSize));
					func(std::string(_("Downloading") + " " + label + " >>> " + pc + " %"));
				}

				curPos = pos;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	if (httpreq.status() != HttpReq::REQ_SUCCESS)
		return false;

	return true;
}

std::pair<std::string, int> ApiSystem::installTheme(std::string themeName, const std::function<void(const std::string)>& func)
{
	return std::pair<std::string, int>(std::string("Theme not found"), 1);
}

	// Storage functions
unsigned long ApiSystem::getFreeSpaceGB(std::string mountpoint)
{
	LOG(LogDebug) << "ApiSystem::getFreeSpaceGB";

	int free = 0;

	struct statvfs fiData;
	if ((statvfs(mountpoint.c_str(), &fiData)) >= 0)
		free = (fiData.f_bfree * fiData.f_bsize) / (1024 * 1024 * 1024);

	return free;
}

std::string ApiSystem::getFreeSpaceSystemInfo() {
	return getFreeSpaceInfo("/");
}

std::string ApiSystem::getFreeSpaceBootInfo() {
	return getFreeSpaceInfo("/boot");
}

std::string ApiSystem::getFreeSpaceUserInfo() {
	return getFreeSpaceInfo("/roms");
}

std::string ApiSystem::getFreeSpaceInfo(const std::string mountpoint)
{
	LOG(LogDebug) << "ApiSystem::getFreeSpaceInfo";

	std::ostringstream oss;

	struct statvfs fiData;
	if ((statvfs(mountpoint.c_str(), &fiData)) < 0)
		return "";

	unsigned long long total = (unsigned long long) fiData.f_blocks * (unsigned long long) (fiData.f_bsize);
	unsigned long long free = (unsigned long long) fiData.f_bfree * (unsigned long long) (fiData.f_bsize);
	unsigned long long used = total - free;
	unsigned long percent = 0;

	if (total != 0)
	{  //for small SD card ;) with share < 1GB
		percent = used * 100 / total;
		oss << Utils::FileSystem::megaBytesToString(used / (1024L * 1024L)) << "/" << Utils::FileSystem::megaBytesToString(total / (1024L * 1024L)) << " (" << percent << " %)";
	}
	else
		oss << "N/A";

	return oss.str();
}

bool ApiSystem::isFreeSpaceSystemLimit() {
	return isFreeSpaceLimit("/");
}

bool ApiSystem::isFreeSpaceBootLimit() {
	return isFreeSpaceLimit("/boot");
}

bool ApiSystem::isFreeSpaceUserLimit() {
	return isFreeSpaceLimit("/roms", 2);
}

bool ApiSystem::isFreeSpaceLimit(const std::string mountpoint, int limit)
{
	return ((int) getFreeSpaceGB(mountpoint)) < limit;
}

bool ApiSystem::isTemperatureLimit(float temperature, float limit)
{
	return temperature > limit;
}

bool ApiSystem::isLoadCpuLimit(float load_cpu, float limit) // %
{
	return load_cpu > limit;
}

bool ApiSystem::isMemoryLimit(float total_memory, float free_memory, int limit) // %
{
	int percent = ( (int) ( (free_memory * 100) / total_memory ) );
	return percent < limit;
	//return ( (int) ( (free_memory * 100) / total_memory ) ) < limit;
}

bool ApiSystem::isBatteryLimit(float battery_level, int limit) // %
{
	return battery_level < limit;
}


std::string ApiSystem::getVersion()
{
	LOG(LogDebug) << "ApiSystem::getVersion()";
	return querySoftwareInformation(true).version;
}

std::string ApiSystem::getApplicationName()
{
	LOG(LogDebug) << "ApiSystem::getApplicationName()";
	return querySoftwareInformation(true).application_name;
}

std::string ApiSystem::getHostname()
{
	LOG(LogDebug) << "ApiSystem::getHostname()";
	return querySoftwareInformation(true).hostname;
}

std::string ApiSystem::getIpAddress()
{
	LOG(LogDebug) << "ApiSystem::getIpAddress()";

	std::string result = queryIPAddress(); // platform.h
	if (result.empty())
		return "NOT CONNECTED";

	return result;
}

float ApiSystem::getLoadCpu()
{
	LOG(LogDebug) << "ApiSystem::getLoadCpu()";
	return queryLoadCpu();
}

int ApiSystem::getFrequencyCpu()
{
	LOG(LogDebug) << "ApiSystem::getFrequencyCpu()";
	return queryFrequencyCpu();
}

float ApiSystem::getTemperatureCpu()
{
	LOG(LogDebug) << "ApiSystem::getTemperatureCpu()";
	return queryTemperatureCpu();
}

float ApiSystem::getTemperatureGpu()
{
	LOG(LogDebug) << "ApiSystem::getTemperatureGpu()";
	return queryTemperatureGpu();
}

int ApiSystem::getFrequencyGpu()
{
	LOG(LogDebug) << "ApiSystem::getFrequencyGpu()";
	return queryFrequencyGpu();
}

bool ApiSystem::isNetworkConnected()
{
	LOG(LogDebug) << "ApiSystem::isNetworkConnected()";
	return queryNetworkConnected();
}

NetworkInformation ApiSystem::getNetworkInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getNetworkInformation()";

	return queryNetworkInformation(summary); // platform.h
}

BatteryInformation ApiSystem::getBatteryInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getBatteryInformation()";

	return queryBatteryInformation(summary); // platform.h
}

CpuAndSocketInformation ApiSystem::getCpuAndChipsetInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getCpuAndChipsetInformation()";

	return queryCpuAndChipsetInformation(summary); // platform.h
}

RamMemoryInformation ApiSystem::getRamMemoryInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getRamMemoryInformation()";

	return queryRamMemoryInformation(summary); // platform.h
}

DisplayAndGpuInformation ApiSystem::getDisplayAndGpuInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getDisplayAndGpuInformation()";

	return queryDisplayAndGpuInformation(summary); // platform.h
}

SoftwareInformation ApiSystem::getSoftwareInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getSoftwareInformation()";

	return querySoftwareInformation(summary); // platform.h
}

DeviceInformation ApiSystem::getDeviceInformation(bool summary)
{
	LOG(LogDebug) << "ApiSystem::getDeviceInformation()";

	return queryDeviceInformation(summary); // platform.h
}

int ApiSystem::getBrightnessLevel()
{
	LOG(LogDebug) << "ApiSystem::getBrightnessLevel()";

	return queryBrightnessLevel();
}

void ApiSystem::setBrightnessLevel(int brightnessLevel)
{
	LOG(LogDebug) << "ApiSystem::setBrightnessLevel()";

	saveBrightnessLevel(brightnessLevel);
}

int ApiSystem::getVolume()
{
	LOG(LogDebug) << "ApiSystem::getVolume()";

	return VolumeControl::getInstance()->getVolume();
}

void ApiSystem::setVolume(int volumeLevel)
{
	LOG(LogDebug) << "ApiSystem::setVolume()";

	VolumeControl::getInstance()->setVolume(volumeLevel);
}
