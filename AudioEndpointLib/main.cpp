// Modified version of https://github.com/DanStevens/AudioEndPointController by DanStevens to work as a library (and add extra features like volume getting/setting)
// Also I'm sorry for whoever looks at this bc everything's repeated a bunch of times for each exported function

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include <string>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <windows.h>
#include "PolicyConfig.h"
#include "Functiondiscoverykeys_devpkey.h"

using namespace std;

extern "C" {
	#define DEVICE_OUTPUT_FORMAT "%d %ws %d"

	typedef struct TGlobalState
	{
		HRESULT hr;
		int option;
		IMMDeviceEnumerator* pEnum;
		IMMDeviceCollection* pDevices;
		LPWSTR strDefaultDeviceID;
		IMMDevice* pCurrentDevice;
		LPCWSTR pDeviceFormatStr;
		int deviceStateFilter;
	} TGlobalState;

	__declspec(dllexport) void getAudioDeviceInfo(void (add)(wchar_t* pstr));
	__declspec(dllexport) void setAudioDevice(int index);
	__declspec(dllexport) float getVolume();
	__declspec(dllexport) void setVolume(float volume);
	__declspec(dllexport) void initialize();
	__declspec(dllexport) void uninitialize();
	HRESULT printDeviceInfo(wchar_t* buffer, IMMDevice* pDevice, int index, LPCWSTR outFormat, LPWSTR strDefaultDeviceID);
	std::wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key);
	HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID);

	__declspec(dllexport) void initialize() {
		CoInitialize(NULL);
	}

	__declspec(dllexport) void uninitialize() {
		CoUninitialize();
	}

	__declspec(dllexport) void getAudioDeviceInfo(void (add)(wchar_t* pstr)) {
		TGlobalState state;

		state.strDefaultDeviceID = (LPWSTR)'\0';
		state.pDeviceFormatStr = _T(DEVICE_OUTPUT_FORMAT);
		state.deviceStateFilter = DEVICE_STATE_ACTIVE;

		state.pEnum = NULL;
		state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&state.pEnum);
		if (SUCCEEDED(state.hr))
		{
			state.hr = state.pEnum->EnumAudioEndpoints(eRender, state.deviceStateFilter, &state.pDevices);
			if SUCCEEDED(state.hr)
			{
				UINT count;
				state.pDevices->GetCount(&count);

				// Get default device
				IMMDevice* pDefaultDevice;
				state.hr = state.pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultDevice);
				if (SUCCEEDED(state.hr))
				{
					state.hr = pDefaultDevice->GetId(&state.strDefaultDeviceID);

					// Iterate all devices
					for (int i = 1; i <= (int)count; i++)
					{
						state.hr = state.pDevices->Item(i - 1, &state.pCurrentDevice);
						if (SUCCEEDED(state.hr))
						{
							wchar_t buffer[256] = { '\0' };
							state.hr = printDeviceInfo(buffer, state.pCurrentDevice, i, state.pDeviceFormatStr, state.strDefaultDeviceID);
							add(buffer);
							state.pCurrentDevice->Release();
						}
					}
				}
				state.pDevices->Release();
			}
			state.pEnum->Release();
		}
	}

	__declspec(dllexport) void setAudioDevice(int index) {
		TGlobalState state;

		state.strDefaultDeviceID = (LPWSTR)'\0';
		state.pDeviceFormatStr = _T(DEVICE_OUTPUT_FORMAT);
		state.deviceStateFilter = DEVICE_STATE_ACTIVE;

		state.pEnum = NULL;
		state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&state.pEnum);
		if (SUCCEEDED(state.hr))
		{
			state.hr = state.pEnum->EnumAudioEndpoints(eRender, state.deviceStateFilter, &state.pDevices);
			if SUCCEEDED(state.hr)
			{
				UINT count;
				state.pDevices->GetCount(&count);

				state.hr = state.pDevices->Item(index, &state.pCurrentDevice);
				if (SUCCEEDED(state.hr))
				{
					LPWSTR strID = NULL;
					state.hr = state.pCurrentDevice->GetId(&strID);
					if (SUCCEEDED(state.hr))
					{
						state.hr = SetDefaultAudioPlaybackDevice(strID);
					}
					state.pCurrentDevice->Release();
				}
				state.pDevices->Release();
			}
			state.pEnum->Release();
		}
	}

	__declspec(dllexport) float getVolume() {
		TGlobalState state;

		float currentVolume = 0;

		state.pEnum = nullptr;
		state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&state.pEnum);
		if (SUCCEEDED(state.hr))
		{
			// Get default device
			IMMDevice* pDefaultDevice = nullptr;
			state.hr = state.pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultDevice);
			state.pEnum->Release();
			state.pEnum = nullptr;
			if (SUCCEEDED(state.hr))
			{
				IAudioEndpointVolume* pEndpointVolume = NULL;
				state.hr = pDefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&pEndpointVolume);
				pDefaultDevice->Release();
				pDefaultDevice = nullptr;
				if (SUCCEEDED(state.hr)) {
					state.hr = pEndpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
					pEndpointVolume->Release();
				}
			}
		}

		return currentVolume;
	}

	__declspec(dllexport) void setVolume(float volume) {
		TGlobalState state;

		state.pEnum = NULL;
		state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&state.pEnum);
		if (SUCCEEDED(state.hr))
		{
			// Get default device
			IMMDevice* pDefaultDevice = NULL;
			state.hr = state.pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultDevice);
			state.pEnum->Release();
			state.pEnum = nullptr;
			if (SUCCEEDED(state.hr))
			{
				IAudioEndpointVolume* pEndpointVolume = NULL;
				state.hr = pDefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&pEndpointVolume);
				pDefaultDevice->Release();
				pDefaultDevice = nullptr;
				if (SUCCEEDED(state.hr)) {
					state.hr = pEndpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
					pEndpointVolume->Release();
				}
			}
		}
	}

	HRESULT printDeviceInfo(wchar_t* buffer, IMMDevice* pDevice, int index, LPCWSTR outFormat, LPWSTR strDefaultDeviceID)
	{
		// Device ID
		LPWSTR strID = NULL;
		HRESULT hr = pDevice->GetId(&strID);
		if (!SUCCEEDED(hr))
		{
			return hr;
		}

		int deviceDefault = (strDefaultDeviceID != (LPWSTR)'\0' && (wcscmp(strDefaultDeviceID, strID) == 0));

		// Device state
		DWORD dwState;
		hr = pDevice->GetState(&dwState);
		if (!SUCCEEDED(hr))
		{
			return hr;
		}

		IPropertyStore* pStore;
		hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
		if (SUCCEEDED(hr))
		{
			std::wstring friendlyName = getDeviceProperty(pStore, PKEY_Device_FriendlyName);
			std::wstring Desc = getDeviceProperty(pStore, PKEY_Device_DeviceDesc);
			std::wstring interfaceFriendlyName = getDeviceProperty(pStore, PKEY_DeviceInterface_FriendlyName);

			if (SUCCEEDED(hr))
			{
				swprintf(buffer, 256, L"%d %s %d", index, friendlyName.c_str(), deviceDefault);

				//,dwState,Desc.c_str(),interfaceFriendlyName.c_str(),strID
			}

			pStore->Release();
		}
		return hr;
	}

	std::wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key)
	{
		PROPVARIANT prop;
		PropVariantInit(&prop);
		HRESULT hr = pStore->GetValue(key, &prop);
		if (SUCCEEDED(hr))
		{
			std::wstring result(prop.pwszVal);
			PropVariantClear(&prop);
			return result;
		}
		else
		{
			return std::wstring(L"");
		}
	}

	HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID)
	{
		IPolicyConfigVista* pPolicyConfig;
		ERole reserved = eConsole;

		HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient),
			NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID*)&pPolicyConfig);
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
			pPolicyConfig->Release();
		}
		return hr;
	}
}