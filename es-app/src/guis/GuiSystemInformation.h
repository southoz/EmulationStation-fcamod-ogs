#pragma once
#ifndef ES_APP_GUIS_GUI_SYSTEM_INFORMATION_H
#define ES_APP_GUIS_GUI_SYSTEM_INFORMATION_H

#include "components/MenuComponent.h"
#include "GuiSettings.h"
#include "platform.h"
#include "Settings.h"


class GuiSystemInformation : public GuiSettings
{
public:
	GuiSystemInformation(Window* window);
	~GuiSystemInformation();

//	virtual void update(int deltaTime);

private:
	void initializeMenu();
	void showSummarySystemInfo();
	void showDetailedSystemInfo();
	void openCpuAndSocket();
	void openRamMemory();
	void openDisplayAndGpu();
	void openStorage();
	void openNetwork();
	void openBattery(const BatteryInformation *bi);
	void openSoftware();
	void openDevice();

	static std::string formatTemperature	(float temp_raw);
	static std::string formatFrequency		(int freq_raw);
	static std::string formatBattery			(int bat_level);
	static std::string formatWifiSignal		(int wifi_signal);
	static std::string formatMemory				(float mem_raw, float total_memory = 0.f, bool show_percent = false);
	static std::string formatLoadCpu			(float cpu_load);
	static std::string formatVoltage			(float voltage_raw);
	static std::string formatNetworkRate	(int rate, std::string units);

};

#endif // ES_APP_GUIS_GUI_SYSTEM_INFORMATION_H
