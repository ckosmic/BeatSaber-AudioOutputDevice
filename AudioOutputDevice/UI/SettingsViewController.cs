using System;
using System.Collections.Generic;
using System.Linq;

using BeatSaberMarkupLanguage;
using BeatSaberMarkupLanguage.Attributes;
using BeatSaberMarkupLanguage.Components;
using BeatSaberMarkupLanguage.Components.Settings;
using BeatSaberMarkupLanguage.GameplaySetup;
using BeatSaberMarkupLanguage.ViewControllers;
using HMUI;
using IPA.Utilities;
using UnityEngine;
using UnityEngine.EventSystems;
using Zenject;

namespace AudioOutputDevice
{
	internal class SettingsViewController : IInitializable, IDisposable
	{
		private ModalView _dropdownModal;
		private AudioDeviceInfo[] _deviceInfos;
		private bool _skipRemembrance = false;

		[UIValue("device-list")]
		private List<object> _deviceList = new List<object>();

		[UIValue("system-volume")]
		private int _systemVolume = (int)(PluginInterop.GetVolume() * 100.0f);

		[UIValue("rember")]
		private bool _rememberDevice { 
			get { return Plugin.Settings.RememberDevice; }
			set { Plugin.Settings.RememberDevice = value; }
		}

		[UIComponent("device-dropdown")]
		private readonly DropDownListSetting _dropDownListSetting;

		[UIComponent("volume-slider")]
		private readonly SliderSetting _sliderSetting;

		[UIAction("#post-parse")]
		public void PostParse() {
			_deviceList = new List<object>();
			_deviceInfos = PluginInterop.GetAudioDeviceInfo();
			foreach (AudioDeviceInfo deviceInfo in _deviceInfos) {
				_deviceList.Add(deviceInfo.Name);
			}
			_dropDownListSetting.values = _deviceList;
			_dropDownListSetting.UpdateChoices();
			_dropDownListSetting.GetComponent<RectTransform>().sizeDelta = new Vector2(50.0f, 0.0f);
			_dropDownListSetting.GetComponentInChildren<NoTransitionsButton>(true).onClick.AddListener(OnDropDownClicked);

			AudioDeviceInfo selectedDevice = _deviceInfos.ToList().Where((info) => info.IsDefault == true).First();
			string deviceNameSearch = "";
			if (_rememberDevice) {
				deviceNameSearch = Plugin.Settings.LastDeviceName;
				// Yikes vv
				if(DeviceSelected(deviceNameSearch) <= 0) {
					deviceNameSearch = selectedDevice.Name;
					_skipRemembrance = true;
					DeviceSelected(deviceNameSearch);
					_skipRemembrance = false;
				}
			} else {
				deviceNameSearch = selectedDevice.Name;
			}

			_dropDownListSetting.Value = _deviceList.Where(obj => ((string)obj) == deviceNameSearch).FirstOrDefault();
			_dropDownListSetting.ApplyValue();

			_dropDownListSetting.transform.Find("DropDownButton/Text").GetComponent<CurvedTextMeshPro>().overflowMode = TMPro.TextOverflowModes.Ellipsis;

			_dropdownModal = _dropDownListSetting.GetComponentInChildren<ModalView>(true);
			_dropdownModal.GetComponent<RectTransform>().sizeDelta = new Vector2(50.0f, 43.2f);

			_sliderSetting.GetComponentInChildren<CustomFormatRangeValuesSlider>().GetComponent<RectTransform>().sizeDelta = new Vector2(50.0f, 0.0f);
		}

		[UIAction("device-selected")]
		public int DeviceSelected(string itemName) {
			int index = _deviceInfos.ToList().FindIndex(deviceInfo => deviceInfo.Name == itemName);
			if (index >= 0) {
				PluginInterop.SetAudioDevice(index);
				_sliderSetting.Value = (int)(PluginInterop.GetVolume() * 100.0f);
				_sliderSetting.ApplyValue();
				if (!_skipRemembrance) {
					Plugin.Settings.LastDeviceName = itemName;
				}
			} else {
				Plugin.Log.Error($"Failed to find { itemName } in device list.");
			}
			return index;
		}

		[UIAction("volume-changed")]
		public void SetSystemVolume(int volume) {
			_systemVolume = volume;
			PluginInterop.SetVolume((float)volume / 100.0f);
		}

		private void OnDropDownClicked() {
			_dropdownModal.transform.localPosition = new Vector3(-40.0f, _dropdownModal.transform.localPosition.y, 0.0f);
		}

		public void Initialize() {
			GameplaySetup.instance.AddTab("Audio Device", $"AudioOutputDevice.UI.Views.settingsView.bsml", this);
		}

		public void Dispose() {
			GameplaySetup.instance.RemoveTab("Audio Device");
		}
	}
}
