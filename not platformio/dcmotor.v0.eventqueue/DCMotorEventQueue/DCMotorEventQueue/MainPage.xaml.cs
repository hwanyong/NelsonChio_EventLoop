/*
 * Datasheet
 * BTData[0]	Task ID
 * BTData[1]	Status Bit Flag
 *		----- Task Start/Stop -----
 *		0: { 0/1: Task Enable }
 *		1: { 0: Task Stop, 1: Task Start }
 *		----- Task(with BTData[0]) -----
 *		2: { 0/1: CPLT Task }
 *		3: { 0/1: Regist Task }
 *		----- NULL -----
 *		4: { null }
 *		5: { null }
 *		----- Rotation -----
 *		6: { 0/1: Rotation Enable }
 *		7: { 0: Rotation CCW, 1: Rotation CW }
 * BTData[2]	Motor Speed (170 ~ 255)
 * BTData[3 ~ 6]	time / -1 (only Arduino or Event queue server)
 */
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;
using Plugin.Bluetooth;
using Plugin.Bluetooth.Abstractions;

namespace DCMotorEventQueue
{
	public enum BTStatusFlag {
		TaskStart	= 0b11000000,
		TaskStop	= 0b10000000,
		TaskCPLT	= 0b00100000,
		TaskRegi	= 0b00010000,
		CCW			= 0b00000010,
		CW			= 0b00000011
	}

	// Learn more about making custom code visible in the Xamarin.Forms previewer
	// by visiting https://aka.ms/xamarinforms-previewer
	[DesignTimeVisible(false)]
	public partial class MainPage : ContentPage
	{
		public IBluetoothDevice PairedDevice;
		private byte[] btData = new byte[7];
		public ObservableCollection<TaskItem> Tasks = new ObservableCollection<TaskItem>();

		public MainPage()
		{
			InitializeComponent();

			this.AutoBTPairing();
			TaskList.ItemsSource = Tasks;
		}

		private void handlerStopClicked(object sender, EventArgs args) {
			clearBTData();
			btData[1] = (byte)BTStatusFlag.TaskStop;
			SendBTData();
		}
		private void handlerStartClicked(object sender, EventArgs args) {
			clearBTData();
			btData[1] = (byte)BTStatusFlag.TaskStart;
			SendBTData();
		}

		private void handlerSendClicked(object sender, EventArgs args)
		{
			byte lastId;
			if (Tasks.Count == 0) { lastId = 0; }
			else
			{
				lastId = Tasks[Tasks.Count - 1].id;

				if (lastId == 255) { lastId = 0; }
				else { lastId++; }
			}

			int _nTime = Convert.ToInt32(this.etrTime.Text);
			byte[] _time = BitConverter.GetBytes(_nTime);

			clearBTData();
			btData[0] = lastId;
			btData[1] = (byte)(BTStatusFlag.TaskRegi + Convert.ToByte(this.swRotation.IsToggled));
			btData[2] = Convert.ToByte(Convert.ToInt32(this.etrSpeed.Text));
			btData[3] = _time[3];
			btData[4] = _time[2];
			btData[5] = _time[1];
			btData[6] = _time[0];
			SendBTData();

			Tasks.Add(new TaskItem() { id = btData[0], speed=btData[1], rotation = btData[2], time = _nTime });
		}

		private async void AutoBTPairing()
		{
			using (IBluetooth bluetooth = CrossBluetooth.Current)
			{
				List<IBluetoothDevice> deviceList = await bluetooth.GetPairedDevices();

				if (deviceList.Any())
				{
					#region !!BADCODE Save Paired Device
					this.PairedDevice = deviceList[0];
					this.lbBTStatus.Text = $"Paired: {this.PairedDevice.Name}";

					if (!this.PairedDevice.IsConnected)
					{
						this.PairedDevice.Connect();
					}
					#endregion
				}
				else
				{
					await DisplayAlert("Warning", "There are no paired devices", "Ok");
				}
			}
		}

		#region Bluetooth API
		private void clearBTData()
		{
			for (int cnt = 0; cnt < this.btData.Length; cnt++)
			{
				this.btData[cnt] = 0;
			}
		} 
		private async void SendBTData()
		{
			if (!this.PairedDevice.IsWriting)
			{
				await this.PairedDevice.Write(this.btData);
			}
		}
		#endregion
	}

	public class TaskItem {
		public byte id { get; set; }
		public byte speed { get; set; }
		public byte rotation { get; set; }
		public int time { get; set; }
		public string rule {
			get {
				return $"[{this.id}]{this.speed}:{this.rotation}:{this.time}";
			}
		}
	}
}
