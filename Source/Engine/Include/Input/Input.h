#pragma once

#include "../EngineAPI.h"
#include "Math/Math.h"
#include "Types/String.h"
#include "Types/Map.h"

#include "InputDevice.h"
#include "InputAction.h"

#include <functional>

#define INPUT_KEYBOARD_ANY		nullptr
#define INPUT_MOUSE_ANY			nullptr
#define INPUT_CONTROLLER_ANY	nullptr

namespace Quartz
{
	class Window;

	template<typename Scope>
	using ScopedOnAxisInputFunc = void (Scope::*)(Vec2f direction, InputActions actions);
	using OnAxisInputFunc = void (*)(Vec2f direction, InputActions actions);

	template<typename Scope>
	using ScopedOnButtonInputFunc = void (Scope::*)(float value, InputActions actions);
	using OnButtonInputFunc = void (*)(float value, InputActions actions);

	struct InputAxisState
	{
		Vec2f			direction;
		InputActions	actions;
	};

	struct InputButtonState
	{
		float			value;
		InputActions	actions;
	};

	class QUARTZ_ENGINE_API Input
	{
	private:
		struct InputAxisFunctorBase { inline virtual void VFT_Filler() {}; };

		struct InputAxisFunctor : public InputAxisFunctorBase
		{
			void* pInstance;
			OnAxisInputFunc inputFunc;

			inline virtual void Call(Vec2f direction, InputActions actions)
			{
				inputFunc(direction, actions);
			}
		};

		template<typename Scope>
		struct ScopedInputAxisFunctor : public InputAxisFunctor
		{
			inline void Call(Vec2f direction, InputActions actions) override
			{
				ScopedOnAxisInputFunc<Scope>* pfunc =
					(ScopedOnAxisInputFunc<Scope>*)reinterpret_cast<void**>(&inputFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(direction, actions);
			}
		};

		struct InputButtonFunctorBase { inline virtual void VFT_Filler() {}; };

		struct InputButtonFunctor : public InputButtonFunctorBase
		{
			void* pInstance;
			OnButtonInputFunc inputFunc;

			inline virtual void Call(float value, InputActions actions)
			{
				inputFunc(value, actions);
			}
		};

		template<typename Scope>
		struct ScopedInputButtonFunctor : public InputButtonFunctor
		{
			inline void Call(float value, InputActions actions) override
			{
				ScopedOnButtonInputFunc<Scope>* pfunc =
					(ScopedOnButtonInputFunc<Scope>*)reinterpret_cast<void**>(&inputFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(value, actions);
			}
		};

	public:
		enum InputType
		{
			INPUT_TYPE_AXIS,
			INPUT_TYPE_BUTTON
		};

		struct InputMapping
		{
			InputDevice*	pDevice;
			uInt64			input;
			InputType		type;
			InputActions	actions;
		};

	private:
		Map<InputMapping, String>				mMappings;
		Map<String, Array<InputAxisFunctor>>	mAxisFunctors;
		Map<String, Array<InputButtonFunctor>>	mButtonFunctors;
		Map<String, InputAxisState>				mAxisStates;
		Map<String, InputButtonState>			mButtonStates;

	public:
		void MapMouseAxis(const String& mapName, InputMouse* pMouse, InputActions actions);
		void MapControllerAxis(const String& mapName, InputController* pController, uInt64 joystick, InputActions actions);

		void MapMouseButton(const String& mapName, InputMouse* pMouse, uInt64 button, InputActions actions);
		void MapKeyboardButton(const String& mapName, InputKeyboard* pKeyboard, uInt64 key, InputActions actions);
		void MapControllerButton(const String& mapName, InputController* pController, uInt64 button, InputActions actions);

		template<typename Scope>
		void RegisterOnAxisInput(const String& mapName, ScopedOnAxisInputFunc<Scope> inputFunc, Scope* pInstance)
		{
			ScopedInputAxisFunctor<Scope> axisFunctor = {};
			axisFunctor.pInstance = pInstance;
			axisFunctor.inputFunc = inputFunc;

			auto& it = mAxisFunctors.Find(mapName);
			if (it != mAxisFunctors.End())
			{
				it->value.PushBack(axisFunctor);
			}
			else
			{
				auto& functors = mAxisFunctors.Put(mapName, Array<InputAxisFunctor>());
				functors.PushBack(axisFunctor);
			}
		}

		inline void RegisterOnAxisInput(const String& mapName, OnAxisInputFunc inputFunc)
		{
			InputAxisFunctor axisFunctor = {};
			axisFunctor.inputFunc = inputFunc;

			auto& it = mAxisFunctors.Find(mapName);
			if (it != mAxisFunctors.End())
			{
				it->value.PushBack(axisFunctor);
			}
			else
			{
				auto& functors = mAxisFunctors.Put(mapName, Array<InputAxisFunctor>());
				functors.PushBack(axisFunctor);
			}
		}

		template<typename Scope>
		void RegisterOnButtonInput(const String& mapName, ScopedOnButtonInputFunc<Scope> inputFunc, Scope* pInstance)
		{
			ScopedInputButtonFunctor<Scope> buttonFunctor = {};
			buttonFunctor.pInstance = pInstance;
			buttonFunctor.inputFunc = inputFunc;

			auto& it = mButtonFunctors.Find(mapName);
			if (it != mButtonFunctors.End())
			{
				it->value.PushBack(buttonFunctor);
			}
			else
			{
				auto& functors = mButtonFunctors.Put(mapName, Array<InputButtonFunctor>());
				functors.PushBack(buttonFunctor);
			}
		}

		inline void RegisterOnButtonInput(const String& mapName, OnButtonInputFunc inputFunc)
		{
			InputButtonFunctor buttonFunctor = {};
			buttonFunctor.inputFunc = inputFunc;

			auto& it = mButtonFunctors.Find(mapName);
			if (it != mButtonFunctors.End())
			{
				it->value.PushBack(buttonFunctor);
			}
			else
			{
				auto& functors = mButtonFunctors.Put(mapName, Array<InputButtonFunctor>());
				functors.PushBack(buttonFunctor);
			}
		}

		void SendAxisInput(InputDevice* pInputDevice, uInt64 joystick, InputActions actions, Vec2f direction);
		void SendButtonInput(InputDevice* pInputDevice, uInt64 button, InputActions actions, float value);

		bool SetMousePosition(const InputMouse& inputMouse, Vec2i absPosition);
		bool GetMousePosition(const InputMouse& inputMouse, Vec2i& outAbsPosition);
		bool SetMouseHidden(const Window& windowContext, const InputMouse& inputMouse, bool hidden);
		bool SetMouseBounds(const Window& windowContext, const InputMouse& inputMouse, Bounds2i absBounds, bool enabled);

		const InputAxisState*	GetAxisState(const String& mapName) const;
		const InputButtonState*	GetButtonState(const String& mapName) const;
	};
}