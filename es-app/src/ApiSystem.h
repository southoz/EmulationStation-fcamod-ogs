#ifndef API_SYSTEM
#define API_SYSTEM

#include <string>
#include <functional>
#include <vector>
#include "platform.h"

class Window;

namespace UpdateState
{
	enum State
	{
		NO_UPDATE,
		UPDATER_RUNNING,
		UPDATE_READY
	};
}

struct ThemeDownloadInfo
{
	bool installed;
	std::string name;
	std::string url;
};

class ApiSystem 
{
protected:
	ApiSystem();

	virtual bool executeScript(const std::string command);
	virtual std::pair<std::string, int> executeScript(const std::string command, const std::function<void(const std::string)>& func);
	virtual std::vector<std::string> executeEnumerationScript(const std::string command);

	static ApiSystem* instance;

public:
	enum ScriptId : unsigned int
	{
		TIMEZONE = 0,
		POWER_KEY = 1
/*
		WIFI = 0,
		RETROACHIVEMENTS = 1,
		BLUETOOTH = 2,
		RESOLUTION = 3,
		BIOSINFORMATION = 4,
		NETPLAY = 5,
		KODI = 6,
		GAMESETTINGS = 7,
		DECORATIONS = 8,
		SHADERS = 9,
		DISKFORMAT = 10,
		OVERCLOCK = 11,
		PDFEXTRACTION = 12,
		BATOCERASTORE = 13,
		EVMAPY = 14,
		THEMESDOWNLOADER = 15,
		THEBEZELPROJECT = 16,
		PADSINFO = 17
*/
	};

	virtual bool isScriptingSupported(ScriptId script);

	static ApiSystem* getInstance();
	virtual void deinit() { };

	unsigned long getFreeSpaceGB(std::string mountpoint);

	std::string getFreeSpaceBootInfo();
	std::string getFreeSpaceSystemInfo();
	std::string getFreeSpaceUserInfo();
	std::string getFreeSpaceUsbDriveInfo(const std::string mountpoint);
	std::string getFreeSpaceInfo(const std::string mountpoint);

	std::vector<std::string> getUsbDriveMountPoints();

	bool isFreeSpaceBootLimit();
	bool isFreeSpaceSystemLimit();
	bool isFreeSpaceUserLimit();
	bool isFreeSpaceUsbDriveLimit(const std::string mountpoint);

	bool isFreeSpaceLimit(const std::string mountpoint, int limit = 1); // GB
	bool isTemperatureLimit(float temperature, float limit = 70.0f); // ° C
	bool isLoadCpuLimit(float load_cpu, float limit = 90.0f); // %
	bool isMemoryLimit(float total_memory, float free_memory, int limit = 10); // %
	bool isBatteryLimit(float battery_level, int limit = 15); // %

	std::string getVersion();
	std::string getApplicationName();
	std::string getHostname();
	std::string getIpAddress();
	float getLoadCpu();
	float getTemperatureCpu();
	int getFrequencyCpu();
	float getTemperatureGpu();
	int getFrequencyGpu();
	bool isNetworkConnected();
	int getBatteryLevel();
	bool isBatteryCharging();
	float getBatteryVoltage();

	NetworkInformation getNetworkInformation(bool summary = true);
	BatteryInformation getBatteryInformation(bool summary = true);
	CpuAndSocketInformation getCpuAndChipsetInformation(bool summary = true);
	RamMemoryInformation getRamMemoryInformation(bool summary = true);
	DisplayAndGpuInformation getDisplayAndGpuInformation(bool summary = true);
	SoftwareInformation getSoftwareInformation(bool summary = true);
	DeviceInformation getDeviceInformation(bool summary = true);

	int getBrightness();
	int getBrightnessLevel();
	void setBrightnessLevel(int brightnessLevel);

	int getVolume();
	void setVolume(int volumeLevel);

	std::string getTimezones();
	std::string getCurrentTimezone();
	bool setTimezone(std::string timezone);

	bool setPowerkeyState(bool state);
	bool getPowerkeyState();
	bool setPowerkeyIntervalTime(int interval_time);
	int getPowerkeyIntervalTime();

	static UpdateState::State state;

	std::pair<std::string, int> updateSystem(const std::function<void(const std::string)>& func = nullptr);
	std::string checkUpdateVersion();
	void startUpdate(Window* c);

	std::vector<ThemeDownloadInfo> getThemesList();
	std::pair<std::string, int> installTheme(std::string themeName, const std::function<void(const std::string)>& func = nullptr);
};

#endif
