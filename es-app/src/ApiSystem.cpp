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
#include "EsLocale.h"
#include <algorithm>

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
		std::pair<std::string, int> updateStatus = ApiSystem::getInstance()->updateSystem([this](const std::string info)
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

ApiSystem::ApiSystem() { }

ApiSystem* ApiSystem::instance = nullptr;

ApiSystem *ApiSystem::getInstance()
{
	if (ApiSystem::instance == nullptr)
		ApiSystem::instance = new ApiSystem();

	return ApiSystem::instance;
}

std::vector<std::string> ApiSystem::executeEnumerationScript(const std::string command)
{
	return executeSystemEnumerationScript(command);
}

std::pair<std::string, int> ApiSystem::executeScript(const std::string command, const std::function<void(const std::string)>& func)
{
	LOG(LogInfo) << "ApiSystem::executeScript() - Running -> " << command;

	FILE *pipe = popen(command.c_str(), "r");
	if (pipe == NULL)
	{
		LOG(LogError) << "Error executing " << command;
		return std::pair<std::string, int>("Error starting command : " + command, -1);
	}

	char line[1024];
	while (fgets(line, 1024, pipe))
	{
		strtok(line, "\n");

		// Long theme names/URL can crash the GUI MsgBox
		// "48" found by trials and errors. Ideally should be fixed
		// in es-core MsgBox -- FIXME
		if (strlen(line) > 48)
			line[47] = '\0';

		if (func != nullptr)
			func(std::string(line));
	}

	int exitCode = WEXITSTATUS(pclose(pipe));
	return std::pair<std::string, int>(line, exitCode);
}

bool ApiSystem::executeScript(const std::string command)
{
	return executeSystemScript(command);
}

bool ApiSystem::isScriptingSupported(ScriptId script)
{
	std::vector<std::string> executables;

	switch (script)
	{
		case TIMEZONE:
				executables.push_back("timezones");
				break;
		case POWER_KEY:
				executables.push_back("es-powerkey");
				break;
		case DISPLAY:
				executables.push_back("es-display");
				break;
		case SYSTEM_HOTKEY_EVENTS:
				executables.push_back("es-system_hotkey");
				break;
		case WIFI:
				executables.push_back("es-wifi");
				break;
	case ApiSystem::RETROACHIVEMENTS:
				executables.push_back("es-cheevos");
				break;
	case ApiSystem::LANGUAGE:
				executables.push_back("es-language");
				break;

/*
	case ApiSystem::RETROACHIVEMENTS:
#ifdef CHEEVOS_DEV_LOGIN
		return true;
#endif
		break;
	case ApiSystem::KODI:
		executables.push_back("kodi");
		break;
	case ApiSystem::WIFI:
		executables.push_back("batocera-wifi");
		break;
	case ApiSystem::BLUETOOTH:
		executables.push_back("batocera-bluetooth");
		break;
	case ApiSystem::RESOLUTION:
		executables.push_back("batocera-resolution");
		break;
	case ApiSystem::BIOSINFORMATION:
		executables.push_back("batocera-systems");
		break;
	case ApiSystem::DISKFORMAT:
		executables.push_back("batocera-format");
		break;
	case ApiSystem::OVERCLOCK:
		executables.push_back("batocera-overclock");
		break;
	case ApiSystem::THEMESDOWNLOADER:
		executables.push_back("batocera-es-theme");
		break;
	case ApiSystem::NETPLAY:
		executables.push_back("7zr");
		break;
	case ApiSystem::PDFEXTRACTION:
		executables.push_back("pdftoppm");
		executables.push_back("pdfinfo");
		break;
	case ApiSystem::BATOCERASTORE:
		executables.push_back("batocera-store");
		break;
	case ApiSystem::THEBEZELPROJECT:
		executables.push_back("batocera-es-thebezelproject");
		break;
	case ApiSystem::PADSINFO:
		executables.push_back("batocera-padsinfo");
		break;
	case ApiSystem::EVMAPY:
		executables.push_back("evmapy");
		break;
*/
	}

//	if (executables.size() == 0)
//		return true;

	for (auto executable : executables)
		if (Utils::FileSystem::exists("/usr/bin/" + executable) || Utils::FileSystem::exists("/usr/local/bin/" + executable))
			return true;

	return false;
}

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
					"/etc/emulationstation/themes",
					Utils::FileSystem::getEsConfigPath() + "/themes",
					Utils::FileSystem::getUserDataPath() + "/themes"
				};

				for (auto path : paths)
				{
					themeExists = Utils::FileSystem::isDirectory(path + "/" + themeName) ||
						Utils::FileSystem::isDirectory(path + "/" + themeFolder) ||
						Utils::FileSystem::isDirectory(path + "/" + themeFolder + "-master");
					
					if (themeExists)
					{
						LOG(LogInfo) << "ApiSystem::getThemesList() - Get themes directory path '" << path << "'...";
						break;
					}
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

std::string ApiSystem::getFreeSpaceUsbDriveInfo(const std::string mountpoint)
{
	LOG(LogDebug) << "ApiSystem::getFreeSpaceUsbDriveInfo() - mount point: " << mountpoint;
	if ( isUsbDriveMounted(mountpoint) )
		return getFreeSpaceInfo(mountpoint);

	return "";
}

std::string ApiSystem::getFreeSpaceInfo(const std::string mountpoint)
{
	LOG(LogDebug) << "ApiSystem::getFreeSpaceInfo() - mount point: " << mountpoint;

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


std::vector<std::string> ApiSystem::getUsbDriveMountPoints()
{
	return queryUsbDriveMountPoints();
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

bool ApiSystem::isFreeSpaceUsbDriveLimit(const std::string mountpoint) {
	if ( isUsbDriveMounted(mountpoint) )
		return isFreeSpaceLimit(mountpoint, 2);

	return false;
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
	return queryHostname();
}

bool ApiSystem::setHostname(std::string hostname)
{
	LOG(LogDebug) << "ApiSystem::setHostname()";
	return setCurrentHostname(hostname);
}

std::string ApiSystem::getIpAddress()
{
	LOG(LogDebug) << "ApiSystem::getIpAddress()";

	std::string result = queryIPAddress(); // platform.h
	if (result.empty())
		return "___.___.___.___/__";

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

int ApiSystem::getBrightness()
{
	LOG(LogDebug) << "ApiSystem::getBrightness()";

	return queryBrightness();
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

int ApiSystem::getBatteryLevel()
{
	LOG(LogDebug) << "ApiSystem::getBatteryLevel()";

	return queryBatteryLevel();
}

bool ApiSystem::isBatteryCharging()
{
	LOG(LogDebug) << "ApiSystem::isBatteryCharging()";

	return queryBatteryCharging();
}

float ApiSystem::getBatteryVoltage()
{
	LOG(LogDebug) << "ApiSystem::getBatteryVoltage()";

	return queryBatteryVoltage();
}

std::string ApiSystem::getTimezones()
{
	LOG(LogDebug) << "ApiSystem::getTimezones()";

	return queryTimezones();
}

std::string ApiSystem::getCurrentTimezone()
{
	LOG(LogInfo) << "ApiSystem::getCurrentTimezone()";

	return queryCurrentTimezone();
}

bool ApiSystem::setTimezone(std::string timezone)
{
	LOG(LogInfo) << "ApiSystem::setTimezone() - TZ: " << timezone;

	return setCurrentTimezone(timezone);
}

bool ApiSystem::setPowerkeyState(bool state)
{
	LOG(LogInfo) << "ApiSystem::setPowerkeyState()";

	return executeSystemScript("es-powerkey set two_push_shutdown " + stateToString(state));
}

bool ApiSystem::isPowerkeyState()
{
	LOG(LogInfo) << "ApiSystem::isPowerkeyState()";

	return stringToState(getShOutput(R"(es-powerkey get two_push_shutdown)"));
}

bool ApiSystem::setPowerkeyTimeInterval(int time_interval)
{
	LOG(LogInfo) << "ApiSystem::setPowerkeyTimeInterval()";

	return executeSystemScript("es-powerkey set max_interval_time " + std::to_string(time_interval));
}

int ApiSystem::getPowerkeyTimeInterval()
{
	LOG(LogInfo) << "ApiSystem::getPowerkeyTimeInterval()";

	std::string time_interval = Utils::String::replace(getShOutput(R"(es-powerkey get max_interval_time)"), "\n", "");
	if (time_interval.empty())
		return 5;

	return std::atoi( time_interval.c_str() );
}

bool ApiSystem::setPowerkeyAction(const std::string action)
{
	LOG(LogInfo) << "ApiSystem::setPowerkeyAction()";

	return executeSystemScript("es-powerkey set action " + action);
}

std::string ApiSystem::getPowerkeyAction()
{
	LOG(LogInfo) << "ApiSystem::getPowerkeyAction()";

	std::string action = Utils::String::replace(getShOutput(R"(es-powerkey get action)"), "\n", "");
	if (action.empty())
		return "shutdown";

	return action;
}

bool ApiSystem::setDisplayBlinkLowBattery(bool blink)
{
	LOG(LogInfo) << "ApiSystem::setPowerkeyAction()";

	return executeSystemScript("es-display blink_low_battery " + stateToString(state));
}

bool ApiSystem::setSystemHotkeyBrightnessEvent( bool state )
{
	LOG(LogInfo) << "ApiSystem::setSystemHotkeyBrightnessEvent()";

	return executeSystemScript("es-system_hotkey set brightness " + stateToString(state));
}

bool ApiSystem::isSystemHotkeyBrightnessEvent()
{
	LOG(LogInfo) << "ApiSystem::isSystemHotkeyBrightnessEvent()";

	return stringToState(getShOutput(R"(es-system_hotkey get brightness)"));
}

bool ApiSystem::setSystemHotkeyVolumeEvent( bool state )
{
	LOG(LogInfo) << "ApiSystem::setSystemHotkeyVolumeEvent()";

	return executeSystemScript("es-system_hotkey set volume " + stateToString(state));
}

bool ApiSystem::isSystemHotkeyVolumeEvent()
{
	LOG(LogInfo) << "ApiSystem::isSystemHotkeyVolumeEvent()";

	return stringToState(getShOutput(R"(es-system_hotkey get volume)"));
}

bool ApiSystem::setSystemHotkeyWifiEvent( bool state )
{
	LOG(LogInfo) << "ApiSystem::setSystemHotkeyWifiEvent()";

	return executeSystemScript("es-system_hotkey set wifi " + stateToString(state));
}

bool ApiSystem::isSystemHotkeyWifiEvent()
{
	LOG(LogInfo) << "ApiSystem::isSystemHotkeyWifiEvent()";

	return stringToState(getShOutput(R"(es-system_hotkey get wifi)"));
}

bool ApiSystem::setSystemHotkeyPerformanceEvent( bool state )
{
	LOG(LogInfo) << "ApiSystem::setSystemHotkeyPerformanceEvent()";

	return executeSystemScript("es-system_hotkey set performance " + stateToString(state));
}

bool ApiSystem::isSystemHotkeyPerformanceEvent()
{
	LOG(LogInfo) << "ApiSystem::isSystemHotkeyPerformanceEvent()";

	return stringToState(getShOutput(R"(es-system_hotkey get performance)"));
}

bool ApiSystem::setSystemHotkeySuspendEvent( bool state )
{
	LOG(LogInfo) << "ApiSystem::setSystemHotkeySuspendEvent()";

	return executeSystemScript("es-system_hotkey set suspend " + stateToString(state));
}

bool ApiSystem::isSystemHotkeySuspendEvent()
{
	LOG(LogInfo) << "ApiSystem::isSystemHotkeySuspendEvent()";

	return stringToState(getShOutput(R"(es-system_hotkey get suspend)"));
}

bool ApiSystem::ping()
{
	if (!executeScript("timeout 1 ping -c 1 -t 255 8.8.8.8")) // ping Google DNS
		return executeScript("timeout 2 ping -c 1 -t 255 8.8.4.4"); // ping Google secondary DNS & give 2 seconds

	return true;
}

std::string ApiSystem::getInternetStatus()
{
	return ping() ? _("CONNECTED") : _("NOT CONNECTED");
}

std::vector<std::string> ApiSystem::getWifiNetworks(bool scan)
{
	LOG(LogInfo) << "ApiSystem::getWifiNetworks()";

	return executeEnumerationScript(scan ? "es-wifi scanlist" : "es-wifi list");
}

bool ApiSystem::enableWifi(std::string ssid, std::string key)
{
	LOG(LogInfo) << "ApiSystem::enableWifi()";

	return executeScript("es-wifi enable \"" + ssid + "\" \"" + key + "\"");
}

bool ApiSystem::disableWifi()
{
	LOG(LogInfo) << "ApiSystem::disableWifi()";

	return executeScript("es-wifi disable");
}

bool ApiSystem::isWifiEnabled()
{
	LOG(LogInfo) << "ApiSystem::disableWifi()";

	return queryWifiEnabled();
}

std::string ApiSystem::stateToString(bool state)
{
	return state ? std::string("enabled") : std::string("disabled");
}

bool ApiSystem::stringToState(const std::string state)
{
	return ( Utils::String::replace(state, "\n", "") == "enabled" );
}

bool ApiSystem::setLanguage(std::string language)
{
	LOG(LogInfo) << "ApiSystem::setLanguage()";

	return executeScript("es-language set " + language + " &");
}

bool ApiSystem::setRetroachievementsEnabled(bool state)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsEnabled()";

	return executeScript("es-cheevos set cheevos_enable " + Utils::String::boolToString(state));
}

bool ApiSystem::getRetroachievementsEnabled()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsEnabled()";

	return Utils::String::toBool(getShOutput(R"(es-cheevos get cheevos_enable)"));
}

bool ApiSystem::setRetroachievementsHardcoreEnabled(bool state)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsHardcoreEnabled()";

	return executeScript("es-cheevos set cheevos_hardcore_mode_enable " + Utils::String::boolToString(state));
}

bool ApiSystem::getRetroachievementsHardcoreEnabled()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsHardcoreEnabled()";

	return Utils::String::toBool(getShOutput(R"(es-cheevos get cheevos_hardcore_mode_enable)"));
}

bool ApiSystem::setRetroachievementsLeaderboardsEnabled(bool state)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsLeaderboardsEnabled()";

	return executeScript("es-cheevos set cheevos_leaderboards_enable " + Utils::String::boolToString(state));
}

