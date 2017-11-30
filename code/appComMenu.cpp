/*
File:   appComMenu.cpp
Author: Taylor Robbins
Date:   11\28\2017
Description: 
	** Holds functions that handle the COM Menu that is presented to
	** the user when they need to choose a COM port to open 

#included from app.cpp
*/

void ComMenuInitialize(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	ClearPointer(comMenu);
	
	
}

void ComMenuShow(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (!comMenu->open == false)
	{
		DEBUG_WriteLine("Showing COM Menu");
		//TODO: Any opening initialization?
	}
	comMenu->open = true;
}

void ComMenuHide(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (!comMenu->open == false)
	{
		DEBUG_WriteLine("Hiding COM Menu");
		//TODO: Any opening initialization?
	}
	comMenu->open = false;
}

void ComMenuToggle(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (comMenu->open) { ComMenuHide(comMenu); }
	else               { ComMenuShow(comMenu); }
}

void ComMenuUpdate(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	
	// +==============================+
	// |     Update Anim Percent      |
	// +==============================+
	if (comMenu->open)
	{
		if (comMenu->animPercent < 1.0f)
		{
			comMenu->animPercent += COM_MENU_OPEN_SPEED;
			if (comMenu->animPercent >= 1.0f) { comMenu->animPercent = 1.0f; }
		}
	}
	else
	{
		if (comMenu->animPercent > 0.0f)
		{
			comMenu->animPercent -= COM_MENU_OPEN_SPEED;
			if (comMenu->animPercent <= 0.0f) { comMenu->animPercent = 0.0f; }
		}
	}
	
	// +==============================+
	// |    Update Menu Rectangle     |
	// +==============================+
	comMenu->drawRec = NewRec(Vec2_Zero, RenderScreenSize);
	comMenu->drawRec = RecInflateY(comMenu->drawRec, -(50));
	if (comMenu->animPercent < 0.5f)
	{
		comMenu->drawRec.y += comMenu->drawRec.height/2;
		comMenu->drawRec.height = 1.0f;
		r32 openPercent = Ease(EasingStyle_QuadraticOut, comMenu->animPercent * 2.0f);
		comMenu->drawRec = RecInflateX(comMenu->drawRec, -(comMenu->drawRec.width/2) * (1.0f - openPercent));
	}
	else if (comMenu->animPercent < 1.0f)
	{
		r32 openPercent = Ease(EasingStyle_QuadraticOut, (comMenu->animPercent - 0.5f) * 2.0f);
		comMenu->drawRec = RecInflateY(comMenu->drawRec, -(comMenu->drawRec.height/2) * (1.0f - openPercent));
	}
}

void ComMenuDraw(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	
	if (comMenu->animPercent > 0)
	{
		Color_t menuBackColor1 = GC->colors.windowBackground1;
		Color_t menuBackColor2 = GC->colors.windowBackground2;
		Color_t menuBorderColor = GC->colors.windowOutline;
		// RsDrawRectangle(comMenu->drawRec, menuBackColor);
		RsDrawGradient(comMenu->drawRec, menuBackColor1, menuBackColor2, Dir2_Down);
		rec dropShadowRec = comMenu->drawRec;
		dropShadowRec.height = 15;
		dropShadowRec.y = comMenu->drawRec.y + comMenu->drawRec.height;
		RsDrawGradient(dropShadowRec, ColorTransparent(NewColor(Color_Black), 0.5f), NewColor(Color_TransparentBlack), Dir2_Down);
		dropShadowRec.y = comMenu->drawRec.y - dropShadowRec.height;
		RsDrawGradient(dropShadowRec, ColorTransparent(NewColor(Color_Black), 0.5f), NewColor(Color_TransparentBlack), Dir2_Up);
		
		rec comListRec = comMenu->drawRec;
		comListRec.width = (r32)RoundR32(2*comListRec.width/5);
		comListRec.x -= 1;
		RsDrawButton(comListRec, GC->colors.textBackground, GC->colors.windowOutline, 1.0f);
		
		RsDrawButton(RecInflateX(comMenu->drawRec, 1), NewColor(Color_TransparentBlack), menuBorderColor, 1.0f);
	}
}
