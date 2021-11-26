#include "guis/GuiSystemHotkeyEventsOptions.h"

#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "ApiSystem.h"


GuiSystemHotkeyEventsOptions::GuiSystemHotkeyEventsOptions(Window* window) : GuiSettings(window, _("SYSTEM HOTKEY EVENTS SETTINGS").c_str()), mPopupDisplayed(false)
{
	initializeMenu(window);
}

GuiSystemHotkeyEventsOptions::~GuiSystemHotkeyEventsOptions()
{

}

void GuiSystemHotkeyEventsOptions::initializeMenu(Window* window)
{
	// brightness events
	auto brightness = std::make_shared<SwitchComponent>(window);
	bool brightness_value = ApiSystem::getInstance()->isSystemHotkeyBrightnessEvent();
	brightness->setState(brightness_value);
	addWithLabel(_("BRIGHTNESS"), brightness);

	// volume events
	auto volume = std::make_shared<SwitchComponent>(window);
	bool volume_value = ApiSystem::getInstance()->isSystemHotkeyVolumeEvent();
	volume->setState(volume_value);
	addWithLabel(_("VOLUME"), volume);

	// wifi events
	auto wifi = std::make_shared<SwitchComponent>(window);
	bool wifi_value = ApiSystem::getInstance()->isSystemHotkeyWifiEvent();
	wifi->setState(wifi_value);
	addWithLabel(_("WIFI"), wifi);

	// performance events
	auto performance = std::make_shared<SwitchComponent>(window);
	bool performance_value = ApiSystem::getInstance()->isSystemHotkeyPerformanceEvent();
	performance->setState(performance_value);
	addWithLabel(_("PERFORMANCE"), performance);

	// suspend events
	auto suspend = std::make_shared<SwitchComponent>(window);
	bool suspend_value = ApiSystem::getInstance()->isSystemHotkeySuspendEvent();
	suspend->setState(suspend_value);
	addWithLabel(_("SUSPEND"), suspend);

	addSaveFunc([brightness, brightness_value, volume, volume_value, wifi, wifi_value, performance, performance_value, suspend, suspend_value]
		{
			bool brightness_new_value = brightness->getState();
			bool volume_new_value = volume->getState();
			bool wifi_new_value = wifi->getState();
			bool performance_new_value = performance->getState();
			bool suspend_new_value = suspend->getState();

			if ( (brightness_value != brightness_new_value) || (volume_value != volume_new_value) || (wifi_value != wifi_new_value)
					|| (performance_value != performance_new_value) || (suspend_value != suspend_new_value) )
			{
				ApiSystem::getInstance()->setSystemHotkeysValues( brightness_new_value, volume_new_value, wifi_new_value, performance_new_value, suspend_new_value );
			}
		});
}
