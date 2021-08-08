using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Zenject;

namespace AudioOutputDevice
{
	internal class AODMenuInstaller : Installer
	{
		public override void InstallBindings() {
			Container.BindInterfacesAndSelfTo<SettingsViewController>().AsSingle();
		}
	}
}
