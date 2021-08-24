#include "platform.h"
#include <SDL_events.h>

#include <unistd.h>

#include "Window.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include "Log.h"

#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <go2/display.h>
#include <vector>

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

bool executeSystemScript(const std::string command)
{
	LOG(LogInfo) << "Platform::executeSystemScript() - Running -> " << command;

	if (system(command.c_str()) == 0)
		return true;

	LOG(LogError) << "Platform::executeSystemScript() - Error executing " << command;
	return false;
}

void touch(const std::string& filename)
{
	int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
	if (fd >= 0)
		close(fd);
}

std::vector<std::string> executeSystemEnumerationScript(const std::string command)
{
	LOG(LogDebug) << "Platform::executeSystemEnumerationScript -> " << command;

	std::vector<std::string> res;

	FILE *pipe = popen(command.c_str(), "r");

	if (pipe == NULL)
		return res;

	char line[1024];
	while (fgets(line, 1024, pipe))
	{
		strtok(line, "\n");
		res.push_back(std::string(line));
	}

	pclose(pipe);
	return res;
}

void processQuitMode()
{
	switch (quitMode)
	{
	case QuitMode::RESTART:
		LOG(LogInfo) << "Platform::processQuitMode() - Restarting EmulationStation";
		touch("/tmp/es-restart");
		break;
	case QuitMode::REBOOT:
		LOG(LogInfo) << "Platform::processQuitMode() - Rebooting system";
		touch("/tmp/es-sysrestart");
		runRestartCommand();
		break;
	case QuitMode::SHUTDOWN:
		LOG(LogInfo) << "Platform::processQuitMode() - Shutting system down";
		touch("/tmp/es-shutdown");
		runShutdownCommand();
		break;
	}
}

std::string queryBatteryRootPath()
{
	static std::string batteryRootPath;

	if (batteryRootPath.empty())
	{
		auto files = Utils::FileSystem::getDirContent("/sys/class/power_supply");
		for (auto file : files)
		{
			if (Utils::String::toLower(file).find("/bat") != std::string::npos)
			{
				batteryRootPath = file;
				break;
			}
		}
	}
	return batteryRootPath;
}

BatteryInformation queryBatteryInformation(bool summary)
{
	BatteryInformation ret;

	std::string batteryRootPath = queryBatteryRootPath();

	// Find battery path - only at the first call
	if (!queryBatteryRootPath().empty())
	{
		try
		{
			ret.hasBattery = true;
			ret.level = queryBatteryLevel();
			if (!summary)
			{
				ret.isCharging = queryBatteryCharging();
				ret.health = Utils::String::toLower( Utils::String::replace( Utils::FileSystem::readAllText(batteryRootPath + "/health"), "\n", "" ) );
				ret.max_capacity = std::atoi(Utils::FileSystem::readAllText(batteryRootPath + "/charge_full").c_str()) / 1000; // milli amperes
				ret.voltage = queryBatteryVoltage();
			}
		} catch (...) {
			LOG(LogError) << "Platform::queryBatteryInformation() - Error reading battery data!!!";
		}
	}

	return ret;
}

int queryBatteryLevel()
{
	std::string batteryCapacityPath = queryBatteryRootPath() + "/capacity";
	if ( Utils::FileSystem::exists(batteryCapacityPath) )
		return std::atoi(Utils::FileSystem::readAllText(batteryCapacityPath).c_str());

	return 0;
}

bool queryBatteryCharging()
{
	std::string batteryStatusPath = queryBatteryRootPath() + "/status";
	if ( Utils::FileSystem::exists(batteryStatusPath) )
		return Utils::String::compareIgnoreCase( Utils::String::replace(Utils::FileSystem::readAllText(batteryStatusPath), "\n", ""), "discharging" );

	return false;
}

float queryBatteryVoltage()
{
	std::string batteryVoltagePath = queryBatteryRootPath() + "/voltage_now";
	if ( Utils::FileSystem::exists(batteryVoltagePath) )
		return std::atof(Utils::FileSystem::readAllText(batteryVoltagePath).c_str()) / 1000000; // volts

	return false;
}

