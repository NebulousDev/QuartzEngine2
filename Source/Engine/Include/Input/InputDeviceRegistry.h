#pragma once

#include "../EngineAPI.h"
#include "InputDevice.h"
#include "Types/Map.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API InputDeviceRegistry
	{
	private:
		Array<InputDevice> mAllDevices;

	public:
		InputDevice&	RegisterDevice(const InputDeviceInfo& deviceInfo, const InputDeviceCallbacks& callbacks);
		void			UnregisterDevice(const InputDevice& device);

		inline const Array<InputDevice>& GetDevices() const { return mAllDevices; }
	};
}