#include "Input/InputDeviceRegistry.h"

namespace Quartz
{
	InputDevice& InputDeviceRegistry::RegisterDevice(const InputDeviceInfo& deviceInfo, const InputDeviceCallbacks& callbacks)
	{
		InputDevice& device = mAllDevices.PushBack(InputDevice(deviceInfo, callbacks));
		return device;
	}

	void InputDeviceRegistry::UnregisterDevice(const InputDevice& device)
	{
		mAllDevices.Remove(device);
	}
}