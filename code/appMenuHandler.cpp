/*
File:   appMenuHanadler.cpp
Author: Taylor Robbins
Date:   06\15\2017
Description: 
	** Holds functions that handle menu movement, interaction,
	** and rendering to the screen

#included from app.cpp 
*/

void InitializeMenuHandler(MenuHandler_t* menuHandlerPntr, MemoryArena_t* memArena)
{
	ClearPointer(menuHandlerPntr);
	
	menuHandlerPntr->memArena = memArena;
	CreateLinkedList(&menuHandlerPntr->menuList);
	
	menuHandlerPntr->closeTexture = LoadTexture("Resources/Sprites/close.png");	
}

void UpdateMenuRecs(Menu_t* menu)
{
	menu->titleBarRec = NewRectangle(
		menu->drawRec.x, 
		menu->drawRec.y, 
		menu->drawRec.width, 
		menu->titleBarSize
	);
	menu->usableRec = NewRectangle(
		menu->drawRec.x, 
		menu->drawRec.y + menu->titleBarSize,
		menu->drawRec.width - MENU_BORDER_WIDTH*2,
		menu->drawRec.height - menu->titleBarSize - MENU_BORDER_WIDTH
	);
}

Menu_t* AddMenu(MenuHandler_t* menuHandlerPntr, const char* name, rec drawRec, 
	MenuUpdate_f* updateFunctionPntr = nullptr, MenuRender_f* renderFunctionPntr = nullptr, void* specialPntr = nullptr)
{
	Menu_t* newMenu = LinkedPushStruct(&menuHandlerPntr->menuList, menuHandlerPntr->memArena, Menu_t);
	
	ClearArray(newMenu->name);
	strncpy(newMenu->name, name, ArrayCount(newMenu->name)-1);
	newMenu->drawRec = drawRec;
	
	newMenu->alive = true;
	newMenu->show = true;
	newMenu->allowMovement = true;
	newMenu->allowResize = false;
	newMenu->titleBarSize = MENU_DEFAULT_TITLEBAR_SIZE;
	newMenu->backgroundColor = Color_WindowBackground;
	newMenu->borderColor = Color_WindowBorder;
	newMenu->titleBarColor = Color_WindowTitleBar;
	
	newMenu->specialPntr = specialPntr;
	newMenu->updateFunctionPntr = updateFunctionPntr;
	newMenu->renderFunctionPntr = renderFunctionPntr;
	
	UpdateMenuRecs(newMenu);
	
	return newMenu;
}

Menu_t* GetMenuByName(MenuHandler_t* menuHandlerPntr, const char* name)
{
	for (i32 mIndex = 0; mIndex < menuHandlerPntr->menuList.numItems; mIndex++)
	{
		Menu_t* menuPntr = (Menu_t*)LinkedListGetItem(&menuHandlerPntr->menuList, mIndex);
		
		if (strcmp(name, menuPntr->name) == 0)
		{
			return menuPntr;
		}
	}
	
	return nullptr;
}

