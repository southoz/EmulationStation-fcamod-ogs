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
public:

	static unsigned long getFreeSpaceGB(std::string mountpoint);

	static std::string getFreeSpaceBootInfo();
	static std::string getFreeSpaceSystemInfo();
	static std::string getFreeSpaceUserInfo();
	static std::string getFreeSpaceInfo(const std::string mountpoint);

	static bool isFreeSpaceBootLimit();
	static bool isFreeSpaceSystemLimit();
	static bool isFreeSpaceUserLimit();

	static bool isFreeSpaceLimit(const std::string mountpoint, int limit = 1); // GB
	static bool isTemperatureLimit(float temperature, float limit = 70.0f); // ° C
	static bool isLoadCpuLimit(float load_cpu, float limit = 90.0f); // %
	static bool isMemoryLimit(float total_memory, float free_memory, int limit = 10); // %
	static bool isBatteryLimit(float battery_level, int limit = 15); // %

	static std::string getVersion();
	static std::string getApplicationName();
	static std::string getHostname();
	static std::string getIpAddress();
	static float getLoadCpu();
	static float getTemperatureCpu();
	static int getFrequencyCpu();
	static float getTemperatureGpu();
	static int getFrequencyGpu();
	static bool isNetworkConnected();

	static NetworkInformation getNetworkInformation(bool summary = true);
	static BatteryInformation getBatteryInformation(bool summary = true);
	static CpuAndSocketInformation getCpuAndChipsetInformation(bool summary = true);
	static RamMemoryInformation getRamMemoryInformation(bool summary = true);
	static DisplayAndGpuInformation getDisplayAndGpuInformation(bool summary = true);
	static SoftwareInformation getSoftwareInformation(bool summary = true);
	static DeviceInformation getDeviceInformation(bool summary = true);

	static int getBrightnessLevel();
  static void setBrightnessLevel(int brightnessLevel);

	static int getVolume();
  static void setVolume(int volumeLevel);

	static UpdateState::State state;

	static std::pair<std::string, int> updateSystem(const std::function<void(const std::string)>& func = nullptr);
	static std::string checkUpdateVersion();
	static void startUpdate(Window* c);

	static std::vector<ThemeDownloadInfo> getThemesList();
	static std::pair<std::string, int> installTheme(std::string themeName, const std::function<void(const std::string)>& func = nullptr);
};

#endif

