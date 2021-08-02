#include "guis/GuiSystemInformation.h"

#include "components/MenuComponent.h"
#include "guis/GuiSettings.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include "views/ViewController.h"
#include "Window.h"
#include "ApiSystem.h"
#include "Log.h"

//	MenuComponent(Window* window, const std::string title, const std::shared_ptr<Font>& titleFont = Font::get(FONT_SIZE_LARGE), const std::string subTitle = "");
GuiSystemInformation::GuiSystemInformation(Window* window) : GuiSettings(window, _("SYSTEM INFORMATION").c_str())
{
	LOG(LogInfo) << "GuiSystemInformation::GuiSystemInformation()";
	initializeMenu();
}

GuiSystemInformation::~GuiSystemInformation()
{
	LOG(LogInfo) << "GuiSystemInformation::~GuiSystemInformation()";
}

void GuiSystemInformation::initializeMenu()
{
	if (Settings::getInstance()->getBool("ShowDetailedSystemInfo"))
		showDetailedSystemInfo();
	else
		showSummarySystemInfo();
}

void GuiSystemInformation::showSummarySystemInfo()
{
	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	CpuAndSocketInformation csi = ApiSystem::getCpuAndChipsetInformation();
	DisplayAndGpuInformation di = ApiSystem::getDisplayAndGpuInformation();
	RamMemoryInformation memory = ApiSystem::getRamMemoryInformation();
	NetworkInformation ni = ApiSystem::getNetworkInformation();

	addGroup(_("CPU"));
	// CPU load
	bool warning = ApiSystem::isLoadCpuLimit( csi.cpu_load );
	auto loadCpu = std::make_shared<TextComponent>(mWindow, formatLoadCpu( csi.cpu_load ), font, warning ? 0xFF0000FF : color);
	addWithLabel(_("CPU LOAD"), loadCpu);

	// temperature
	warning = ApiSystem::isTemperatureLimit( csi.temperature );
	auto temperature_cpu = std::make_shared<TextComponent>(mWindow, formatTemperature( csi.temperature ), font, warning ? 0xFF0000FF : color);
	addWithLabel(_("TEMPERATURE"), temperature_cpu);

	addGroup(_("OTHER INFORMATION"));
	// temperature
	warning = ApiSystem::isTemperatureLimit( di.temperature );
	auto temperature_gpu = std::make_shared<TextComponent>(mWindow, formatTemperature( di.temperature ), font, warning ? 0xFF0000FF : color);
	addWithLabel(_("GPU") + " - " + _("TEMPERATURE"), temperature_gpu);

	// roms
	warning = ApiSystem::isFreeSpaceUserLimit();
	auto userspace = std::make_shared<TextComponent>(mWindow, ApiSystem::getFreeSpaceUserInfo(), font, warning ? 0xFF0000FF : color);
	addWithLabel(_("ROMS DISK USAGE"), userspace);

	// free ram
	warning = ApiSystem::isMemoryLimit( memory.total, memory.free );
	auto memFree = std::make_shared<TextComponent>(mWindow, formatMemory( memory.free, memory.total, true ), font, warning ? 0xFF0000FF : color);
	addWithLabel(_("FREE RAM MEMORY"), memFree);

	addGroup(_("NETWORK"));

// connected to network
	auto network_status = std::make_shared<TextComponent>(mWindow, (ni.isConnected ? "    " + _("CONNECTED") + " " : _("NOT CONNECTED")), font, color);
	addWithLabel(_("STATUS"), network_status);
	auto wifi_ssid = std::make_shared<TextComponent>(mWindow, ni.ssid, font, color);
	auto ip_address = std::make_shared<TextComponent>(mWindow, ni.ip_address, font, color);

	if (ni.isConnected)
	{
		if (ni.isWifi) // Wifi ssid
		{
			addWithLabel(_("WIFI SSID"), wifi_ssid);
		}

		// IP address
		addWithLabel(_("IP ADDRESS"), ip_address);
	}
}

void GuiSystemInformation::showDetailedSystemInfo()
{
	addSubMenu(_("CPU AND SOCKET"), [this]() { openCpuAndSocket(); });
	addSubMenu(_("RAM MEMORY"), [this]() { openRamMemory(); });
	addSubMenu(_("DISPLAY AND GPU"), [this]() { openDisplayAndGpu(); });
	addSubMenu(_("STORAGE"), [this]() { openStorage(); });
	addSubMenu(_("NETWORK"), [this]() { openNetwork(); });

	BatteryInformation bi = ApiSystem::getBatteryInformation(false);
	if (bi.hasBattery)
		addSubMenu(_("BATTERY"), [this, bi]() { openBattery(&bi); });

	addSubMenu(_("SOFTWARE"), [this]() { openSoftware(); });
	addSubMenu(_("DEVICE"), [this]() { openDevice(); });
}

