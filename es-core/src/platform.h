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

struct BatteryInformation
{
	BatteryInformation()
	{
		hasBattery = false;
		level = 0;
		isCharging = false;
		std::string health("Good");
		max_capacity = 0;
		voltage = 0.f;
	}

	bool hasBattery;
	int  level;
	bool isCharging;
	std::string health;
	int max_capacity;
	float voltage;
};

BatteryInformation queryBatteryInformation(bool summary);
int queryBatteryLevel();
bool queryBatteryCharging();
float queryBatteryVoltage();

struct NetworkInformation
{
	NetworkInformation()
	{
		isConnected = false;
		iface = "lo";
		isEthernet = false;
		isWifi = false;
		isIPv6 = false;
		netmask = "255.255.0.0";
		ip_address = "127.0.0.1";
		mac = "00:00:00:00:00:00";
		dns1 = "--";
		dns2 = "--";
		gateway = "--";
		ssid = "--"; // wifi
		signal = 0; // wifi
		channel = 0; // wifi
		security = "N/A"; // wifi
		rate = 0;
		rate_unit = "N/A";
	}

	bool isConnected;
	std::string iface;
	bool isEthernet;
	bool isWifi;
	bool isIPv6;
	std::string netmask;
	std::string ip_address;
	std::string mac;
	std::string gateway;
	std::string dns1;
	std::string dns2;
	std::string ssid;  // wifi
	int signal; // wifi
	int channel; // wifi
	std::string security; // wifi
	int rate;
	std::string rate_unit;
};

NetworkInformation queryNetworkInformation(bool summary);
std::string queryIPAddress();
bool queryNetworkConnected();


struct CpuAndSocketInformation
{
	CpuAndSocketInformation()
	{
		vendor_id = "N/A";
		model_name = "N/A";
		ncpus = 0;
		architecture = "N/A";
		nthreads_core = 0;
		cpu_load = 0.f;
		temperature = 0.f;
		governor = "N/A";
		frequency = 0;
		frequency_max = 0;
		frequency_min = 0;
	}

	std::string vendor_id;
	std::string model_name;
	int ncpus;
	std::string architecture;
	int nthreads_core;
	float cpu_load;
	float temperature;
	std::string governor;
	int frequency;
	int frequency_max;
	int frequency_min;
};

CpuAndSocketInformation queryCpuAndChipsetInformation(bool summary);
float queryLoadCpu();
float queryTemperatureCpu();
int queryFrequencyCpu();

struct RamMemoryInformation
{
	RamMemoryInformation()
	{
		total = 0.f;
		free = 0.f;
		used = 0.f;
		cached = 0.f;
	}

	float total;
	float free;
	float used;
	float cached;
};

RamMemoryInformation queryRamMemoryInformation(bool summary);

struct DisplayAndGpuInformation
{
	DisplayAndGpuInformation()
	{
		gpu_model = "N/A";
		resolution = "N/A";
		bits_per_pixel = 0;
		temperature = 0.f;
		governor = "N/A";
		frequency = 0;
		frequency_max = 0;
		frequency_min = 0;
		brightness_level = 0;
		brightness_system = 0;
		brightness_system_max = 0;
	}

	std::string gpu_model;
	std::string resolution;
	int bits_per_pixel;
	float temperature;
	std::string governor;
	int frequency;
	int frequency_max;
	int frequency_min;
	int brightness_level;
	int brightness_system;
	int brightness_system_max;
};

DisplayAndGpuInformation queryDisplayAndGpuInformation(bool summary);
float queryTemperatureGpu();
int queryFrequencyGpu();
int queryBrightness();
int queryBrightnessLevel();
void saveBrightnessLevel(int brightness_level);

struct SoftwareInformation
{
	SoftwareInformation()
	{
		hostname = "N/A";
		application_name = "N/A";
		version = "N/A";
		so_base = "N/A";
		linux = "N/A";
	}

	std::string hostname;
	std::string application_name;
	std::string version;
	std::string so_base;
	std::string linux;
};

SoftwareInformation querySoftwareInformation(bool summary);

struct DeviceInformation
{
	DeviceInformation()
	{
		hardware = "N/A";
		revision = "N/A";
		serial = "N/A";
		machine_id = "N/A";
		boot_id = "N/A";
	}

	std::string hardware;
	std::string revision;
	std::string serial;
	std::string machine_id;
	std::string boot_id;
};

DeviceInformation queryDeviceInformation(bool summary);

std::string getShOutput(const std::string& mStr);

#endif // ES_CORE_PLATFORM_H
