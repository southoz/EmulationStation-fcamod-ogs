#include "guis/GuiMenusOptions.h"

#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "ApiSystem.h"
#include "Log.h"


GuiMenusOptions::GuiMenusOptions(Window* window) : GuiSettings(window, _("MENUS SETTINGS").c_str()), mPopupDisplayed(false)
{
	initializeMenu(window);
}

GuiMenusOptions::~GuiMenusOptions()
{
	mPopupDisplayed = false;
}

void GuiMenusOptions::initializeMenu(Window* window)
{
	// menus on top
	auto menusOnTop = std::make_shared<SwitchComponent>(window);
	menusOnTop->setState(Settings::getInstance()->getBool("MenusOnDisplayTop"));
	addWithLabel(_("ON THE TOP OF THE DISPLAY"), menusOnTop);
	addSaveFunc([menusOnTop, window]
		{
			bool old_value = Settings::getInstance()->getBool("MenusOnDisplayTop");
			if (old_value != menusOnTop->getState())
			{
				Settings::getInstance()->setBool("MenusOnDisplayTop", menusOnTop->getState());
				if (menusOnTop->getState() && Settings::getInstance()->getBool("MenusAllWidth"))
				{
					Settings::getInstance()->setBool("MenusAllHeight", false);
					window->pushGui(new GuiMsgBox(window, _("YOU SELECTED TO SHOW ON THE TOP OF THE DISPLAY, THE MENU \"FILLS THE ALL HEIGHT\" OPTION IS DISABLED."), _("OK")));
				}
				//setVariable("reloadGuiMenu", true);
			}
		});

	// Auto adjust menu with by font size
	auto menu_auto_width = std::make_shared<SwitchComponent>(window);
	menu_auto_width->setState(Settings::getInstance()->getBool("AutoMenuWidth"));
	addWithLabel(_("AUTO SIZE WIDTH BASED ON FONT SIZE"), menu_auto_width);
	addSaveFunc([menu_auto_width, window]
		{
			bool old_value = Settings::getInstance()->getBool("AutoMenuWidth");
			if (old_value != menu_auto_width->getState())
			{
				Settings::getInstance()->setBool("AutoMenuWidth", menu_auto_width->getState());
					if (menu_auto_width->getState() && Settings::getInstance()->getBool("MenusAllWidth"))
					{
						Settings::getInstance()->setBool("MenusAllWidth", false);
						window->pushGui(new GuiMsgBox(window, _("YOU SELECTED TO AUTO SIZE WIDTH, THE MENU \"FILLS THE ALL WIDTH\" OPTION IS DISABLED."), _("OK")));
					}
			}
		});

	// menus all height
	auto menusAllHeight = std::make_shared<SwitchComponent>(window);
	menusAllHeight->setState(Settings::getInstance()->getBool("MenusAllHeight"));
	addWithLabel(_("FILLS THE ALL HEIGHT"), menusAllHeight);
	addSaveFunc([menusAllHeight, window]
		{
			bool old_value = Settings::getInstance()->getBool("MenusAllHeight");
			if (old_value != menusAllHeight->getState())
			{
				Settings::getInstance()->setBool("MenusAllHeight", menusAllHeight->getState());
				if (menusAllHeight->getState() && Settings::getInstance()->getBool("MenusOnDisplayTop"))
				{
					Settings::getInstance()->setBool("MenusOnDisplayTop", false);
					window->pushGui(new GuiMsgBox(window, _("YOU SELECTED TO FILLS THE ALL HEIGHT, THE MENU \"ON THE TOP OF THE DISPLAY\" OPTION IS DISABLED."), _("OK")));
				}
			}
		});

	// menus all width
	auto menusAllWith = std::make_shared<SwitchComponent>(window);
	menusAllWith->setState(Settings::getInstance()->getBool("MenusAllWidth"));
	addWithLabel(_("FILLS THE ALL WIDTH"), menusAllWith);
	addSaveFunc([menusAllWith, window]
		{
			bool old_value = Settings::getInstance()->getBool("MenusAllWidth");
			if (old_value != menusAllWith->getState())
			{
				Settings::getInstance()->setBool("MenusAllWidth", menusAllWith->getState());
				if (menusAllWith->getState() && Settings::getInstance()->getBool("AutoMenuWidth"))
				{
					Settings::getInstance()->setBool("AutoMenuWidth", false);
					window->pushGui(new GuiMsgBox(window, _("YOU SELECTED TO FILLS THE ALL WIDTH, THE MENU \"AUTO SIZE WIDTH BASED ON FONT SIZE\" OPTION IS DISABLED."), _("OK")));
				}
			}
		});

/*
	onFinalize([pthis, window]
	{
		if (getVariable("reloadGuiMenu"))
		{
			delete pthis;
			window->pushGui(new GuiMenu(window, false));
		}
	});
*/
}
