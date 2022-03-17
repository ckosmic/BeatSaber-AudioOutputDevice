using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace AudioOutputDevice
{
	internal class PluginInterop
	{
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void AddAnsi([MarshalAs(UnmanagedType.LPWStr)] string str);

		[DllImport("AudioEndpointLib", CallingConvention = CallingConvention.Cdecl)]
		private static extern void getAudioDeviceInfo(AddAnsi add);
		[DllImport("AudioEndpointLib")]
		private static extern void setAudioDevice(int index);
		[DllImport("AudioEndpointLib")]
		private static extern float getVolume();
		[DllImport("AudioEndpointLib")]
		private static extern void setVolume(float volume);

		[DllImport("AudioEndpointLib")]
		private static extern void initialize();
		[DllImport("AudioEndpointLib")]
		private static extern void uninitialize();

		public static AudioDeviceInfo[] GetAudioDeviceInfo() {
			List<string> deviceInfoStrings = new List<string>();
			getAudioDeviceInfo(deviceInfoStrings.Add);

			AudioDeviceInfo[] deviceInfos = new AudioDeviceInfo[deviceInfoStrings.Count];
			for (int i = 0; i < deviceInfoStrings.Count; i++) {
				deviceInfos[i] = CreateAudioDeviceInfo(deviceInfoStrings[i]);
			}

			return deviceInfos;
		}

		public static void SetAudioDevice(int index) {
			setAudioDevice(index);
		}

		public static void SetVolume(float volume) {
			setVolume(volume);
		}
		public static float GetVolume() {
			return getVolume();
		}

		public static void Initialize() {
			initialize();
		}
		public static void Uninitialize() {
			uninitialize();
		}

		// Device info strings are formatted as such:       {index} {name} {isDefault}
		internal static AudioDeviceInfo CreateAudioDeviceInfo(string deviceInfoString) {
			string[] infoParts = deviceInfoString.Split(' ');

			Plugin.Log.Debug(infoParts[0]);
			Plugin.Log.Debug(infoParts[infoParts.Length - 1]);

			int index = int.Parse(infoParts[0]);
			bool isDefault = int.Parse(infoParts[infoParts.Length - 1]) == 1;
			string[] nameParts = infoParts.Skip(1).Take(infoParts.Length - 2).ToArray();
			string name = string.Join(" ", nameParts);

			return new AudioDeviceInfo(index, name, isDefault);
		}
	}
}
