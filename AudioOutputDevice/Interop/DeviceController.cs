using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace AudioOutputDevice
{
	internal class DeviceController : MonoBehaviour
	{
		public static DeviceController instance { get; private set; }

		public AudioDeviceInfo[] deviceInfos;

		private void Awake() {
			if (instance != null) {
				GameObject.DestroyImmediate(this);
				return;
			}
			GameObject.DontDestroyOnLoad(this);
			instance = this;
		}

		public AudioDeviceInfo[] FetchDeviceInfos() {
			deviceInfos = PluginInterop.GetAudioDeviceInfo();
			return deviceInfos;
		}

		public AudioDeviceInfo GetSelectedDevice() {
			return deviceInfos.ToList().Where((info) => info.IsDefault == true).First();
		}

		public void SetAudioDevice(int index) {
			PluginInterop.SetAudioDevice(index);
		}
	}
}
