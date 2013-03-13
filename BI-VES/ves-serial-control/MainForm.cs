using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ves_serial_control
{
	public partial class MainForm : Form
	{
		public MainForm ()
		{
			InitializeComponent ();
			toggle_enabled (false);

			string[] ports = System.IO.Ports.SerialPort.GetPortNames ();
			com_port_cbx.Items.AddRange (ports);
			if (ports.Length > 0)
				com_port_cbx.SelectedIndex = 0;
		}

		private void reset ()
		{
			receive_lbl.Text = "";
		}

		private void toggle_enabled (bool enabled)
		{
			led_box.Enabled = enabled;
			display_btn.Enabled = enabled;
			display_textbox.Enabled = enabled;
			reset_btn.Enabled = enabled;
			receive_btn.Enabled = enabled;
			receive_lbl.Enabled = enabled;
		}

		private void MainForm_Shown (object sender, EventArgs e)
		{
		}

		private void connect_btn_Click (object sender, EventArgs e)
		{
			serial_port.PortName = (string) com_port_cbx.SelectedItem;
			try
			{
				serial_port.Open ();
				connect_btn.Enabled = false;
				com_port_cbx.Enabled = false;
				toggle_enabled (true);
			}
			catch (Exception exc)
			{
				MessageBox.Show (this, exc.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void ctrl_red_on_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&R");
		}

		private void ctrl_green_on_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&G");
		}

		private void ctrl_blue_on_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&B");
		}

		private void ctrl_red_off_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&r");
		}

		private void ctrl_green_off_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&g");
		}

		private void ctrl_blue_off_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&b");
		}

		private void reset_btn_Click (object sender, EventArgs e)
		{
			serial_port.Write ("!");
		}

		private void receive_btn_Click (object sender, EventArgs e)
		{
			receive_lbl.Text = "";
			serial_port.Write ("&i");
		}

		private void display_btn_Click (object sender, EventArgs e)
		{
			serial_port.Write ("&s" + display_textbox.Text + ";");
		}

		private void serial_port_DataReceived (object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
		{
			System.IO.Ports.SerialPort sp = (System.IO.Ports.SerialPort) sender;
			string text = sp.ReadExisting ();
			if (receive_lbl.InvokeRequired)
				receive_lbl.Invoke (new MethodInvoker (delegate { receive_lbl.Text += text; }));
			else
				receive_lbl.Text += text;
		}
	}
}
