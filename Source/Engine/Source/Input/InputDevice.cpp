#include "Input/InputDevice.h"

namespace Quartz
{
	InputDevice::InputDevice() : mDeviceInfo{}, mCallbacks{} { }

	InputDevice::InputDevice(const InputDeviceInfo& deviceInfo, const InputDeviceCallbacks& callbacks) :
		mDeviceInfo(deviceInfo), mCallbacks(callbacks) { }
}