bool ApiSystem::getRetroachievementsLeaderboardsEnabled()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsLeaderboardsEnabled()";

	return Utils::String::toBool(getShOutput(R"(es-cheevos get cheevos_leaderboards_enable)"));
}

bool ApiSystem::setRetroachievementsVerboseEnabled(bool state)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsVerboseEnabled()";

	return executeScript("es-cheevos set cheevos_verbose_enable " + Utils::String::boolToString(state));
}

bool ApiSystem::getRetroachievementsVerboseEnabled()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsVerboseEnabled()";

	return Utils::String::toBool(getShOutput(R"(es-cheevos get cheevos_verbose_enable)"));
}

bool ApiSystem::setRetroachievementsAutomaticScreenshotEnabled(bool state)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsAutomaticScreenshotEnabled()";

	return executeScript("es-cheevos set cheevos_auto_screenshot " + Utils::String::boolToString(state));
}

bool ApiSystem::getRetroachievementsAutomaticScreenshotEnabled()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsAutomaticScreenshotEnabled()";

	return Utils::String::toBool(getShOutput(R"(es-cheevos get cheevos_auto_screenshot)"));
}

bool ApiSystem::setRetroachievementsUnlockSoundEnabled(bool state)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsUnlockSoundEnabled()";

	return executeScript("es-cheevos set cheevos_unlock_sound_enable " + Utils::String::boolToString(state));
}

bool ApiSystem::getRetroachievementsUnlockSoundEnabled()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsUnlockSoundEnabled()";

	return Utils::String::toBool(getShOutput(R"(es-cheevos get cheevos_unlock_sound_enable)"));
}

bool ApiSystem::setRetroachievementsUsername(std::string username)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsUsername()";

	return executeScript("es-cheevos set cheevos_username \"" + username + '"');
}

std::string ApiSystem::getRetroachievementsUsername()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsUsername()";

	return getShOutput(R"(es-cheevos get cheevos_username)");
}

bool ApiSystem::setRetroachievementsPassword(std::string password)
{
	LOG(LogInfo) << "ApiSystem::setRetroachievementsPassword()";

	return executeScript("es-cheevos set cheevos_password \"" + password + '"');
}

std::string ApiSystem::getRetroachievementsPassword()
{
	LOG(LogInfo) << "ApiSystem::getRetroachievementsPassword()";

	return getShOutput(R"(es-cheevos get cheevos_password)");
}

