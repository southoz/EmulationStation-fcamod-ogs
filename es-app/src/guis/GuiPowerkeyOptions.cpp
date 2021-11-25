#include "guis/GuiPowerkeyOptions.h"

#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"
#include "Window.h"
#include "ApiSystem.h"


GuiPowerkeyOptions::GuiPowerkeyOptions(Window* window) : GuiSettings(window, _("POWERKEY BUTTON SETTINGS").c_str())
{
	initializeMenu();
}

GuiPowerkeyOptions::~GuiPowerkeyOptions()
{
}

void GuiPowerkeyOptions::initializeMenu()
{
	// powerkey action
	std::string powerkey_action = ApiSystem::getInstance()->getPowerkeyAction();
	auto powerkey_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("ACTION TO EXECUTE"), false);

	powerkey_list->add(_("shutdown"), "shutdown", powerkey_action == "shutdown");
	powerkey_list->add(_("suspend"), "suspend", powerkey_action == "suspend");
	powerkey_list->add(_("disabled"), "disabled", powerkey_action == "disabled");
	addWithLabel(_("ACTION TO EXECUTE"), powerkey_list);

	if (!powerkey_list->hasSelection())
	{
		powerkey_list->selectFirstItem();
		powerkey_action = powerkey_list->getSelected();
	}

	// push powerkey double to shutdown
	auto powerkey = std::make_shared<SwitchComponent>(mWindow);
	bool powerkey_value = ApiSystem::getInstance()->isPowerkeyState();
	powerkey->setState( powerkey_value );
	addWithLabel(_("PUSH TWO TIMES TO EXECUTE ACTION"), powerkey);

	// max interval time
	auto time_interval = std::make_shared<SliderComponent>(mWindow, 1.f, 10.f, 1.f, "s", true);
	float time_interval_value = (float) ApiSystem::getInstance()->getPowerkeyTimeInterval();
	time_interval->setValue( time_interval_value );
	addWithLabel(_("TIME INTERVAL"), time_interval);

	addSaveFunc([powerkey, powerkey_list, time_interval]
		{
			ApiSystem::getInstance()->setPowerkeyValues( powerkey_list->getSelected(), powerkey->getState(), (int) Math::round( time_interval->getValue() ) );
		});
}
