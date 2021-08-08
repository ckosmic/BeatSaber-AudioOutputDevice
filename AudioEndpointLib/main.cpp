#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include <string>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include "PolicyConfig.h"
#include "windows.h"
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

	__declspec(dllexport) void getAudioDeviceInfo(void (add)(char* pstr));
	__declspec(dllexport) void setAudioDevice(int index);
	HRESULT printDeviceInfo(char* buffer, IMMDevice* pDevice, int index, LPCWSTR outFormat, LPWSTR strDefaultDeviceID);
	std::wstring getDeviceProperty(IPropertyStore* pStore, const PROPERTYKEY key);
	HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID);

	__declspec(dllexport) void getAudioDeviceInfo(void (add)(char* pstr)) {
		TGlobalState state;

		state.strDefaultDeviceID = (LPWSTR)'\0';
		state.pDeviceFormatStr = _T(DEVICE_OUTPUT_FORMAT);
		state.deviceStateFilter = DEVICE_STATE_ACTIVE;

		state.hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (SUCCEEDED(state.hr))
		{
			state.pEnum = NULL;
			state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void**)&state.pEnum);
			if (SUCCEEDED(state.hr))
			{
				state.hr = state.pEnum->EnumAudioEndpoints(eRender, state.deviceStateFilter, &state.pDevices);
				if SUCCEEDED(state.hr)
				{
					UINT count;
					state.pDevices->GetCount(&count);

					// Get default device
					IMMDevice* pDefaultDevice;
					state.hr = state.pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);
					if (SUCCEEDED(state.hr))
					{
						state.hr = pDefaultDevice->GetId(&state.strDefaultDeviceID);

						// Iterate all devices
						for (int i = 1; i <= (int)count; i++)
						{
							state.hr = state.pDevices->Item(i - 1, &state.pCurrentDevice);
							if (SUCCEEDED(state.hr))
							{
								char buffer[256] = { '\0' };
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
	}

	__declspec(dllexport) void setAudioDevice(int index) {
		TGlobalState state;

		state.strDefaultDeviceID = (LPWSTR)'\0';
		state.pDeviceFormatStr = _T(DEVICE_OUTPUT_FORMAT);
		state.deviceStateFilter = DEVICE_STATE_ACTIVE;

		state.hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (SUCCEEDED(state.hr))
		{
			state.pEnum = NULL;
			state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void**)&state.pEnum);
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
	}

	__declspec(dllexport) float getVolume() {
		TGlobalState state;

		state.strDefaultDeviceID = (LPWSTR)'\0';
		state.pDeviceFormatStr = _T(DEVICE_OUTPUT_FORMAT);
		state.deviceStateFilter = DEVICE_STATE_ACTIVE;

		float currentVolume = 0;

		state.hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (SUCCEEDED(state.hr))
		{
			state.pEnum = NULL;
			state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void**)&state.pEnum);
			if (SUCCEEDED(state.hr))
			{
				state.hr = state.pEnum->EnumAudioEndpoints(eRender, state.deviceStateFilter, &state.pDevices);
				if SUCCEEDED(state.hr)
				{
					UINT count;
					state.pDevices->GetCount(&count);

					// Get default device
					IMMDevice* pDefaultDevice;
					state.hr = state.pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);
					if (SUCCEEDED(state.hr))
					{
						IAudioEndpointVolume* endpointVolume;
						state.hr = pDefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
						if (SUCCEEDED(state.hr)) {
							state.hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
						}

						endpointVolume->Release();
						pDefaultDevice->Release();
					}

					state.pDevices->Release();
				}
				state.pEnum->Release();
			}
		}

		return currentVolume;
	}

	__declspec(dllexport) void setVolume(float volume) {
		TGlobalState state;

		state.strDefaultDeviceID = (LPWSTR)'\0';
		state.pDeviceFormatStr = _T(DEVICE_OUTPUT_FORMAT);
		state.deviceStateFilter = DEVICE_STATE_ACTIVE;

		state.hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (SUCCEEDED(state.hr))
		{
			state.pEnum = NULL;
			state.hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void**)&state.pEnum);
			if (SUCCEEDED(state.hr))
			{
				state.hr = state.pEnum->EnumAudioEndpoints(eRender, state.deviceStateFilter, &state.pDevices);
				if SUCCEEDED(state.hr)
				{
					UINT count;
					state.pDevices->GetCount(&count);

					// Get default device
					IMMDevice* pDefaultDevice;
					state.hr = state.pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);
					if (SUCCEEDED(state.hr))
					{
						IAudioEndpointVolume* endpointVolume;
						state.hr = pDefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
						if (SUCCEEDED(state.hr)) {
							state.hr = endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
						}

						endpointVolume->Release();
						pDefaultDevice->Release();
					}

					state.pDevices->Release();
				}
				state.pEnum->Release();
			}
		}
	}

	HRESULT printDeviceInfo(char* buffer, IMMDevice* pDevice, int index, LPCWSTR outFormat, LPWSTR strDefaultDeviceID)
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
				sprintf(buffer, "%d %ws %d", index, friendlyName.c_str(), deviceDefault);

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