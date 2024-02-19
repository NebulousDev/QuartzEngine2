#pragma once

#include "../EngineAPI.h"
#include "Math/Math.h"
#include "Types/Types.h"
#include "Types/String.h"

namespace Quartz
{
	class Window;
	class InputDevice;

	using OnMouseSetPositionFunc	= bool (*)(const InputDevice& device, Vec2i absPosition);
	using OnMouseGetPositionFunc	= bool (*)(const InputDevice& device, Vec2i& outAbsPosition);
	using OnMouseSetHiddenFunc		= bool (*)(const Window& windowContext, const InputDevice& device, bool hidden);
	using OnMouseSetBoundsFunc		= bool (*)(const Window& windowContext, const InputDevice& device, Bounds2i absBounds, bool enabled);

	enum InputDeviceType
	{
		INPUT_DEVICE_TYPE_UNKNOWN,
		INPUT_DEVICE_TYPE_MOUSE,
		INPUT_DEVICE_TYPE_KEYBOARD,
		INPUT_DEVICE_TYPE_CONTROLLER
	};

	struct InputDeviceInfo
	{
		StringW			deviceName;
		StringW			devicePath;
		StringW			deviceVendor;
		InputDeviceType	deviceType;
		uInt32			axisCount;
		uInt32			buttonCount;
	};

	struct InputDeviceCallbacks
	{
		OnMouseSetPositionFunc	onMouseSetPositionFunc;
		OnMouseGetPositionFunc	onMouseGetPositionFunc;
		OnMouseSetHiddenFunc	onMouseSetHiddenFunc;
		OnMouseSetBoundsFunc	onMouseSetBoundsFunc;
	};

	class QUARTZ_ENGINE_API InputDevice
	{
	private:
		InputDeviceInfo			mDeviceInfo;
		InputDeviceCallbacks	mCallbacks;

	public:
		InputDevice();
		InputDevice(const InputDeviceInfo& deviceInfo, const InputDeviceCallbacks& callbacks);

		inline const StringW&		GetDeviceName() const { return mDeviceInfo.deviceName; }
		inline const StringW&		GetDevicePath() const { return mDeviceInfo.devicePath; }
		inline const StringW&		GetDeviceVendor() const { return mDeviceInfo.deviceVendor; }
		inline InputDeviceType		GetDeviceType() const { return mDeviceInfo.deviceType; }
		inline uInt32				GetAxisCount() const { return mDeviceInfo.axisCount; }
		inline uInt32				GetButtonCount() const { return mDeviceInfo.buttonCount; }
		inline InputDeviceCallbacks	GetCallbacks() const { return mCallbacks; }

		virtual void* GetNativeHandle() const { return nullptr; }

		inline friend bool operator==(const InputDevice& device0, const InputDevice& device1)
		{
			return device0.mDeviceInfo.deviceType == device1.mDeviceInfo.deviceType &&
				device0.mDeviceInfo.deviceName == device1.mDeviceInfo.deviceName;
		}
	};

	using InputMouse		= InputDevice;
	using InputKeyboard		= InputDevice;
	using InputController	= InputDevice;
}