void GuiSystemInformation::openCpuAndSocket()
{
	LOG(LogDebug) << "GuiSystemInformation::openCpuAndSocket()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("CPU AND SOCKET"));

	CpuAndSocketInformation csi = ApiSystem::getCpuAndChipsetInformation(false);

	// vendor ID
	s->addWithLabel(_("VENDOR ID"), std::make_shared<TextComponent>(window, csi.vendor_id,font, color));

	// model
	s->addWithLabel(_("MODEL NAME"), std::make_shared<TextComponent>(window, csi.model_name, font, color));

	// CPU(s)
	s->addWithLabel(_("Nº CPU"), std::make_shared<TextComponent>(window, std::to_string( csi.ncpus ), font, color));

	// architecture
	s->addWithLabel(_("ARCHITECTURE"), std::make_shared<TextComponent>(window, Utils::String::toUpper( csi.architecture ), font, color));

	// threads per core
	s->addWithLabel(_("THREAD(S) PER CORE"), std::make_shared<TextComponent>(window, std::to_string( csi.nthreads_core ), font, color));

	// CPU load
	bool warning = ApiSystem::isLoadCpuLimit( csi.cpu_load );
	auto loadCpu = std::make_shared<TextComponent>(window, formatLoadCpu( csi.cpu_load ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("CPU LOAD"), loadCpu);

	// temperature
	warning = ApiSystem::isTemperatureLimit( csi.temperature );
	auto temperature = std::make_shared<TextComponent>(window, formatTemperature( csi.temperature ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("TEMPERATURE"), temperature);

	// governor
	s->addWithLabel(_("GOVERNOR"), std::make_shared<TextComponent>(window, _( Utils::String::toUpper( csi.governor ) ), font, color));

	// frequency
	auto frequency = std::make_shared<TextComponent>(window, formatFrequency( csi.frequency ), font, color);
	s->addWithLabel(_("FREQUENCY"), frequency);

	// maximus frequency
	s->addWithLabel(_("FREQUENCY MAX"), std::make_shared<TextComponent>(window, formatFrequency( csi.frequency_max ), font, color));

	// minimus frequency
	s->addWithLabel(_("FREQUENCY MIN"), std::make_shared<TextComponent>(window, formatFrequency( csi.frequency_min ), font, color));

	window->pushGui(s);
}

void GuiSystemInformation::openRamMemory()
{
	LOG(LogDebug) << "GuiSystemInformation::openRamMemory()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("RAM MEMORY"));

	RamMemoryInformation memory = ApiSystem::getRamMemoryInformation(false);

	// total ram
	s->addWithLabel(_("TOTAL"), std::make_shared<TextComponent>(window, formatMemory( memory.total ), font, color));

	// free ram
	bool warning = ApiSystem::isMemoryLimit( memory.total, memory.free );
	auto memFree = std::make_shared<TextComponent>(window, formatMemory( memory.free, memory.total, true ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("FREE"), memFree);

	// used ram
	auto memUsed = std::make_shared<TextComponent>(window, formatMemory( memory.used, memory.total, true ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("USED"), memUsed);

	// cached ram
	auto memCached = std::make_shared<TextComponent>(window, formatMemory( memory.cached, memory.total, true ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("BUFFERED / CACHED"), memCached);
	window->pushGui(s);
}

void GuiSystemInformation::openDisplayAndGpu()
{
	LOG(LogDebug) << "GuiSystemInformation::openDisplayAndGpu()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("DISPLAY AND GPU"));

	DisplayAndGpuInformation di = ApiSystem::getDisplayAndGpuInformation(false);

	// GPU model
	s->addWithLabel(_("GPU"), std::make_shared<TextComponent>(window, di.gpu_model, font, color));

	// resolution
	s->addWithLabel(_("RESOLUTION"), std::make_shared<TextComponent>(window, di.resolution, font, color));

	// bits per pixel
	s->addWithLabel(_("BITS PER PIXEL"), std::make_shared<TextComponent>(window, std::to_string( di.bits_per_pixel ), font, color));

	// temperature
	bool warning = ApiSystem::isTemperatureLimit( di.temperature );
	auto temperature = std::make_shared<TextComponent>(window, formatTemperature( di.temperature ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("TEMPERATURE"), temperature);

	// governor
	s->addWithLabel(_("GOVERNOR"), std::make_shared<TextComponent>(window, _( Utils::String::toUpper( di.governor ) ), font, color));

	// frequency
	auto frequency = std::make_shared<TextComponent>(window, formatFrequency( di.frequency ), font, color);
	s->addWithLabel(_("FREQUENCY"), frequency);

	// maximus frequency
	s->addWithLabel(_("FREQUENCY MAX"), std::make_shared<TextComponent>(window, formatFrequency( di.frequency_max ), font, color));

	// minimus frequency
	s->addWithLabel(_("FREQUENCY MIN"), std::make_shared<TextComponent>(window, formatFrequency( di.frequency_min ), font, color));

	// brightness %
	s->addWithLabel(_("BRIGHTNESS"), std::make_shared<TextComponent>(window, std::to_string( di.brightness_level ) + " (%)", font, color));

	// brightness raw
	s->addWithLabel(_("SYSTEM BRIGHTNESS"), std::make_shared<TextComponent>(window, std::to_string( di.brightness_system ), font, color));

	// maximus brightness raw
	s->addWithLabel(_("MAX SYSTEM BRIGHTNESS"), std::make_shared<TextComponent>(window, std::to_string( di.brightness_system_max ), font, color));

	window->pushGui(s);
}

void GuiSystemInformation::openStorage()
{
	LOG(LogDebug) << "GuiSystemInformation::openStorage()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("STORAGE"));

	// boot
	bool warning = ApiSystem::isFreeSpaceBootLimit();
	auto bootspace = std::make_shared<TextComponent>(window, ApiSystem::getFreeSpaceBootInfo(), font, warning ? 0xFF0000FF : color);
	//bootspace->update(60000);
	s->addWithLabel(_("BOOT DISK USAGE"), bootspace);

	// system
	warning = ApiSystem::isFreeSpaceSystemLimit();
	auto systemspace = std::make_shared<TextComponent>(window, ApiSystem::getFreeSpaceSystemInfo(), font, warning ? 0xFF0000FF : color);
	//systemspace->update(60000);
	s->addWithLabel(_("SYSTEM DISK USAGE"), systemspace);

	// roms
	warning = ApiSystem::isFreeSpaceUserLimit();
	auto userspace = std::make_shared<TextComponent>(window, ApiSystem::getFreeSpaceUserInfo(), font, warning ? 0xFF0000FF : color);
	//userspace->update(60000);
	s->addWithLabel(_("ROMS DISK USAGE"), userspace);

	window->pushGui(s);
}

void GuiSystemInformation::openNetwork()
{
	LOG(LogDebug) << "GuiSystemInformation::openNetwork()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("NETWORK"));

	NetworkInformation ni = ApiSystem::getNetworkInformation(false);

// connected to network
	auto status = std::make_shared<TextComponent>(window, ni.isConnected ? "    " + _("CONNECTED") : _("NOT CONNECTED"), font, color);
	auto is_wifi = std::make_shared<TextComponent>(window, _( Utils::String::boolToString(ni.isWifi, true) ), font, color);
	auto ssid = std::make_shared<TextComponent>(window, ni.ssid, font, color);
	auto address = std::make_shared<TextComponent>(window, ni.ip_address, font, color);
	auto netmask = std::make_shared<TextComponent>(window, ni.netmask, font, color);
	auto gateway = std::make_shared<TextComponent>(window, ni.gateway, font, color);
	auto mac = std::make_shared<TextComponent>(window, ni.mac, font, color);
	auto dns1 = std::make_shared<TextComponent>(window, ni.dns1, font, color);
	auto dns2 = std::make_shared<TextComponent>(window, ni.dns2, font, color);
	auto signal = std::make_shared<TextComponent>(window, formatWifiSignal( ni.signal ), font, color);
	auto channel = std::make_shared<TextComponent>(window, std::to_string(ni.channel), font, color);
	auto security = std::make_shared<TextComponent>(window, ni.security, font, color);
	auto rate = std::make_shared<TextComponent>(window, formatNetworkRate(ni.rate, ni.rate_unit), font, color);

	s->addWithLabel(_("STATUS"), status);
	if (ni.isConnected)
	{
	// is Wifi network
		s->addWithLabel(_("WIFI"), is_wifi);

		if (ni.isWifi) // Wifi ssid
			s->addWithLabel(_("WIFI SSID"), ssid);

		// IP address
		s->addWithLabel(_("IP ADDRESS"), address);

		// IPv4 netmask or IPv6 subnet
		s->addWithLabel(_( (ni.isIPv6 ? "SUBNET" : "NETMASK") ), netmask);

		// gateway
		s->addWithLabel(_("GATEWAY"), gateway);

		// mac
		s->addWithLabel(_("MAC"), mac);

		// dns1
		s->addWithLabel(_("DNS1"), dns1);

		// dns2
		s->addWithLabel(_("DNS2"), dns2);

		if (ni.isWifi)
		{
				// signal
				s->addWithLabel(_("WIFI SIGNAL"), signal);

				// chanel
				s->addWithLabel(_("WIFI CHANNEL"), channel);

				// security
				s->addWithLabel(_("WIFI SECURITY"), security);
		}

		// rate
		s->addWithLabel(_("CONNECTION RATE"), rate);
	}

	window->pushGui(s);
}

void GuiSystemInformation::openBattery(const BatteryInformation *bi)
{
	LOG(LogDebug) << "GuiSystemInformation::openBattery()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("BATTERY"));

	// level
	bool warning = ApiSystem::isBatteryLimit(bi->level);
	auto level = std::make_shared<TextComponent>(window, formatBattery( bi->level ), font, warning ? 0xFF0000FF : color);
	s->addWithLabel(_("LEVEL"), level);

// is charging
	auto charging = std::make_shared<TextComponent>(window, _(Utils::String::boolToString(bi->isCharging, true) ), font, color);
	s->addWithLabel(_("CHARGING"), charging);

	// health
	s->addWithLabel(_("HEALTH"), std::make_shared<TextComponent>(window, _( Utils::String::toUpper(bi->health) ), font, color));

	// max capacity
	s->addWithLabel(_("CAPACITY"), std::make_shared<TextComponent>(window, std::to_string(bi->max_capacity) + " mA", font, color));

	// voltage
	auto voltage = std::make_shared<TextComponent>(window, formatVoltage( bi->voltage ), font, color);
	s->addWithLabel(_("VOLTAGE"), voltage);

	window->pushGui(s);
}

void GuiSystemInformation::openSoftware()
{
	LOG(LogDebug) << "GuiSystemInformation::openSoftware()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("SOFTWARE"));

	SoftwareInformation si = ApiSystem::getSoftwareInformation(false);

	// kernel
	s->addWithLabel(_("HOSTNAME"), std::make_shared<TextComponent>(window, si.hostname, font, color));
	// application Name
	s->addWithLabel(_("SO"), std::make_shared<TextComponent>(window, si.application_name, font, color));

	// application Name
	s->addWithLabel(_("VERSION"), std::make_shared<TextComponent>(window, si.version, font, color));

	// SO base
	s->addWithLabel(_("SO BASE"), std::make_shared<TextComponent>(window, si.so_base, font, color));

	// kernel
	s->addWithLabel(_("KERNEL"), std::make_shared<TextComponent>(window, si.linux, font, color));

	window->pushGui(s);
}

void GuiSystemInformation::openDevice()
{
	LOG(LogDebug) << "GuiSystemInformation::openSoftware()";
	auto pthis = this;
	Window* window = mWindow;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	auto s = new GuiSettings(window, _("DEVICE"));

	DeviceInformation di = ApiSystem::getDeviceInformation(false);

	// device info
	s->addWithLabel(_("HARDWARE"), std::make_shared<TextComponent>(window, di.hardware, font, color));

	// revision
	s->addWithLabel(_("REVISION"), std::make_shared<TextComponent>(window, di.revision, font, color));

	// serial
	s->addWithLabel(_("SERIAL"), std::make_shared<TextComponent>(window, di.serial, font, color));

	// machine ID
	s->addWithLabel(_("MACHINE ID"), std::make_shared<TextComponent>(window, di.machine_id, font, color));

	// boot ID
	s->addWithLabel(_("BOOT ID"), std::make_shared<TextComponent>(window, di.boot_id, font, color));

	window->pushGui(s);
}

std::string GuiSystemInformation::formatTemperature(float temp_raw)
{
	char buffer [16];
	sprintf (buffer, "%.2f° C", temp_raw);
	std::string temperature(buffer);
	return temperature;
}

std::string GuiSystemInformation::formatFrequency(int freq_raw)
{
	char buffer [16];
	sprintf (buffer, "%d MHZ", freq_raw);
	std::string freq(buffer);
	return freq;
}

std::string GuiSystemInformation::formatBattery(int bat_level)
{
	return std::to_string( bat_level ) + " %";
}

std::string GuiSystemInformation::formatWifiSignal(int wifi_signal)
{
	return formatBattery( wifi_signal );
}

std::string GuiSystemInformation::formatMemory(float mem_raw, float total_memory, bool show_percent)
{
	char buffer [32];
	float percent = 0.f;
	std::string format("%3.2f MB");
	if (show_percent)
	{
		format.append(" (%2.1f %%)");
		percent = (mem_raw * 100) / total_memory;
	}

	sprintf (buffer, format.c_str(), mem_raw, percent);
	std::string mem(buffer);
	return mem;
}

std::string GuiSystemInformation::formatLoadCpu(float cpu_load_raw)
{
	char buffer [16];
	sprintf (buffer, "%3.2f %%", cpu_load_raw);
	std::string cpu_load(buffer);
	return cpu_load;
}

std::string GuiSystemInformation::formatVoltage(float voltage_raw)
{
	char buffer [16];
	sprintf (buffer, "%1.2f V", voltage_raw);
	std::string voltage(buffer);
	return voltage;
}

std::string GuiSystemInformation::formatNetworkRate(int rate, std::string units)
{
	return std::to_string(rate) + " " + units;
}

