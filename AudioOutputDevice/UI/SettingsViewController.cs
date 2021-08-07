using System;
using System.Collections.Generic;
using System.Linq;

using BeatSaberMarkupLanguage;
using BeatSaberMarkupLanguage.Attributes;
using BeatSaberMarkupLanguage.Components;
using BeatSaberMarkupLanguage.Components.Settings;
using BeatSaberMarkupLanguage.ViewControllers;
using HMUI;
using IPA.Utilities;
using UnityEngine;

namespace AudioOutputDevice
{
	internal class SettingsViewController : PersistentSingleton<SettingsViewController>
	{
		[UIValue("device-list")]
		private List<object> _deviceList = new List<object>();

		[UIComponent("device-dropdown")]
		private readonly DropDownListSetting _dropDownListSetting;

		[UIAction("#post-parse")]
		public void PostParse() {
			_deviceList = new List<object>();
			AudioDeviceInfo[] deviceInfos = DeviceController.instance.FetchDeviceInfos();
			foreach (AudioDeviceInfo deviceInfo in deviceInfos) {
				_deviceList.Add(deviceInfo.Name);
			}
			_dropDownListSetting.values = _deviceList;
			_dropDownListSetting.UpdateChoices();
			_dropDownListSetting.GetComponent<RectTransform>().sizeDelta = new Vector2(50.0f, 0.0f);
			_dropDownListSetting.GetComponentInChildren<ModalView>(true).GetComponent<RectTransform>().sizeDelta = new Vector2(60.0f, 43.2f);

			AudioDeviceInfo selectedDevice = DeviceController.instance.GetSelectedDevice();
			_dropDownListSetting.Value = _deviceList.Where(obj => ((string)obj) == selectedDevice.Name).FirstOrDefault();
			_dropDownListSetting.ApplyValue();

			_dropDownListSetting.transform.Find("DropDownButton/Text").GetComponent<CurvedTextMeshPro>().overflowMode = TMPro.TextOverflowModes.Ellipsis;
		}

		[UIAction("device-selected")]
		public void DeviceSelected(string itemName) {
			int index = DeviceController.instance.deviceInfos.ToList().FindIndex(deviceInfo => deviceInfo.Name == itemName);
			DeviceController.instance.SetAudioDevice(index);
		}
	}
}
