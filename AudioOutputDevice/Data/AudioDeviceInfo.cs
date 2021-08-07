using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AudioOutputDevice
{
	internal class AudioDeviceInfo
	{
		public int Index { get; set; }
		public string Name { get; set; }
		public bool IsDefault { get; set; }

		public AudioDeviceInfo(int index, string name, bool isDefault) {
			this.Index = index;
			this.Name = name;
			this.IsDefault = isDefault;
		}

		public override string ToString() {
			return this.Name;
		}
	}
}