void MenuHandlerUpdate(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, MenuHandler_t* menuHandler)
{
	for (i32 mIndex = 0; mIndex < menuHandler->menuList.numItems; mIndex++)
	{
		Menu_t* menuPntr = (Menu_t*)LinkedListGetItem(&menuHandler->menuList, mIndex);
		
		if (menuPntr->alive)
		{
			UpdateMenuRecs(menuPntr);
			rec closeRec = NewRectangle(
				menuPntr->titleBarRec.x + menuPntr->titleBarRec.width - menuPntr->titleBarRec.height, 
				menuPntr->titleBarRec.y,
				menuPntr->titleBarRec.height,
				menuPntr->titleBarRec.height);
			closeRec = RectangleInflate(closeRec, -5);
			
			if (menuPntr->allowMovement)
			{
				if (menuHandler->activeMenuIndex == mIndex ||
					(!menuHandler->movingMenu && 
					!menuHandler->resizingMenuVert && !menuHandler->resizingMenuHor))
				{
					v2 mousePos = AppInput->mousePos;
					v2 mouseStart = AppInput->mouseStartPos[MouseButton_Left];
					r32 mouseMaxDist = AppInput->mouseMaxDist[MouseButton_Left];
					
					if (AppInput->buttons[MouseButton_Left].isDown)
					{
						if (AppInput->buttons[MouseButton_Left].transCount > 0)
						{
							if (IsInsideRectangle(mousePos, menuPntr->titleBarRec))
							{
								menuHandler->movingMenu = true;
								menuHandler->resizingMenuVert = false;
								menuHandler->resizingMenuHor = false;
								menuHandler->grabMenuOffset = mousePos - menuPntr->titleBarRec.topLeft;
								menuHandler->activeMenuIndex = mIndex;
							}
						}
						else if (menuHandler->movingMenu)
						{
							menuPntr->drawRec.topLeft = mousePos - menuHandler->grabMenuOffset;
						}
					}
					else
					{
						menuHandler->movingMenu = false;
						menuHandler->resizingMenuVert = false;
						menuHandler->resizingMenuHor = false;
					}
				}
			}
			
			if (!AppInput->buttons[MouseButton_Left].isDown && AppInput->buttons[MouseButton_Left].transCount > 0 &&
				AppInput->mouseMaxDist[MouseButton_Left] < 10)
			{
				if (IsInsideRectangle(AppInput->mousePos, closeRec) &&
					IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], closeRec))
				{
					menuPntr->show = false;
				}
			}
			
			if (menuPntr->drawRec.x < 0)
				menuPntr->drawRec.x = 0;
			if (menuPntr->drawRec.y < 0)
				menuPntr->drawRec.y = 0;
			if (menuPntr->drawRec.x > PlatformInfo->screenSize.x - menuPntr->drawRec.width)
				menuPntr->drawRec.x = PlatformInfo->screenSize.x - menuPntr->drawRec.width;
			if (menuPntr->drawRec.y > PlatformInfo->screenSize.y - menuPntr->drawRec.height)
				menuPntr->drawRec.y = PlatformInfo->screenSize.y - menuPntr->drawRec.height;
			
			UpdateMenuRecs(menuPntr);
			
			if (menuPntr->updateFunctionPntr != nullptr)
			{
				((MenuUpdate_f*)menuPntr->updateFunctionPntr)(PlatformInfo, AppInput, menuHandler, menuPntr);
			}
			
			UpdateMenuRecs(menuPntr);
		}
		else
		{
			//TODO: Remove it from the list
		}
	}
}

void MenuHandlerDrawMenus(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, RenderState_t* renderState, MenuHandler_t* menuHandler)
{
	for (i32 mIndex = 0; mIndex < menuHandler->menuList.numItems; mIndex++)
	{
		Menu_t* menuPntr = (Menu_t*)LinkedListGetItem(&menuHandler->menuList, mIndex);
		
		if (menuPntr->show)
		{
			UpdateMenuRecs(menuPntr);
			rec closeRec = NewRectangle(
				menuPntr->titleBarRec.x + menuPntr->titleBarRec.width - menuPntr->titleBarRec.height, 
				menuPntr->titleBarRec.y,
				menuPntr->titleBarRec.height,
				menuPntr->titleBarRec.height);
			closeRec = RectangleInflate(closeRec, -5);
			
			menuPntr->titleBarColor = Color_UiGray3;
			menuPntr->backgroundColor = Color_UiGray1;
			menuPntr->borderColor = Color_UiLightGray1;
			
			renderState->DrawGradient(menuPntr->drawRec, menuPntr->backgroundColor, Color_UiGray2, Direction2D_Down);
			
			renderState->DrawRectangle(menuPntr->titleBarRec, menuPntr->titleBarColor);
			
			renderState->BindTexture(&menuHandler->closeTexture);
			if (IsInsideRectangle(AppInput->mousePos, closeRec))
			{
				renderState->DrawTexturedRec(closeRec, {Color_Gray});
			}
			else
			{
				renderState->DrawTexturedRec(closeRec, {Color_White});
			}
			
			renderState->DrawRectangle(NewRectangle(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y + menuPntr->titleBarRec.height - MENU_BORDER_WIDTH,
				menuPntr->drawRec.width,
				MENU_BORDER_WIDTH),
				Color_UiGray4);
			
			renderState->DrawRectangle(NewRectangle(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y,
				menuPntr->drawRec.width,
				MENU_BORDER_WIDTH),
				menuPntr->borderColor);
			renderState->DrawRectangle(NewRectangle(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y,
				MENU_BORDER_WIDTH,
				menuPntr->drawRec.height),
				menuPntr->borderColor);
			renderState->DrawRectangle(NewRectangle(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y + menuPntr->drawRec.height - MENU_BORDER_WIDTH,
				menuPntr->drawRec.width,
				MENU_BORDER_WIDTH),
				menuPntr->borderColor);
			renderState->DrawRectangle(NewRectangle(
				menuPntr->drawRec.x + menuPntr->drawRec.width - MENU_BORDER_WIDTH,
				menuPntr->drawRec.y,
				MENU_BORDER_WIDTH,
				menuPntr->drawRec.height),
				menuPntr->borderColor);
			
			if (menuPntr->renderFunctionPntr != nullptr)
			{
				((MenuRender_f*)menuPntr->renderFunctionPntr)(PlatformInfo, AppInput, renderState, menuHandler, menuPntr);
			}
		}
	}
}
