#include "Windows/RawInput.h"

#include "Engine.h"
#include "Types/Array.h"
#include "Log.h"

#include <initguid.h>
#include <Cfgmgr32.h>
#include <Devpkey.h>

#pragma comment(lib, "Cfgmgr32")

#define INPUT_MAX_RAWINPUT_BUFFER_SIZE		64
#define INPUT_RAW_INPUT_ERROR				-1

#define INPUT_UNKNOWN_DEVICE_PATH_STRING	StringW(L"Unknown Device Path")
#define INPUT_UNKNOWN_DEVICE_NAME_STRING	StringW(L"Unknown Device Name")
#define INPUT_UNKNOWN_DEVICE_VENDOR_STRING	StringW(L"Unknown Device Vendor")
#define INPUT_UNKNOWN_DEVICE_CLASS_STRING	StringW(L"Unknown Device Class")
#define INPUT_UNKNOWN_DEVICE_SERIAL_STRING	StringW(L"Unknown Device Serial Number")

namespace Quartz
{
	void RawInput::EnumerateDevices(Array<RAWINPUTDEVICELIST>& rawInputDeviceList)
	{
		uInt32 deviceCount = 0;

		if (GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST)) == INPUT_RAW_INPUT_ERROR)
		{
			LogError("Failed to get raw input device list : GetRawInputDeviceList failed.");
			WinApiPrintError();
			return;
		}

		if (deviceCount == 0)
		{
			LogWarning("No raw input devices found.");
			return;
		}

		rawInputDeviceList.Resize(deviceCount);

		if (GetRawInputDeviceList(rawInputDeviceList.Data(), &deviceCount, sizeof(RAWINPUTDEVICELIST)) == INPUT_RAW_INPUT_ERROR)
		{
			LogError("Failed to get raw input device list : GetRawInputDeviceList failed.");
			WinApiPrintError();
			rawInputDeviceList.Clear();
			return;
		}
	}

	bool RawInput::GetDeviceInfo(const RAWINPUTDEVICELIST& rawInputDevice, RawInputDeviceInfo& outDeviceInfo)
	{
		StringW deviceName;
		StringW vendorName;
		StringW className;
		StringW pathName;
		StringW deviceId;
		StringW parentId;

		DEVINST		devInst;
		DEVPROPTYPE propertyType;
		ULONG		bufferSize = 0;

		RID_DEVICE_INFO ridDeviceInfo;

		// Device Path 

		if (GetRawInputDeviceInfoW(rawInputDevice.hDevice, RIDI_DEVICENAME, nullptr, (PUINT)&bufferSize) == INPUT_RAW_INPUT_ERROR)
		{
			//LogError("Failed to get raw input device info : GetRawInputDeviceInfoW failed.");
			//WinApiPrintError();
			return false;
		}

		pathName.Resize(bufferSize);
		if (GetRawInputDeviceInfoW(rawInputDevice.hDevice, RIDI_DEVICENAME, pathName.Data(), (PUINT)&bufferSize) == INPUT_RAW_INPUT_ERROR)
		{
			//LogError("Failed to get raw input device info : GetRawInputDeviceInfoW failed.");
			//WinApiPrintError();
			return false;
		}

		// Device Instance ID

		bufferSize = 0;
		CONFIGRET cr = CM_Get_Device_Interface_PropertyW(pathName.Str(),
			&DEVPKEY_Device_InstanceId, &propertyType, nullptr, &bufferSize, 0);
		if (cr != CR_BUFFER_SMALL)
		{
			//LogError("Failed to get raw input device info : CM_Get_Device_Interface_PropertyW failed.");
			//WinApiPrintError();
			return false;
		}

		deviceId.Resize(bufferSize);
		cr = CM_Get_Device_Interface_PropertyW(pathName.Str(),
			&DEVPKEY_Device_InstanceId, &propertyType, (PBYTE)deviceId.Data(), &bufferSize, 0);
		if (cr != CR_SUCCESS)
		{
			//LogError("Failed to get raw input device info : CM_Get_Device_Interface_PropertyW failed.");
			//WinApiPrintError();
			return false;
		}

		// Dev Instance

		cr = CM_Locate_DevNodeW(&devInst, (DEVINSTID_W)deviceId.Str(), CM_LOCATE_DEVNODE_NORMAL);
		if (cr != CR_SUCCESS)
		{
			//LogError("Failed to get raw input device info : CM_Locate_DevNodeW failed.");
			//WinApiPrintError();
			return false;
		}

		// Friendly Device Name

		bufferSize = 0;
		cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, nullptr, &bufferSize, 0);
		if (cr != CR_BUFFER_SMALL)
		{
			//LogError("Failed to get raw input device info : CM_Get_DevNode_PropertyW failed.");
			//WinApiPrintError();
			return false;
		}

		deviceName.Resize(bufferSize);
		cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME,
			&propertyType, (PBYTE)deviceName.Data(), &bufferSize, 0);
		if (cr != CR_SUCCESS)
		{
			deviceName = INPUT_UNKNOWN_DEVICE_NAME_STRING;
		}

		// Friendly Vendor Name

		bufferSize = 0;
		cr = CM_Get_DevNode_PropertyW(devInst,
			&DEVPKEY_Device_Manufacturer, &propertyType, nullptr, &bufferSize, 0);
		if (cr != CR_BUFFER_SMALL)
		{
			//LogError("Failed to get raw input device info : CM_Get_DevNode_PropertyW failed.");
			//WinApiPrintError();
			return false;
		}

		vendorName.Resize(bufferSize);
		cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Manufacturer, &propertyType, (PBYTE)vendorName.Data(), &bufferSize, 0);
		if (cr != CR_SUCCESS)
		{
			vendorName = INPUT_UNKNOWN_DEVICE_VENDOR_STRING;
		}

		// Device Class

		bufferSize = 0;
		cr = CM_Get_DevNode_PropertyW(devInst,
			&DEVPKEY_Device_Class, &propertyType, nullptr, &bufferSize, 0);
		if (cr != CR_BUFFER_SMALL)
		{
			//LogError("Failed to get raw input device info : CM_Get_DevNode_PropertyW failed.");
			//WinApiPrintError();
			return false;
		}

		className.Resize(bufferSize);
		cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Class, &propertyType, (PBYTE)className.Data(), &bufferSize, 0);
		if (cr != CR_SUCCESS)
		{
			vendorName = INPUT_UNKNOWN_DEVICE_CLASS_STRING;
		}

		// Device Parent Path

		bufferSize = 0;
		cr = CM_Get_DevNode_PropertyW(devInst,
			&DEVPKEY_Device_Parent, &propertyType, nullptr, &bufferSize, 0);
		if (cr != CR_BUFFER_SMALL)
		{
			//LogError("Failed to get raw input device info : CM_Get_DevNode_PropertyW failed.");
			//WinApiPrintError();
			return false;
		}

		parentId.Resize(bufferSize);
		cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Parent, &propertyType, (PBYTE)parentId.Data(), &bufferSize, 0);
		if (cr != CR_SUCCESS)
		{
			parentId = INPUT_UNKNOWN_DEVICE_CLASS_STRING;
		}

		// Page Usages

		bufferSize = 0;
		if (GetRawInputDeviceInfoW(rawInputDevice.hDevice, RIDI_DEVICEINFO, nullptr, (PUINT)&bufferSize) == INPUT_RAW_INPUT_ERROR)
		{
			//LogError("Failed to get raw input device info : GetRawInputDeviceInfoW failed.");
			//WinApiPrintError();
			return false;
		}

		if (bufferSize != sizeof(RID_DEVICE_INFO))
		{
			//LogError("Failed to get raw input device info : bufferSize != sizeof(RID_DEVICE_INFO)");
			return false;
		}

		if (GetRawInputDeviceInfoW(rawInputDevice.hDevice, RIDI_DEVICEINFO, &ridDeviceInfo, (PUINT)&bufferSize) == INPUT_RAW_INPUT_ERROR)
		{
			//LogError("Failed to get raw input device info : GetRawInputDeviceInfoW failed.");
			//WinApiPrintError();
			return false;
		}

		outDeviceInfo.deviceId		= deviceId;
		outDeviceInfo.deviceName	= deviceName;
		outDeviceInfo.vendorName	= vendorName;
		outDeviceInfo.className		= className;
		outDeviceInfo.pathName		= pathName;
		outDeviceInfo.parentId		= parentId;
		outDeviceInfo.axisCount		= ridDeviceInfo.mouse.fHasHorizontalWheel ? 1 : 2;
		outDeviceInfo.buttonCount	= ridDeviceInfo.mouse.dwNumberOfButtons;

		switch (rawInputDevice.dwType)
		{
		case RIM_TYPEMOUSE:
		{
			outDeviceInfo.usagePage	= 0x01;
			outDeviceInfo.usage		= 0x02;
			break;
		}

		case RIM_TYPEKEYBOARD:
		{
			outDeviceInfo.usagePage	= 0x01;
			outDeviceInfo.usage		= 0x06;
			break;
		}

		case RIM_TYPEHID:
		{
			outDeviceInfo.usagePage = ridDeviceInfo.hid.usUsagePage;
			outDeviceInfo.usage		= ridDeviceInfo.hid.usUsage;
			break;
		}

		default:
			break;
		}
	}

	bool RawInput::RegisterUsage(uInt16 usagePage, uInt16 usage, DWORD flags)
	{
		RAWINPUTDEVICE rawInputDeviceInfo{};
		rawInputDeviceInfo.usUsagePage	= usagePage;
		rawInputDeviceInfo.usUsage		= usage;
		rawInputDeviceInfo.dwFlags		= flags;
		rawInputDeviceInfo.hwndTarget	= 0;

		if (RegisterRawInputDevices(&rawInputDeviceInfo, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
		{
			LogError("Failed to register raw input device usage : RegisterRawInputDevices failed.");
			WinApiPrintError();
			return false;
		}

		return true;
	}

	InputDevice& RegisterDevice(const RawInputDevice& rawInputDevice)
	{
		RawInputDeviceInfo rawDeviceInfo = rawInputDevice.deviceInfo;

		InputDeviceInfo deviceInfo;
		deviceInfo.deviceName	= rawDeviceInfo.deviceName;
		deviceInfo.deviceVendor = rawDeviceInfo.vendorName;
		deviceInfo.devicePath	= rawDeviceInfo.pathName;
		deviceInfo.deviceType	= rawInputDevice.deviceType;
		deviceInfo.axisCount	= rawDeviceInfo.axisCount;
		deviceInfo.buttonCount	= rawDeviceInfo.buttonCount;

		InputDeviceCallbacks deviceCallbacks = {};

		if (rawInputDevice.deviceType == INPUT_DEVICE_TYPE_MOUSE)
		{
			deviceCallbacks.onMouseSetPositionFunc =
				[](const InputDevice& device, Vec2i absPosition)
				{
					return (bool)SetCursorPos(absPosition.x, absPosition.y);
				};

			deviceCallbacks.onMouseGetPositionFunc =
				[](const InputDevice& device, Vec2i& outAbsPosition)
				{
					POINT cursorPos = {};
					bool result = GetCursorPos(&cursorPos);
					outAbsPosition.x = cursorPos.x;
					outAbsPosition.y = cursorPos.y;
					return result;
				};

			deviceCallbacks.onMouseSetBoundsFunc =
				[](const Window& windowContext, const InputDevice& device, Bounds2i absBounds, bool enabled)
				{
					RECT clipRect = {};
					clipRect.left	= absBounds.BottomLeft().x;
					clipRect.right	= absBounds.TopRight().x;
					clipRect.top	= absBounds.BottomLeft().y;
					clipRect.bottom = absBounds.TopRight().y;
					bool result = ClipCursor(enabled ? &clipRect : NULL);
					return result;
				};

			deviceCallbacks.onMouseSetHiddenFunc =
				[](const Window& windowContext, const InputDevice& device, bool hidden)
				{
					ShowCursor(!hidden);
					return true;
				};
		}

		return Engine::GetDeviceRegistry().RegisterDevice(deviceInfo, deviceCallbacks);
	}

	void RawInput::Init()
	{
		PollConnections();

		InputDevice* pPrimaryMouse		= nullptr;
		InputDevice* pPrimaryKeyboard	= nullptr;
		InputDevice* pPrimaryController	= nullptr;

		for (auto& devicePair : mDeviceMap)
		{
			if (!pPrimaryMouse && devicePair.value.deviceType == INPUT_DEVICE_TYPE_MOUSE)
			{
				pPrimaryMouse = devicePair.value.pInputDevice;
				Engine::GetDeviceRegistry().SetPrimaryMouse(*pPrimaryMouse);
				continue;
			}
			
			if (!pPrimaryKeyboard && devicePair.value.deviceType == INPUT_DEVICE_TYPE_KEYBOARD)
			{
				pPrimaryKeyboard = devicePair.value.pInputDevice;
				Engine::GetDeviceRegistry().SetPrimaryKeyboard(*pPrimaryMouse);
				continue;
			}

			if (!pPrimaryController && devicePair.value.deviceType == INPUT_DEVICE_TYPE_CONTROLLER)
			{
				pPrimaryController = devicePair.value.pInputDevice;
				Engine::GetDeviceRegistry().SetPrimaryController(*pPrimaryMouse);
				continue;
			}
		}
	}

	void RawInput::PollConnections()
	{
		Array<RAWINPUTDEVICELIST>	rawInputDevices;
		Array<HANDLE>				activeIds;
		StringW						typeStr;

		EnumerateDevices(rawInputDevices);

		for (const RAWINPUTDEVICELIST& rawInputDevice : rawInputDevices)
		{
			if (mActiveIds.Contains(rawInputDevice.hDevice))
			{
				activeIds.PushBack(rawInputDevice.hDevice);
				continue; // Ignore non-new devices
			}

			RawInputDeviceInfo rawDeviceInfo;
			bool result = GetDeviceInfo(rawInputDevice, rawDeviceInfo);

			if (rawDeviceInfo.usagePage != 1)
			{
				continue; // Ignore extraneous devices
			}
			
			if (!(rawDeviceInfo.usage == 2 || rawDeviceInfo.usage == 6))
			{
				continue; // Ignore extraneous devices
			}

			if (rawDeviceInfo.vendorName == L"(Standard keyboards)")
			{
				continue; // Ignore virtual devices
			}

			/*
			LogInfo(L"[DeviceFound] Name: %s, \n\tVendor: %s, \n\tClass: %s, \n\tID: %s \n\tParentID: %s \n\tUsagePage: %d\n\tUsage: %d",
				rawDeviceInfo.deviceName.Str(), rawDeviceInfo.vendorName.Str(), rawDeviceInfo.className.Str(),
				rawDeviceInfo.deviceId.Str(), rawDeviceInfo.parentId.Str(), rawDeviceInfo.usagePage, rawDeviceInfo.usage);
			*/

			mDeviceIdMap.Put((uInt64)rawInputDevice.hDevice, rawDeviceInfo.deviceId);

			RawInputDevice device = {};
			device.hDevice		= rawInputDevice.hDevice;
			device.deviceInfo	= rawDeviceInfo;

			switch (rawInputDevice.dwType)
			{
				case RIM_TYPEMOUSE:
				{
					device.deviceType	= INPUT_DEVICE_TYPE_MOUSE;
					typeStr				= L"Mouse";
					break;
				}

				case RIM_TYPEKEYBOARD:
				{
					device.deviceType	= INPUT_DEVICE_TYPE_KEYBOARD;
					typeStr				= L"Keyboard";
					break;
				}

				case RIM_TYPEHID:
				{
					device.deviceType	= INPUT_DEVICE_TYPE_CONTROLLER;
					typeStr				= L"Controller";
					break;
				}

				default:
					// Error
					break;
			}

			if (!RegisterUsage(rawDeviceInfo.usagePage, rawDeviceInfo.usage, 0))//RIDEV_NOLEGACY))
			{
				continue; // Error
			}

			LogTrace(L"RawInput device connected: [Type: %s] Name: %s, Vendor: %s", typeStr.Str(), rawDeviceInfo.deviceName.Str(), rawDeviceInfo.vendorName.Str());

			InputDevice& inputDevice = RegisterDevice(device);
			device.pInputDevice = &inputDevice;

			mDeviceMap.Put(rawDeviceInfo.deviceId, device);
			activeIds.PushBack(rawInputDevice.hDevice);
		}

		// Check for lost devices
		for (HANDLE hDevice : mActiveIds)
		{
			if (!activeIds.Contains(hDevice))
			{
				StringW deviceId		= mDeviceIdMap.Get((uInt64)hDevice);
				RawInputDevice& device	= mDeviceMap.Get(deviceId);

				Engine::GetDeviceRegistry().UnregisterDevice(*device.pInputDevice);

				mDeviceMap.Remove(deviceId);
				mDeviceIdMap.Remove((uInt64)hDevice);

				switch (device.deviceType)
				{
					case INPUT_DEVICE_TYPE_MOUSE:		typeStr = L"Mouse";			break;
					case INPUT_DEVICE_TYPE_KEYBOARD:	typeStr	= L"Keyboard";		break; 
					case INPUT_DEVICE_TYPE_CONTROLLER:	typeStr = L"Controller";	break;
				}

				LogTrace(L"RawInput device disconnected: [Type: %s] Name: %s, Vendor: %s", typeStr.Str(), device.deviceInfo.deviceName.Str(), device.deviceInfo.vendorName.Str());
			}
		}

		Swap(mActiveIds, activeIds);
	}

	void RawInput::PollInput()
	{
		RAWINPUT inputBuffer[INPUT_MAX_RAWINPUT_BUFFER_SIZE]{};
		uInt32 bufferSize = sizeof(RAWINPUT) * INPUT_MAX_RAWINPUT_BUFFER_SIZE;
	
		int32 inputCount = GetRawInputBuffer(inputBuffer, &bufferSize, sizeof(RAWINPUTHEADER));
	
		for (uInt32 i = 0; i < inputCount; i++)
		{
			if (!mDeviceIdMap.Contains((uInt64)inputBuffer[i].header.hDevice))
			{
				// Idk where these phantom hDevices come from
				// @TODO: Figure out why
				continue;
			}
	
			const StringW& deviceId = mDeviceIdMap[(uInt64)inputBuffer[i].header.hDevice];
			RawInputDevice& device = mDeviceMap[deviceId];
	
			switch (inputBuffer[i].header.dwType)
			{
			case RIM_TYPEMOUSE:
			{
				if (inputBuffer[i].data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
				{
					float relX = inputBuffer[i].data.mouse.lLastX;
					float relY = inputBuffer[i].data.mouse.lLastY;

					Engine::GetInput().SendAxisInput(device.pInputDevice, 0, INPUT_ACTION_MOVE, { relX , relY });
				}
	
				if (inputBuffer[i].data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
				{
					float x = inputBuffer[i].data.mouse.lLastX;
					float y = inputBuffer[i].data.mouse.lLastY;

					Engine::GetInput().SendAxisInput(device.pInputDevice, 0, INPUT_ACTION_MOVE, { x , y });
				}
	
				if (inputBuffer[i].data.mouse.usButtonFlags)
				{
					const uInt32 maxButtonDownBit = 0x1 << (5/*inputBuffer[i]->buttonCount*/ * 2);
					for (uInt32 j = 0x1, idx = 0; j < maxButtonDownBit; j <<= 2, idx++)
					{
						if (inputBuffer[i].data.mouse.usButtonFlags & j)
						{
							//mVPMouseButtonCallbackFunc(
							//	static_cast<HVPInputMouse>(pMouse),
							//	static_cast<Int32>(idx), BUTTON_STATE_DOWN);
						}
					}
	
					const uInt32 maxButtonUpBit = 0x2 << (5/*inputBuffer[i]->buttonCount*/ * 2);
					for (uInt32 j = 0x2, idx = 0; j < maxButtonUpBit; j <<= 2, idx++)
					{
						if (inputBuffer[i].data.mouse.usButtonFlags & j)
						{
							//mVPMouseButtonCallbackFunc(
							//	static_cast<HVPInputMouse>(pMouse),
							//	static_cast<Int32>(idx), BUTTON_STATE_UP);
						}
					}
				}
	
				break;
			}
	
			case RIM_TYPEKEYBOARD:
			{
				//Win32InputKeyboard* pKeyboard = mKeyboards[deviceId];
				//
				//mVPKeyboardKeyCallbackFunc(
				//	static_cast<HVPInputKeyboard>(pKeyboard),
				//	static_cast<uInt32>(inputBuffer[i].data.keyboard.MakeCode),
				//	inputBuffer[i].data.keyboard.Flags & RI_KEY_BREAK ? BUTTON_STATE_UP : BUTTON_STATE_DOWN);
	
				break;
			}
	
			case RIM_TYPEHID:
			{
				//Win32InputController* pController = mControllers[deviceId];
	
				//LogDebug(L"[CONTROLLER][%s]", pController->info.deviceName.Str());
	
				break;
			}
	
			default:
				break;
			}
		}
	}
}