std::string calculateIp4Netmask(std::string nbits_str)
{
	std::string netmask("");
	int bits = std::atoi( nbits_str.c_str() ),
			parts,
			rest;

	if (bits >= 8)
	{
		parts = bits / 8,
		rest = bits % 8;

		for (int i = 0 ; i < parts; i++)
			netmask.append("255.");
	}
	else
	{
		parts = 0;
		rest = bits;
	}

	std::string binary("");
	binary.append(rest, '1').append(8 - rest, '0');

	netmask.append( std::to_string( std::stoi(binary.c_str(), 0, 2) ) );
	parts++;

	if (parts < 4)
	{
		netmask.append(".");
		for (int i = parts ; i < 4; i++)
		{
			netmask.append( "0" );
			if (i < 3)
				netmask.append(".");
		}
	}

	return netmask;
}

NetworkInformation queryNetworkInformation(bool summary)
{
	NetworkInformation network;

	try
	{
		struct ifaddrs *ifAddrStruct = NULL;
		struct ifaddrs *ifa = NULL;
		void *tmpAddrPtr = NULL;

		getifaddrs(&ifAddrStruct);

		for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (!ifa->ifa_addr)
				continue;

			// check it is IP4 is a valid IP4 Address
			if (ifa->ifa_addr->sa_family == AF_INET)
			{
				tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
				char addressBuffer[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

				std::string ifName = ifa->ifa_name;
				if (ifName.find("eth") != std::string::npos || ifName.find("wlan") != std::string::npos || ifName.find("mlan") != std::string::npos || ifName.find("en") != std::string::npos || ifName.find("wl") != std::string::npos || ifName.find("p2p") != std::string::npos)
				{
					network.isConnected = true;
					network.iface = ifName;
					network.ssid = "";
					network.isEthernet = ifName.find("eth") != std::string::npos;
					network.isWifi = ifName.find("wlan") != std::string::npos;
					network.isIPv6 = false;
					network.ip_address = std::string(addressBuffer);
					break;
				}
			}
		}
		// Seeking for ipv6 if no IPV4
		if (!network.isConnected)
		{
			for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
			{
				if (!ifa->ifa_addr)
					continue;

				// check it is IP6 is a valid IP6 Address
				if (ifa->ifa_addr->sa_family == AF_INET6)
				{
					tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
					char addressBuffer[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);

					std::string ifName = ifa->ifa_name;
					if (ifName.find("eth") != std::string::npos || ifName.find("wlan") != std::string::npos || ifName.find("mlan") != std::string::npos || ifName.find("en") != std::string::npos || ifName.find("wl") != std::string::npos || ifName.find("p2p") != std::string::npos)
					{
						network.isConnected = true;
						network.iface = ifName;
						network.ssid = "";
						network.isEthernet = ifName.find("eth") != std::string::npos;
						network.isWifi = ifName.find("wlan") != std::string::npos;
						network.isIPv6 = true;
						network.ip_address = std::string(addressBuffer);
						break;
					}
				}
			}
		}

		if (ifAddrStruct != NULL)
			freeifaddrs(ifAddrStruct);

		if ( queryNetworkConnected() ) // NetworkManager running
		{
			std::string cmd("");

			char result_buffer[256];
			std::string field(""),
									awk_flag(""),
									nmcli_command("nmcli -f %s device show %s | awk '%s {print $2}'");

			if (network.isWifi)
			{
				field.clear();
				field.append( "GENERAL.CONNECTION" ); // wifi network ssid
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), "");
				network.ssid = getShOutput( result_buffer );
			}

			std::string ip_version( (network.isIPv6 ? "ip6" : "ip4") );
			if (summary)
			{
				field.clear();
				field.append( ip_version ).append(".address"); // IP address with netmask prefix
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), "");
				network.ip_address = getShOutput( result_buffer );
			}
			else
			{
				field.clear();
				field.append( ip_version ).append(".address"); // IP address with netmask prefix
				awk_flag.append("BEGIN{FS=\"/\"}"); // get netmask prefix
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), awk_flag.c_str());
				if (!network.isIPv6)
					network.netmask = calculateIp4Netmask( getShOutput( result_buffer ) );
				else
					network.netmask = getShOutput( result_buffer );

				field.clear();
				field.append( ip_version ).append(".gateway"); // gateway
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), "");
				network.gateway = getShOutput( result_buffer );

				field.clear();
				field.append( "GENERAL.HWADDR" ); // mac address
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), "");
				network.mac = getShOutput( result_buffer );

				field.clear();
				field.append( "ip4.dns" ), // dns1 address
				awk_flag.clear();
				awk_flag.append("tolower($1) ~ /^").append(field).append("\\[1\\]/");
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), awk_flag.c_str());
				network.dns1 = getShOutput( result_buffer );

				awk_flag.clear();
				awk_flag.append("tolower($1) ~ /^").append(field).append("\\[2\\]/"); // dns2 address
				snprintf(result_buffer, 128, nmcli_command.c_str(), field.c_str(), network.iface.c_str(), awk_flag.c_str());
				network.dns2 = getShOutput( result_buffer );

				nmcli_command.clear();
				if (network.isWifi)
				{
					nmcli_command.append("nmcli dev wifi | grep %s | awk '{print %s}'");
					field.clear();
					field.append( "$8" ) // wifi signal
							 .append( " \" \" $5" ) // wifi channel
							 .append( " \" \"  $10" ) // wifi security
							 .append( " \" \"  $6" ) // rate
							 .append( " \" \"  $7" ); // rate unit
					snprintf(result_buffer, 256, nmcli_command.c_str(), network.ssid.c_str(), field.c_str());
					std::vector<std::string> results = Utils::String::split(getShOutput( result_buffer ), ' ');
					network.signal = std::atoi( results.at(0).c_str() ); // wifi signal
					network.channel = std::atoi( results.at(1).c_str() ); // wifi channel
					network.security = results.at(2); // wifi security
					network.rate = std::atoi( results.at(3).c_str() ); // rate
					network.rate_unit = results.at(4);
				}
				else
				{
					nmcli_command.append("nmcli -f CAPABILITIES.SPEED dev show %s | awk '{print %s}'");
					field.clear();
					field.append( "$2" ) // rate
							 .append( " \" \" $3" ); // rate unit
					snprintf(result_buffer, 128, nmcli_command.c_str(), network.iface.c_str(), field.c_str());
					std::vector<std::string> results = Utils::String::split(getShOutput( result_buffer ), ' ');
					network.rate = std::atoi( results.at(0).c_str() ); // rate
					network.rate_unit = results.at(1);
				}
			}
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryNetworkInformation() - Error reading network data!!!";
	}

	return network;
}

