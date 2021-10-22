#include "guis/GuiQuitOptions.h"

#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "ApiSystem.h"
#include "PowerSaver.h"


GuiQuitOptions::GuiQuitOptions(Window* window) : GuiSettings(window, _("\"QUIT\" SETTINGS").c_str()), mPopupDisplayed(false)
{
	initializeMenu();
}

GuiQuitOptions::~GuiQuitOptions()
{
	mPopupDisplayed = false;
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
		addSaveFunc([this, powerkey, powerkey_value]
			{
				bool new_powerkey_value = powerkey->getState();
				if (powerkey_value != new_powerkey_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_powerkey_value] { ApiSystem::getInstance()->setPowerkeyState( new_powerkey_value ); }));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setPowerkeyState( new_powerkey_value );
					}
				}

			});

		// powerkey action
		std::string powerkey_action = ApiSystem::getInstance()->getPowerkeyAction();
		auto powerkey_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("ACTION TO EXECUTE"), false);

		powerkey_list->add(_("SHUTDOWN"), "shutdown", powerkey_action == "shutdown");
		powerkey_list->add(_("SUSPEND"), "suspend", powerkey_action == "suspend");

		addWithLabel(_("ACTION TO EXECUTE"), powerkey_list);
		addSaveFunc([this, powerkey_list, powerkey_action]
			{
				std::string new_powerkey_action = powerkey_list->getSelected();
				if (powerkey_action != new_powerkey_action)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_powerkey_action] { ApiSystem::getInstance()->setPowerkeyAction( new_powerkey_action ); }));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setPowerkeyAction( new_powerkey_action );
					}
				}
			});

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
		addSaveFunc([this, time_interval, time_interval_value]
			{
				float new_time_interval_value = Math::round( time_interval->getValue() );
				if (time_interval_value != new_time_interval_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_time_interval_value] { ApiSystem::getInstance()->setPowerkeyTimeInterval( (int) new_time_interval_value ); }));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setPowerkeyTimeInterval( (int) new_time_interval_value );
					}
				}
			});
	}

	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::ScriptId::AUTO_SUSPEND))
	{
		addGroup(_("DEVICE AUTO SUSPEND"));

		// enable device autosuspend
		auto auto_suspend = std::make_shared<SwitchComponent>(mWindow);
		bool auto_suspend_value = ApiSystem::getInstance()->isDeviceAutoSuspend();
		auto_suspend->setState( auto_suspend_value );
		addWithLabel(_("ENABLE"), auto_suspend);
		addSaveFunc([this, auto_suspend, auto_suspend_value]
			{
				bool new_auto_suspend_value = auto_suspend->getState();
				if (auto_suspend_value != new_auto_suspend_value)
				{
					Window *window = mWindow;
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						window->pushGui(new GuiMsgBox(window,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [this, window, new_auto_suspend_value] { configAutoSuspend( window, new_auto_suspend_value ); }));
					}
					else
					{
						mPopupDisplayed = false;
						configAutoSuspend( window, new_auto_suspend_value );
					}
				}
			});

		// auto suspend timeout
		auto auto_suspend_timeout = std::make_shared<SliderComponent>(mWindow, 1.f, 120.f, 1.f, "m", true);
		float auto_suspend_timeout_value = (float) ApiSystem::getInstance()->getAutoSuspendTimeout();
		auto_suspend_timeout->setValue( auto_suspend_timeout_value );
		addWithLabel(_("SUSPEND AFTER"), auto_suspend_timeout);
		addSaveFunc([this, auto_suspend_timeout, auto_suspend_timeout_value]
			{
				float new_auto_suspend_timeout_value = Math::round( auto_suspend_timeout->getValue() );
				if (auto_suspend_timeout_value != new_auto_suspend_timeout_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_auto_suspend_timeout_value]
								{
									ApiSystem::getInstance()->setAutoSuspendTimeout( (int) new_auto_suspend_timeout_value );
								}));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setAutoSuspendTimeout( (int) new_auto_suspend_timeout_value );
					}
				}
			});
	}
}

void GuiQuitOptions::configAutoSuspend(Window *window, bool enabled)
{
	if (enabled && Settings::getInstance()->getString("ScreenSaverBehavior") == "suspend")
	{
		window->pushGui(new GuiMsgBox(window,
			_("THE \"SUSPEND\" SCREENSAVER WAS DISABLED. THE SCREENSAVER BEHAVIOR WAS SETTLED TO \"DIM\"."),
			_("OK"), []
				{
					Settings::getInstance()->setString("ScreenSaverBehavior", "dim");
					PowerSaver::updateTimeouts();
				}));
	}
	ApiSystem::getInstance()->setDeviceAutoSuspend( enabled );
}
