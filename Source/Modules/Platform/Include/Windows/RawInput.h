#pragma once

#include "Windows/WinApi.h"
#include "../PlatformAPI.h"
#include "Input/InputDevice.h"
#include "Types/Array.h"
#include "Types/Map.h"

namespace Quartz
{
	struct RawInputDeviceInfo
	{
		StringW deviceId;
		StringW parentId;
		StringW deviceName;
		StringW vendorName;
		StringW className;
		StringW pathName;
		flags16 usagePage;
		flags16 usage;
		uInt32	axisCount;
		uInt32  buttonCount;
	};

	struct RawInputDevice
	{
		HANDLE				hDevice;
		InputDeviceType		deviceType;
		RawInputDeviceInfo	deviceInfo;
		InputDevice*		pInputDevice;
	};

	class QUARTZ_PLATFORM_API RawInput
	{
	private:
		Array<HANDLE>					mActiveIds;
		Map<uInt64, StringW>			mDeviceIdMap; // uInt64 ~= HANDLE
		Map<StringW, RawInputDevice>	mDeviceMap;

	private:
		void EnumerateDevices(Array<RAWINPUTDEVICELIST>& rawInputDeviceList);
		bool GetDeviceInfo(const RAWINPUTDEVICELIST& rawInputDevice, RawInputDeviceInfo& outDeviceInfo);
		bool RegisterUsage(uInt16 usagePage, uInt16 usage, DWORD flags);

	public:
		void Init();
		void PollConnections();
		void PollInput();
	};

	
}