std::string queryIPAddress()
{
	return queryNetworkInformation(true).ip_address;
}

bool queryNetworkConnected()
{
	try
	{
		if ( Utils::FileSystem::exists("/usr/bin/nmcli")
				&& (Utils::String::replace(getShOutput(R"(nmcli -t -f RUNNING general)"), "\n", "") == "running" )
				&& (Utils::String::replace(getShOutput(R"(nmcli -t -f STATE general)"), "\n", "") == "connected" ) )
			return true;
	} catch (...) {
		LOG(LogError) << "PLATFORM::queryNetworkConnected() - Error reading network data!!!";
	}
	return false;
}

CpuAndSocketInformation queryCpuAndChipsetInformation(bool summary)
{
	CpuAndSocketInformation chipset;

	try
	{
		chipset.cpu_load = queryLoadCpu();
		chipset.temperature = queryTemperatureCpu();

		if (!summary)
		{
			if (Utils::FileSystem::exists("/usr/bin/lscpu"))
			{
				chipset.vendor_id = getShOutput(R"(lscpu | egrep 'Vendor ID' | awk '{print $3}')");
				chipset.model_name = getShOutput(R"(lscpu | egrep 'Model name' | awk '{print $3}')");
				chipset.ncpus = std::atoi( getShOutput(R"(lscpu | egrep 'CPU\(s\)' | awk '{print $2}' | grep -v CPU)").c_str() );
				chipset.architecture = getShOutput(R"(lscpu | egrep 'Architecture' | awk '{print $2}')");
				chipset.nthreads_core = std::atoi( getShOutput(R"(lscpu | egrep 'Thread' | awk '{print $4}')").c_str() );
			}
			if (Utils::FileSystem::exists("/sys/devices/system/cpu/cpufreq/policy0/scaling_governor"))
			{
				chipset.governor = Utils::String::replace ( getShOutput(R"(cat /sys/devices/system/cpu/cpufreq/policy0/scaling_governor)"), "_" , " ");
			}
			chipset.frequency = queryFrequencyCpu(); // MHZ
			if (Utils::FileSystem::exists("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq"))
			{
				chipset.frequency_max = std::atoi( getShOutput(R"(sudo cat /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq)").c_str() );
				chipset.frequency_max = chipset.frequency_max / 1000; // MHZ
			}
			if (Utils::FileSystem::exists("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_min_freq"))
			{
				chipset.frequency_min = std::atoi( getShOutput(R"(sudo cat /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_min_freq)").c_str() );
				chipset.frequency_min = chipset.frequency_min / 1000; // MHZ
			}
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryCpuAndChipsetInformation() - Error reading chipset data!!!";
	}

	return chipset;
}

float queryLoadCpu()
{
	try
	{
		if (Utils::FileSystem::exists("/usr/bin/top"))
		{
			float cpu_iddle = std::atof( getShOutput(R"(top -b -n 1 | egrep '%Cpu' | awk '{print $8}')").c_str() );
			return 100.0f - cpu_iddle;
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryLoadCpu() - Error reading load CPU data!!!";
	}
	return 0.f;
}

float queryTemperatureCpu()
{
	try
	{
		if (Utils::FileSystem::exists("/sys/devices/virtual/thermal/thermal_zone0/temp"))
		{
			float temperature = std::atof( getShOutput(R"(cat /sys/devices/virtual/thermal/thermal_zone0/temp)").c_str() );
			return temperature / 1000;
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryTemperatureCpu() - Error reading temperature CPU data!!!";
	}
	return 0.f;
}

int queryFrequencyCpu()
{
	try
	{
		if (Utils::FileSystem::exists("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq"))
		{
			int frequency = std::atoi( getShOutput(R"(sudo cat /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq)").c_str() );
			return frequency / 1000; // MHZ
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryFrequencyCpu() - Error reading frequency CPU data!!!";
	}
	return 0;
}

RamMemoryInformation queryRamMemoryInformation(bool summary)
{
	RamMemoryInformation memory;
	try
	{
		if (Utils::FileSystem::exists("/usr/bin/top"))
		{
			memory.total = std::atof( getShOutput(R"(top -b -n 1 | egrep 'MiB Mem' | awk '{print $4}')").c_str() );
			memory.free = std::atof( getShOutput(R"(top -b -n 1 | egrep 'MiB Mem' | awk '{print $6}')").c_str() );
			if (!summary)
			{
				memory.used = std::atof( getShOutput(R"(top -b -n 1 | egrep 'MiB Mem' | awk '{print $8}')").c_str() );
				memory.cached = std::atof( getShOutput(R"(top -b -n 1 | egrep 'MiB Mem' | awk '{print $10}')").c_str() );
			}
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryRamMemoryInformation() - Error reading memory data!!!";
	}
	return memory;
}

DisplayAndGpuInformation queryDisplayAndGpuInformation(bool summary)
{
	DisplayAndGpuInformation data;
	try
	{
		data.temperature = queryTemperatureGpu();
		data.brightness_level = queryBrightnessLevel();

		if (!summary)
		{
			if (Utils::FileSystem::exists("/sys/devices/platform/ff400000.gpu/gpuinfo"))
				data.gpu_model = getShOutput(R"(cat /sys/devices/platform/ff400000.gpu/gpuinfo | awk '{print $1}')");

			if (Utils::FileSystem::exists("/sys/devices/platform/display-subsystem/drm/card0/card0-DSI-1/mode"))
				data.resolution = getShOutput(R"(cat /sys/devices/platform/display-subsystem/drm/card0/card0-DSI-1/mode)");

			if (Utils::FileSystem::exists("/sys/devices/platform/display-subsystem/graphics/fb0/bits_per_pixel"))
				data.bits_per_pixel = std::atoi( getShOutput(R"(cat /sys/devices/platform/display-subsystem/graphics/fb0/bits_per_pixel)").c_str() );

			if (Utils::FileSystem::exists("/sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/governor"))
				data.governor = Utils::String::replace ( getShOutput(R"(cat /sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/governor)"), "_" , " ");

			data.frequency = queryFrequencyGpu();

			if (Utils::FileSystem::exists("/sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/max_freq"))
			{
				data.frequency_max = std::atoi( getShOutput(R"(cat /sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/max_freq)").c_str() );
				data.frequency_max = data.frequency_max / 1000000; // MHZ
			}

			if (Utils::FileSystem::exists("/sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/min_freq"))
			{
				data.frequency_min = std::atoi( getShOutput(R"(cat /sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/min_freq)").c_str() );
				data.frequency_min = data.frequency_min / 1000000; // MHZ
			}

			data.brightness_system = queryBrightness();

			if (Utils::FileSystem::exists("/sys/devices/platform/backlight/backlight/backlight/max_brightness"))
				data.brightness_system_max = std::atoi( getShOutput(R"(cat /sys/devices/platform/backlight/backlight/backlight/max_brightness)").c_str() );
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryDisplayAndGpuInformation() - Error reading display and GPU data!!!";
	}
	return data;
}

float queryTemperatureGpu()
{
	try
	{
		if (Utils::FileSystem::exists("/sys/devices/virtual/thermal/thermal_zone1/temp"))
		{
			float temperature = std::atof( getShOutput(R"(cat /sys/devices/virtual/thermal/thermal_zone1/temp)").c_str() );
			return temperature / 1000;
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryTemperatureGpu() - Error reading temperature GPU data!!!";
	}
	return 0.f;
}

int queryFrequencyGpu()
{
	try
	{
		if (Utils::FileSystem::exists("/sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/cur_freq"))
		{
			int frequency = std::atoi( getShOutput(R"(cat /sys/devices/platform/ff400000.gpu/devfreq/ff400000.gpu/cur_freq)").c_str() );
			return frequency / 1000000; // MHZ
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryFrequencyGpu() - Error reading frequency GPU data!!!";
	}
	return 0;
}

int queryBrightness()
{
	if (Utils::FileSystem::exists("/sys/devices/platform/backlight/backlight/backlight/actual_brightness"))
		return std::atoi( getShOutput(R"(cat /sys/devices/platform/backlight/backlight/backlight/actual_brightness)").c_str() );

	return 0;
}

int queryBrightnessLevel()
{
	return (int) go2_display_backlight_get(NULL);
}

void saveBrightnessLevel(int brightness_level)
{
	go2_display_backlight_set(NULL, brightness_level);
}

SoftwareInformation querySoftwareInformation(bool summary)
{
	SoftwareInformation si;
	try
	{
		if ( Utils::FileSystem::exists("/opt/.retrooz/") )
			si.application_name = "RetroOZ";
		else if ( Utils::FileSystem::exists("/home/ark/") )
			si.application_name = "ArkOS";
		else if ( Utils::FileSystem::exists("/usr/share/plymouth/themes/text.plymouth") )
			si.application_name = getShOutput(R"(cat /usr/share/plymouth/themes/text.plymouth | grep -iw title | awk '{ gsub(/=/," "); print $2}')");
		else if ( Utils::FileSystem::exists("/usr/share/plymouth/themes/ubuntu-text/ubuntu-text.plymouth") )
			si.application_name = getShOutput(R"(cat /usr/share/plymouth/themes/ubuntu-text/ubuntu-text.plymouth | grep -iw title | awk '{ gsub(/=/," "); print $2}')");

		if ( Utils::FileSystem::exists("/opt/.retrooz/version") )
			si.version = Utils::String::replace(getShOutput(R"(cat /opt/.retrooz/version)"), "\n", "");
		else if ( Utils::FileSystem::exists("/home/ark/") )
		{
			if ( Utils::FileSystem::exists("/usr/share/plymouth/themes/text.plymouth") )
				si.version = getShOutput(R"(cat /usr/share/plymouth/themes/text.plymouth | grep -iw title | awk '{gsub(/=/," ")}; {print $3 " " $4}')");
			else if ( Utils::FileSystem::exists("/usr/share/plymouth/themes/ubuntu-text/ubuntu-text.plymouth") )
				si.version = getShOutput(R"(cat /usr/share/plymouth/themes/ubuntu-text/ubuntu-text.plymouth | grep -iw title | awk '{gsub(/=/," ")}; {print $3 " " $4}')");
		}

		if ( Utils::FileSystem::exists("/usr/bin/hostnamectl") )
			si.hostname = getShOutput(R"(hostnamectl | grep -iw hostname | awk '{print $3}')");

		if (!summary)
		{
			if ( Utils::FileSystem::exists("/usr/bin/hostnamectl") )
			{
				si.so_base = getShOutput(R"(hostnamectl | grep -iw system | awk '{print $3 " " $4 " " $5}')");
				si.linux = getShOutput(R"(hostnamectl | grep -iw kernel | awk '{print $2 " " $3}')");
			}
		}
	} catch (...) {
		LOG(LogError) << "Platform::querySoftwareInformation() - Error reading software data!!!";
	}
	return si;
}

DeviceInformation queryDeviceInformation(bool summary)
{
	DeviceInformation di;
	try
	{
		if ( Utils::FileSystem::exists("/proc/cpuinfo") )
		{
			di.hardware = getShOutput(R"(cat /proc/cpuinfo | grep -iw hardware | awk '{print $3 " " $4}')");
			di.revision = getShOutput(R"(cat /proc/cpuinfo | grep Revision | awk '{print $3 " " $4}')");
			di.serial = Utils::String::toUpper( getShOutput(R"(cat /proc/cpuinfo | grep -iw serial | awk '{print $3 " " $4}')") );
		}
		if ( Utils::FileSystem::exists("/usr/bin/hostnamectl") )
		{
			di.machine_id = Utils::String::toUpper( getShOutput(R"(hostnamectl | grep -iw machine | awk '{print $3}')") );
			di.boot_id = Utils::String::toUpper( getShOutput(R"(hostnamectl | grep -iw boot | awk '{print $3}')") );
		}
	} catch (...) {
		LOG(LogError) << "Platform::queryDeviceInformation() - Error reading device data!!!";
	}
	return di;
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

bool isUsbDriveMounted(std::string device)
{
	return ( Utils::FileSystem::exists(device) && Utils::FileSystem::exists("/bin/findmnt")
		&& !getShOutput("findmnt -rno SOURCE,TARGET \"" + device + '"').empty() );
}

std::string queryUsbDriveMountPoint(std::string device)
{
	std::string dev = "/dev/" + device;
	if ( Utils::FileSystem::exists(dev) && Utils::FileSystem::exists("/bin/lsblk") )
		return getShOutput("lsblk -no MOUNTPOINT " + dev);

	return "";
}

std::vector<std::string> queryUsbDriveMountPoints()
{
	std::vector<std::string> partitions = executeSystemEnumerationScript(R"(cat /proc/partitions | egrep sda. | awk '{print $4}')"),
													 mount_points;
	for (auto partition = begin (partitions); partition != end (partitions); ++partition)
	{
		std::string mp = queryUsbDriveMountPoint(*partition);
		if (!mp.empty())
			mount_points.push_back(mp);
	}

	return mount_points;
}

std::string queryTimezones()
{
	if (Utils::FileSystem::exists("/usr/local/bin/timezones"))
		return getShOutput(R"(/usr/local/bin/timezones available)");
	else if (Utils::FileSystem::exists("/usr/bin/timedatectl"))
		return getShOutput(R"(/usr/bin/timedatectl list-timezones | sort -u | tr '\n' ',')");
	else if (Utils::FileSystem::exists("/usr/share/zoneinfo/zone1970.tab"))
		return getShOutput(R"(cat /usr/share/zoneinfo/zone1970.tab | grep -v "^#" | awk '{ print $3 }' | sort -u | tr '\n' ',')");

	return "Europe/Paris";
}

std::string queryCurrentTimezone()
{
	if (Utils::FileSystem::exists("/usr/local/bin/timezones"))
		return getShOutput(R"(/usr/local/bin/timezones current)");
	else if (Utils::FileSystem::exists("/usr/bin/timedatectl"))
		return getShOutput(R"(/usr/bin/timedatectl | grep -iw \"Time zone\" | awk '{print $3}')");
	else  if (Utils::FileSystem::exists("/bin/readlink"))
		return getShOutput(R"(readlink -f /etc/localtime | sed 's;/usr/share/zoneinfo/;;')");

	return "Europe/Paris";
}

bool setCurrentTimezone(std::string timezone)
{
	if (timezone.empty())
		return false;

	if (Utils::FileSystem::exists("/usr/local/bin/timezones"))
		return executeSystemScript("/usr/local/bin/timezones set \"" + timezone + '"');
	else if (Utils::FileSystem::exists("/usr/bin/timedatectl"))
		return executeSystemScript("/usr/bin/sudo timedatectl set-timezone \"" + timezone + '"');

	return executeSystemScript("sudo ln -sf \"/usr/share/zoneinfo/"+ timezone +"\" /etc/localtime");
}
