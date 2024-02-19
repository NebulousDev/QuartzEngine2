#pragma once

namespace Quartz
{
	enum InputAction
	{
		/* General Actions */
		INPUT_ACTION_NONE				= 0x000,

		/* Button Actions */
		INPUT_ACTION_UP					= 0x001,
		INPUT_ACTION_DOWN				= 0x002,
		INPUT_ACTION_PRESSED			= 0x004,
		INPUT_ACTION_RELEASED			= 0x008,
		INPUT_ACTION_REPEAT				= 0x010,

		/* Axis Actions */
		INPUT_ACTION_ACTIVE				= 0x020,
		INPUT_ACTION_MOVE				= INPUT_ACTION_ACTIVE,
		INPUT_ACTION_SET				= 0x040,

		/* Mouse Wheel Actions */
		INPUT_ACTION_MOUSE_WHEEL_UP		= 0x080,
		INPUT_ACTION_MOUSE_WHEEL_DOWN	= 0x100,

		/* Combo Actions */
		INPUT_ACTION_ANY				= 0xFFFF,
		INPUT_ACTION_ANY_UP				= (INPUT_ACTION_UP | INPUT_ACTION_RELEASED),
		INPUT_ACTION_ANY_DOWN			= (INPUT_ACTION_DOWN | INPUT_ACTION_PRESSED | INPUT_ACTION_REPEAT),
		INPUT_ACTION_MOUSE_WHEEL_ANY	= (INPUT_ACTION_MOUSE_WHEEL_UP | INPUT_ACTION_MOUSE_WHEEL_DOWN)
	};

	typedef flags8 InputActions;
}