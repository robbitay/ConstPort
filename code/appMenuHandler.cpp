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
	menu->titleBarRec = NewRec(
		menu->drawRec.x, 
		menu->drawRec.y, 
		menu->drawRec.width, 
		menu->titleBarSize
	);
	menu->usableRec = NewRec(
		menu->drawRec.x, 
		menu->drawRec.y + menu->titleBarSize,
		menu->drawRec.width - GC->menuBorderThickness*2,
		menu->drawRec.height - menu->titleBarSize - GC->menuBorderThickness
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
	
	newMenu->specialPntr = specialPntr;
	newMenu->updateFunctionPntr = (void*)updateFunctionPntr;
	newMenu->renderFunctionPntr = (void*)renderFunctionPntr;
	
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

Menu_t* GetMenuAtPoint(MenuHandler_t* menuHandlerPntr, v2 point)
{
	for (i32 mIndex = 0; mIndex < menuHandlerPntr->menuList.numItems; mIndex++)
	{
		Menu_t* menuPntr = (Menu_t*)LinkedListGetItem(&menuHandlerPntr->menuList, mIndex);
		
		if (menuPntr->alive && menuPntr->show && IsInsideRec(menuPntr->drawRec, point))
		{
			return menuPntr;
		}
	}
	
	return nullptr;
}

void MenuHandlerUpdate(MenuHandler_t* menuHandler)
{
	for (i32 mIndex = 0; mIndex < menuHandler->menuList.numItems; mIndex++)
	{
		Menu_t* menuPntr = (Menu_t*)LinkedListGetItem(&menuHandler->menuList, mIndex);
		
		if (menuPntr->alive)
		{
			UpdateMenuRecs(menuPntr);
			rec closeRec = NewRec(
				menuPntr->titleBarRec.x + menuPntr->titleBarRec.width - menuPntr->titleBarRec.height, 
				menuPntr->titleBarRec.y,
				menuPntr->titleBarRec.height,
				menuPntr->titleBarRec.height);
			closeRec = RecInflate(closeRec, -5);
			
			if (menuPntr->allowMovement)
			{
				if (menuHandler->activeMenuIndex == mIndex ||
					(!menuHandler->movingMenu && 
					!menuHandler->resizingMenuVert && !menuHandler->resizingMenuHor))
				{
					if (input->buttons[MouseButton_Left].isDown)
					{
						if (input->buttons[MouseButton_Left].transCount > 0)
						{
							if (IsInsideRec(menuPntr->titleBarRec, RenderMousePos))
							{
								menuHandler->movingMenu = true;
								menuHandler->resizingMenuVert = false;
								menuHandler->resizingMenuHor = false;
								menuHandler->grabMenuOffset = RenderMousePos - menuPntr->titleBarRec.topLeft;
								menuHandler->activeMenuIndex = mIndex;
							}
						}
						else if (menuHandler->movingMenu)
						{
							menuPntr->drawRec.topLeft = RenderMousePos - menuHandler->grabMenuOffset;
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
			
			if (!input->buttons[MouseButton_Left].isDown && input->buttons[MouseButton_Left].transCount > 0 &&
				input->mouseMaxDist[MouseButton_Left] / GUI_SCALE < 10)
			{
				if (ClickedOnRec(closeRec))
				{
					HandleButton(MouseButton_Left);
					menuPntr->show = false;
				}
			}
			
			if (menuPntr->drawRec.x < 0)
				menuPntr->drawRec.x = 0;
			if (menuPntr->drawRec.x > RenderScreenSize.x - menuPntr->drawRec.width)
				menuPntr->drawRec.x = RenderScreenSize.x - menuPntr->drawRec.width;
			if (menuPntr->drawRec.y > RenderScreenSize.y - menuPntr->drawRec.height)
				menuPntr->drawRec.y = RenderScreenSize.y - menuPntr->drawRec.height;
			if (menuPntr->drawRec.y < 0)
				menuPntr->drawRec.y = 0;
			
			UpdateMenuRecs(menuPntr);
			
			if (menuPntr->updateFunctionPntr != nullptr)
			{
				((MenuUpdate_f*)menuPntr->updateFunctionPntr)(menuHandler, menuPntr);
			}
			
			UpdateMenuRecs(menuPntr);
		}
		else
		{
			//TODO: Remove it from the list
		}
	}
}

void MenuHandlerDrawMenus(MenuHandler_t* menuHandler)
{
	for (i32 mIndex = 0; mIndex < menuHandler->menuList.numItems; mIndex++)
	{
		Menu_t* menuPntr = (Menu_t*)LinkedListGetItem(&menuHandler->menuList, mIndex);
		
		if (menuPntr->show)
		{
			UpdateMenuRecs(menuPntr);
			
			
			RsDrawGradient(menuPntr->drawRec, GC->colors.windowBackground1, GC->colors.windowBackground2, Dir2_Down);
			
			RsDrawRectangle(menuPntr->titleBarRec, GC->colors.windowTitlebar);
			
			if (menuPntr->titleBarSize > 0)
			{
				rec closeRec = NewRec(
					menuPntr->titleBarRec.x + menuPntr->titleBarRec.width - menuPntr->titleBarRec.height, 
					menuPntr->titleBarRec.y,
					menuPntr->titleBarRec.height,
					menuPntr->titleBarRec.height);
				closeRec = RecInflate(closeRec, -5);
				
				Color_t exitButtonColor = GC->colors.windowExit;
				if (IsInsideRec(closeRec, RenderMousePos))
				{
					exitButtonColor = ColorMultiply(exitButtonColor, {Color_LightGrey});
				}
				
				RsDrawRectangle(closeRec, exitButtonColor);
				
				rec shrunkRec = RecInflate(closeRec, -2);
				RsDrawLine(
					NewVec2(shrunkRec.x, shrunkRec.y + shrunkRec.height),
					NewVec2(shrunkRec.x + shrunkRec.width, shrunkRec.y),
					2,
					GC->colors.windowExitIcon
				);
				RsDrawLine(
					NewVec2(shrunkRec.x, shrunkRec.y),
					NewVec2(shrunkRec.x + shrunkRec.width, shrunkRec.y + shrunkRec.height),
					2,
					GC->colors.windowExitIcon
				);
			}
			
			RsDrawRectangle(NewRec(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y + menuPntr->titleBarRec.height - GC->menuBorderThickness,
				menuPntr->drawRec.width,
				(r32)GC->menuBorderThickness),
				GC->colors.windowTitlebar);
			
			RsDrawRectangle(NewRec(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y,
				menuPntr->drawRec.width,
				(r32)GC->menuBorderThickness),
				GC->colors.windowOutline);
			RsDrawRectangle(NewRec(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y,
				(r32)GC->menuBorderThickness,
				menuPntr->drawRec.height),
				GC->colors.windowOutline);
			RsDrawRectangle(NewRec(
				menuPntr->drawRec.x,
				menuPntr->drawRec.y + menuPntr->drawRec.height - GC->menuBorderThickness,
				menuPntr->drawRec.width,
				(r32)GC->menuBorderThickness),
				GC->colors.windowOutline);
			RsDrawRectangle(NewRec(
				menuPntr->drawRec.x + menuPntr->drawRec.width - GC->menuBorderThickness,
				menuPntr->drawRec.y,
				(r32)GC->menuBorderThickness,
				menuPntr->drawRec.height),
				GC->colors.windowOutline);
			
			if (menuPntr->renderFunctionPntr != nullptr)
			{
				((MenuRender_f*)menuPntr->renderFunctionPntr)(menuHandler, menuPntr);
			}
		}
	}
}
