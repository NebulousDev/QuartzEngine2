#include "Input/Input.h"

namespace Quartz
{
	template<>
	hash64 Hash<Input::InputMapping>(const Input::InputMapping& value)
	{
		return Hash<uInt64>(32101010123 + ((uInt64)value.type << 24) + ((uInt64)value.input << 16) + 1);
	}

	bool operator==(const Input::InputMapping& mapping0, const Input::InputMapping& mapping1)
	{
		return ((mapping0.pDevice == mapping1.pDevice) || 
			mapping0.pDevice == nullptr || mapping1.pDevice == nullptr) &&
			mapping0.input == mapping1.input &&
			mapping0.type == mapping1.type &&
			(mapping0.actions & mapping1.actions);
	}

	void Input::MapMouseAxis(const String& mapName, InputMouse* pMouse, InputActions actions)
	{
		InputMapping mapping = {};
		mapping.pDevice = pMouse;
		mapping.input	= 0;
		mapping.type	= INPUT_TYPE_AXIS;
		mapping.actions = actions;

		mMappings.Put(mapping, mapName);

		InputAxisState state = {};
		state.direction = { 0.0f, 0.0f };
		state.actions	= INPUT_ACTION_NONE;

		mAxisStates.Put(mapName, state);
	}

	void Input::MapControllerAxis(const String& mapName, InputController* pController, uInt64 joystick, InputActions actions)
	{
		InputMapping mapping = {};
		mapping.pDevice = pController;
		mapping.input	= joystick;
		mapping.type	= INPUT_TYPE_AXIS;
		mapping.actions = actions;

		mMappings.Put(mapping, mapName);

		InputAxisState state = {};
		state.direction = { 0.0f, 0.0f };
		state.actions	= INPUT_ACTION_NONE;

		mAxisStates.Put(mapName, state);
	}

	void Input::MapMouseButton(const String& mapName, InputMouse* pMouse, uInt64 button, InputActions actions)
	{
		InputMapping mapping = {};
		mapping.pDevice = pMouse;
		mapping.input	= button;
		mapping.type	= INPUT_TYPE_BUTTON;
		mapping.actions = actions;

		mMappings.Put(mapping, mapName);

		InputButtonState state = {};
		state.value		= 0.0f;
		state.actions	= INPUT_ACTION_NONE;

		mButtonStates.Put(mapName, state);
	}

	void Input::MapKeyboardButton(const String& mapName, InputKeyboard* pKeyboard, uInt64 key, InputActions actions)
	{
		InputMapping mapping = {};
		mapping.pDevice = pKeyboard;
		mapping.input	= key;
		mapping.type	= INPUT_TYPE_BUTTON;
		mapping.actions = actions;

		mMappings.Put(mapping, mapName);

		InputButtonState state = {};
		state.value		= 0.0f;
		state.actions	= INPUT_ACTION_NONE;

		mButtonStates.Put(mapName, state);
	}

	void Input::MapControllerButton(const String& mapName, InputController* pController, uInt64 button, InputActions actions)
	{
		InputMapping mapping = {};
		mapping.pDevice = pController;
		mapping.input	= button;
		mapping.type	= INPUT_TYPE_BUTTON;
		mapping.actions = actions;

		mMappings.Put(mapping, mapName);

		InputButtonState state = {};
		state.value		= 0.0f;
		state.actions	= INPUT_ACTION_NONE;

		mButtonStates.Put(mapName, state);
	}

	InputActions GetAxisActions(InputActions oldActions, InputActions newActions)
	{
		return newActions;
	}

	void Input::SendAxisInput(InputDevice* pInputDevice, uInt64 joystick, InputActions actions, Vec2f direction)
	{
		InputMapping mapping = {};
		mapping.pDevice = pInputDevice;
		mapping.input	= joystick;
		mapping.type	= INPUT_TYPE_AXIS;
		mapping.actions = INPUT_ACTION_ANY;

		auto& mapIt = mMappings.Find(mapping);

		if (mapIt == mMappings.End())
		{
			return; // No mappings found
		}

		auto& stateIt = mAxisStates.Find(mapIt->value);

		if (stateIt != mAxisStates.End())
		{
			InputAxisState& state = stateIt->value;
			state.direction = direction;
			state.actions	= GetAxisActions(state.actions, actions);
		}
		else
		{
			return; // Error
		}

		if (mapIt->key.actions & stateIt->value.actions)
		{
			for (InputAxisFunctor& functor : mAxisFunctors.Get(mapIt->value))
			{
				functor.Call(direction, actions);
			}
		}
	}

	InputActions GetButtonActions(InputActions oldActions, InputActions newActions)
	{
		if (oldActions & INPUT_ACTION_ANY_UP && newActions & INPUT_ACTION_ANY_DOWN || INPUT_ACTION_NONE)
		{
			return newActions | INPUT_ACTION_PRESSED;
		}
		
		if (oldActions & INPUT_ACTION_ANY_DOWN && newActions & INPUT_ACTION_ANY_UP)
		{
			return newActions | INPUT_ACTION_RELEASED;
		}

		return newActions;
	}

	void Input::SendButtonInput(InputDevice* pInputDevice, uInt64 button, InputActions actions, float value)
	{
		InputMapping mapping = {};
		mapping.pDevice = pInputDevice;
		mapping.input	= button;
		mapping.type	= INPUT_TYPE_BUTTON;
		mapping.actions = INPUT_ACTION_ANY;

		auto& mapIt = mMappings.Find(mapping);

		if (mapIt == mMappings.End())
		{
			return; // No mappings found
		}

		auto& stateIt = mButtonStates.Find(mapIt->value);

		if (stateIt != mButtonStates.End())
		{
			InputButtonState& state = stateIt->value;
			state.value		= value;
			state.actions	= GetButtonActions(state.actions, actions);
		}
		else
		{
			return; // Error
		}

		if (mapIt->key.actions & stateIt->value.actions)
		{
			for (InputButtonFunctor& functor : mButtonFunctors.Get(mapIt->value))
			{
				functor.Call(value, actions);
			}
		}
	}

	bool Input::SetMousePosition(const InputMouse& inputMouse, Vec2i absPosition)
	{
		return inputMouse.GetCallbacks().onMouseSetPositionFunc(inputMouse, absPosition);
	}

	bool Input::GetMousePosition(const InputMouse& inputMouse, Vec2i& outAbsPosition)
	{
		return inputMouse.GetCallbacks().onMouseGetPositionFunc(inputMouse, outAbsPosition);
	}

	bool Input::SetMouseHidden(const Window& windowContext, const InputMouse& inputMouse, bool hidden)
	{
		return inputMouse.GetCallbacks().onMouseSetHiddenFunc(windowContext, inputMouse, hidden);
	}

	bool Input::SetMouseBounds(const Window& windowContext, const InputMouse& inputMouse, Bounds2i absBounds, bool enabled)
	{
		return inputMouse.GetCallbacks().onMouseSetBoundsFunc(windowContext, inputMouse, absBounds, enabled);
	}

	const InputAxisState* Input::GetAxisState(const String& mapName) const
	{
		auto& stateIt = mAxisStates.Find(mapName);

		if (stateIt != mAxisStates.End())
		{
			return &stateIt->value;
		}
		else
		{
			return nullptr; // No mappings found
		}
	}

	const InputButtonState* Input::GetButtonState(const String& mapName) const
	{
		auto& stateIt = mButtonStates.Find(mapName);

		if (stateIt != mButtonStates.End())
		{
			return &stateIt->value;
		}
		else
		{
			return nullptr; // No mappings found
		}
	}
}