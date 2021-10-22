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
private:
	std::string stateToString(bool state);
	bool stringToState(const std::string state);

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
		POWER_KEY = 1,
		DISPLAY = 2,
		SYSTEM_HOTKEY_EVENTS = 3,
		WIFI = 4,
		RETROACHIVEMENTS = 5,
		LANGUAGE = 6,
		SYSTEM_INFORMATION = 7,
		AUTO_SUSPEND = 8
/*
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
	bool setHostname(std::string hostname);
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
	bool isPowerkeyState();
	bool setPowerkeyTimeInterval(int time_interval);
	int getPowerkeyTimeInterval();
	bool setPowerkeyAction(const std::string action);
	std::string getPowerkeyAction();
	bool setDisplayBlinkLowBattery(bool state);
	bool setSystemHotkeyBrightnessEvent( bool state );
	bool isSystemHotkeyBrightnessEvent();
	bool setSystemHotkeyVolumeEvent( bool state );
	bool isSystemHotkeyVolumeEvent();
	bool setSystemHotkeyWifiEvent( bool state );
	bool isSystemHotkeyWifiEvent();
	bool setSystemHotkeyPerformanceEvent( bool state );
	bool isSystemHotkeyPerformanceEvent();
	bool setSystemHotkeySuspendEvent( bool state );
	bool isSystemHotkeySuspendEvent();
	bool setDeviceAutoSuspendByTime( bool state );
	bool isDeviceAutoSuspend();
	bool isDeviceAutoSuspendByTime();
	bool setAutoSuspendTimeout( int timeout );
	int getAutoSuspendTimeout();
	bool setDeviceAutoSuspendByBatteryLevel( bool state );
	bool isDeviceAutoSuspendByBatteryLevel();
	bool setAutoSuspendBatteryLevel( int battery_level );
	int getAutoSuspendBatteryLevel();

	virtual bool ping();
	bool getInternetStatus();
	std::vector<std::string> getWifiNetworks(bool scan = false);
	bool enableWifi(std::string ssid, std::string key);
	bool disconnectWifi(std::string ssid);
	bool disableWifi();
	bool isWifiEnabled();
	bool enableManualWifiDns(std::string ssid, std::string dnsOne, std::string dnsTwo);
	bool disableManualWifiDns(std::string ssid);

	bool setLanguage(std::string language);

	bool setRetroachievementsEnabled(bool state);
	bool getRetroachievementsEnabled();
	bool setRetroachievementsHardcoreEnabled(bool state);
	bool getRetroachievementsHardcoreEnabled();
	bool setRetroachievementsLeaderboardsEnabled(bool state);
	bool getRetroachievementsLeaderboardsEnabled();
	bool setRetroachievementsVerboseEnabled(bool state);
	bool getRetroachievementsVerboseEnabled();
	bool setRetroachievementsAutomaticScreenshotEnabled(bool state);
	bool getRetroachievementsAutomaticScreenshotEnabled();
	bool setRetroachievementsUnlockSoundEnabled(bool state);
	bool getRetroachievementsUnlockSoundEnabled();
	std::string getRetroachievementsUsername();
	bool setRetroachievementsUsername(std::string username);
	std::string getRetroachievementsPassword();
	bool setRetroachievementsPassword(std::string password);

	static UpdateState::State state;

	std::pair<std::string, int> updateSystem(const std::function<void(const std::string)>& func = nullptr);
	std::string checkUpdateVersion();
	void startUpdate(Window* c);

	std::vector<ThemeDownloadInfo> getThemesList();
	std::pair<std::string, int> installTheme(std::string themeName, const std::function<void(const std::string)>& func = nullptr);
};

#endif
