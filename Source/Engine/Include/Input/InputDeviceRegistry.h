#pragma once

#include "../EngineAPI.h"
#include "InputDevice.h"
#include "Types/Map.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API InputDeviceRegistry
	{
	private:
		Array<InputDevice>		mAllDevices;
		const InputMouse*		mPrimaryMouse;
		const InputKeyboard*	mPrimaryKeyboard;
		const InputController*	mPrimaryController;

	public:
		InputDevice&	RegisterDevice(const InputDeviceInfo& deviceInfo, const InputDeviceCallbacks& callbacks);
		void			UnregisterDevice(const InputDevice& device);

		inline const InputMouse*		GetPrimaryMouse() const { return mPrimaryMouse; }
		inline const InputKeyboard*		GetPrimaryKeyboard() const { return mPrimaryKeyboard; }
		inline const InputController*	GetPrimaryController() const { return mPrimaryController; }

		inline void SetPrimaryMouse(const InputMouse& mouse) { mPrimaryMouse = &mouse; }
		inline void SetPrimaryKeyboard(const InputKeyboard& keyboard) { mPrimaryKeyboard = &keyboard; }
		inline void SetPrimaryController(const InputController& controller) { mPrimaryController = &controller; }

		inline const Array<InputDevice>& GetDevices() const { return mAllDevices; }
	};
}