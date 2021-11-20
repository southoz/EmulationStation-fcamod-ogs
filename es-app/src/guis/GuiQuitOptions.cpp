#include "guis/GuiQuitOptions.h"

#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"
#include "Window.h"
#include "ApiSystem.h"
#include "PowerSaver.h"


GuiQuitOptions::GuiQuitOptions(Window* window) : GuiSettings(window, _("\"QUIT\" SETTINGS").c_str())
{
	initializeMenu();
}

GuiQuitOptions::~GuiQuitOptions()
{
}

void GuiQuitOptions::initializeMenu()
{
	// full exit
	auto fullExitMenu = std::make_shared<SwitchComponent>(mWindow);
	fullExitMenu->setState(!Settings::getInstance()->getBool("ShowOnlyExit"));
	addWithLabel(_("VIEW MENU"), fullExitMenu);
	addSaveFunc([this, fullExitMenu]
		{
			bool old_value = !Settings::getInstance()->getBool("ShowOnlyExit");
			if (old_value != fullExitMenu->getState())
			{
				Settings::getInstance()->setBool("ShowOnlyExit", !fullExitMenu->getState());
				setVariable("reloadGuiMenu", true);
			}
		});

	// confirm to exit
	auto confirmToExit = std::make_shared<SwitchComponent>(mWindow);
	confirmToExit->setState(Settings::getInstance()->getBool("ConfirmToExit"));
	addWithLabel(_("CONFIRM TO \"QUIT\""), confirmToExit);
	addSaveFunc([this, confirmToExit]
		{
			bool old_value = Settings::getInstance()->getBool("ConfirmToExit");
			if (old_value != confirmToExit->getState())
			{
				Settings::getInstance()->setBool("ConfirmToExit", confirmToExit->getState());
				setVariable("reloadGuiMenu", true);
			}
		});

	addGroup(_("NO MENU"));

	// only exit action
	std::string only_exit_action = Settings::getInstance()->getString("OnlyExitAction");
	auto only_exit_action_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("ACTION TO EXECUTE"), false);

	only_exit_action_list->add(_("SHUTDOWN"), "shutdown", only_exit_action == "shutdown");
	only_exit_action_list->add(_("SUSPEND"), "suspend", only_exit_action == "suspend");
	only_exit_action_list->add(_("QUIT EMULATIONSTATION"), "exit_es", only_exit_action == "exit_es");

	addWithLabel(_("ACTION TO EXECUTE"), only_exit_action_list);
	addSaveFunc([this, only_exit_action_list]
		{
			std::string old_value = Settings::getInstance()->getString("OnlyExitAction");
			if (old_value != only_exit_action_list->getSelected())
			{
				Settings::getInstance()->setString("OnlyExitAction", only_exit_action_list->getSelected());
				setVariable("reloadGuiMenu", true);
			}
		});

	if (!only_exit_action_list->hasSelection())
	{
		only_exit_action_list->selectFirstItem();
		only_exit_action = only_exit_action_list->getSelected();
	}

	// confirm to exit
	auto show_action_menu = std::make_shared<SwitchComponent>(mWindow);
	show_action_menu->setState(Settings::getInstance()->getBool("ShowOnlyExitActionAsMenu"));
	addWithLabel(_("SHOW ACTION NAME AS MENU OPTION"), show_action_menu);
	addSaveFunc([this, show_action_menu]
		{
			bool old_value = Settings::getInstance()->getBool("ShowOnlyExitActionAsMenu");
			if (old_value != show_action_menu->getState())
			{
				Settings::getInstance()->setBool("ShowOnlyExitActionAsMenu", show_action_menu->getState());
				setVariable("reloadGuiMenu", true);
			}
		});

	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::ScriptId::POWER_KEY))
	{
		addGroup(_("POWERKEY BUTTON"));

		// push powerkey double to shutdown
		auto powerkey = std::make_shared<SwitchComponent>(mWindow);
		bool powerkey_value = ApiSystem::getInstance()->isPowerkeyState();
		powerkey->setState( powerkey_value );
		addWithLabel(_("PUSH TWO TIMES TO EXECUTE ACTION"), powerkey);

		// powerkey action
		std::string powerkey_action = ApiSystem::getInstance()->getPowerkeyAction();
		auto powerkey_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("ACTION TO EXECUTE"), false);

		powerkey_list->add(_("SHUTDOWN"), "shutdown", powerkey_action == "shutdown");
		powerkey_list->add(_("SUSPEND"), "suspend", powerkey_action == "suspend");

		addWithLabel(_("ACTION TO EXECUTE"), powerkey_list);

		if (!powerkey_list->hasSelection())
		{
			powerkey_list->selectFirstItem();
			powerkey_action = powerkey_list->getSelected();
		}

		// max interval time
		auto time_interval = std::make_shared<SliderComponent>(mWindow, 1.f, 10.f, 1.f, "s", true);
		float time_interval_value = (float) ApiSystem::getInstance()->getPowerkeyTimeInterval();
		time_interval->setValue( time_interval_value );
		addWithLabel(_("TIME INTERVAL"), time_interval);

		addSaveFunc([powerkey, powerkey_list, time_interval]
			{
				ApiSystem::getInstance()->setPowerkeyValues( powerkey->getState(), powerkey_list->getSelected(), (int) Math::round( time_interval->getValue() ) );
			});
	}
}
