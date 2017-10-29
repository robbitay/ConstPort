/*
File:   appDynamicColor.h
Author: Taylor Robbins
Date:   10\29\2017
Description:
	** The DynamicColor_t is an array of colors meant to be used with
	** buttons or other interactive UI elements that need to change
	** colors when the hover moves their mouse over them and clicks on
	** them. It can be created as easily as a regular color with NewDynamicColor(Color_t)
	** which will set all states to the same color. The real convenience comes
	** when you want to define two or three of the states but not all of them.
	** There are lots of variants to the NewDynamicColor function that can be used
	** in many of these cases so you don't have to mention colors more than once
*/

typedef enum
{
	ColorState_Idle = 0,
	ColorState_Hover,
	ColorState_Active,
	ColorState_ActiveHover,
	ColorState_Pressed,
} ColorState_t;

union DynamicColor_t
{
	Color_t states[5];
	struct
	{
		Color_t idle;
		Color_t hover;
		Color_t active;
		Color_t activeHover;
		Color_t pressed;
	};
	struct
	{
		Color_t i;
		Color_t h;
		Color_t a;
		Color_t o;
		Color_t p;
	};
	
	Color_t operator [](ColorState_t state)
	{
		return states[state];
	}
};

DynamicColor_t DynamicColor(Color_t all)
{
	DynamicColor_t result = {};
	result.idle        = all;
	result.hover       = all;
	result.active      = all;
	result.activeHover = all;
	result.pressed     = all;
	return result;
}
#define DynamicColor_a(all) DynamicColor(all)

DynamicColor_t DynamicColor(Color_t idle, Color_t active)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = idle;
	result.active      = active;
	result.activeHover = active;
	result.pressed     = active;
	return result;
}
#define DynamicColor_ia(idle, active) DynamicColor(idle, active)

DynamicColor_t DynamicColor(Color_t idle, Color_t active, Color_t pressed)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = idle;
	result.active      = active;
	result.activeHover = active;
	result.pressed     = pressed;
	return result;
}
#define DynamicColor_iap(idle, active, pressed) DynamicColor(idle, active, pressed)

DynamicColor_t DynamicColor(Color_t idle, Color_t hover, Color_t active, Color_t pressed)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = hover;
	result.active      = active;
	result.activeHover = active;
	result.pressed     = pressed;
	return result;
}
#define DynamicColor_ihap(idle, hover, active, pressed) DynamicColor(idle, hover, active, pressed)

DynamicColor_t DynamicColor(Color_t idle, Color_t hover, Color_t active, Color_t activeHover, Color_t pressed)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = hover;
	result.active      = active;
	result.activeHover = activeHover;
	result.pressed     = pressed;
	return result;
}
#define DynamicColor_ihaop(idle, hover, active, activeHover, pressed) DynamicColor(idle, hover, active, activeHover, pressed)

DynamicColor_t DynamicColor_ih(Color_t idle, Color_t hover)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = hover;
	result.active      = idle;
	result.activeHover = hover;
	result.pressed     = idle;
	return result;
}
DynamicColor_t DynamicColor_ihp(Color_t idle, Color_t hover, Color_t pressed)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = hover;
	result.active      = idle;
	result.activeHover = hover;
	result.pressed     = pressed;
	return result;
}
DynamicColor_t DynamicColor_iha(Color_t idle, Color_t hover, Color_t active)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = hover;
	result.active      = active;
	result.activeHover = active;
	result.pressed     = active;
	return result;
}
DynamicColor_t DynamicColor_iaop(Color_t idle, Color_t active, Color_t activeHover, Color_t pressed)
{
	DynamicColor_t result = {};
	result.idle        = idle;
	result.hover       = idle;
	result.active      = active;
	result.activeHover = activeHover;
	result.pressed     = pressed;
	return result;
}

