namespace ves_serial_control
{
	partial class MainForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose (bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose ();
			}
			base.Dispose (disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent ()
		{
			this.components = new System.ComponentModel.Container();
			this.reset_btn = new System.Windows.Forms.Button();
			this.led_box = new System.Windows.Forms.GroupBox();
			this.ctrl_blue_off = new System.Windows.Forms.Button();
			this.ctrl_blue_on = new System.Windows.Forms.Button();
			this.ctrl_green_off = new System.Windows.Forms.Button();
			this.ctrl_green_on = new System.Windows.Forms.Button();
			this.ctrl_red_off = new System.Windows.Forms.Button();
			this.ctrl_red_on = new System.Windows.Forms.Button();
			this.display_textbox = new System.Windows.Forms.TextBox();
			this.display_btn = new System.Windows.Forms.Button();
			this.receive_btn = new System.Windows.Forms.Button();
			this.receive_lbl = new System.Windows.Forms.Label();
			this.serial_port = new System.IO.Ports.SerialPort(this.components);
			this.com_port_cbx = new System.Windows.Forms.ComboBox();
			this.connect_btn = new System.Windows.Forms.Button();
			this.led_box.SuspendLayout();
			this.SuspendLayout();
			// 
			// reset_btn
			// 
			this.reset_btn.Location = new System.Drawing.Point(309, 11);
			this.reset_btn.Name = "reset_btn";
			this.reset_btn.Size = new System.Drawing.Size(75, 23);
			this.reset_btn.TabIndex = 0;
			this.reset_btn.Text = "Reset";
			this.reset_btn.UseVisualStyleBackColor = true;
			this.reset_btn.Click += new System.EventHandler(this.reset_btn_Click);
			// 
			// led_box
			// 
			this.led_box.Controls.Add(this.ctrl_blue_off);
			this.led_box.Controls.Add(this.ctrl_blue_on);
			this.led_box.Controls.Add(this.ctrl_green_off);
			this.led_box.Controls.Add(this.ctrl_green_on);
			this.led_box.Controls.Add(this.ctrl_red_off);
			this.led_box.Controls.Add(this.ctrl_red_on);
			this.led_box.Location = new System.Drawing.Point(12, 47);
			this.led_box.Name = "led_box";
			this.led_box.Size = new System.Drawing.Size(106, 80);
			this.led_box.TabIndex = 3;
			this.led_box.TabStop = false;
			this.led_box.Text = "LED Control";
			// 
			// ctrl_blue_off
			// 
			this.ctrl_blue_off.Location = new System.Drawing.Point(72, 48);
			this.ctrl_blue_off.Name = "ctrl_blue_off";
			this.ctrl_blue_off.Size = new System.Drawing.Size(27, 23);
			this.ctrl_blue_off.TabIndex = 8;
			this.ctrl_blue_off.Text = "0";
			this.ctrl_blue_off.UseVisualStyleBackColor = true;
			this.ctrl_blue_off.Click += new System.EventHandler(this.ctrl_blue_off_Click);
			// 
			// ctrl_blue_on
			// 
			this.ctrl_blue_on.BackColor = System.Drawing.Color.Blue;
			this.ctrl_blue_on.ForeColor = System.Drawing.Color.White;
			this.ctrl_blue_on.Location = new System.Drawing.Point(72, 19);
			this.ctrl_blue_on.Name = "ctrl_blue_on";
			this.ctrl_blue_on.Size = new System.Drawing.Size(27, 23);
			this.ctrl_blue_on.TabIndex = 7;
			this.ctrl_blue_on.Text = "1";
			this.ctrl_blue_on.UseVisualStyleBackColor = false;
			this.ctrl_blue_on.Click += new System.EventHandler(this.ctrl_blue_on_Click);
			// 
			// ctrl_green_off
			// 
			this.ctrl_green_off.Location = new System.Drawing.Point(39, 48);
			this.ctrl_green_off.Name = "ctrl_green_off";
			this.ctrl_green_off.Size = new System.Drawing.Size(27, 23);
			this.ctrl_green_off.TabIndex = 6;
			this.ctrl_green_off.Text = "0";
			this.ctrl_green_off.UseVisualStyleBackColor = true;
			this.ctrl_green_off.Click += new System.EventHandler(this.ctrl_green_off_Click);
			// 
			// ctrl_green_on
			// 
			this.ctrl_green_on.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(0)))));
			this.ctrl_green_on.ForeColor = System.Drawing.Color.White;
			this.ctrl_green_on.Location = new System.Drawing.Point(39, 19);
			this.ctrl_green_on.Name = "ctrl_green_on";
			this.ctrl_green_on.Size = new System.Drawing.Size(27, 23);
			this.ctrl_green_on.TabIndex = 5;
			this.ctrl_green_on.Text = "1";
			this.ctrl_green_on.UseVisualStyleBackColor = false;
			this.ctrl_green_on.Click += new System.EventHandler(this.ctrl_green_on_Click);
			// 
			// ctrl_red_off
			// 
			this.ctrl_red_off.Location = new System.Drawing.Point(6, 48);
			this.ctrl_red_off.Name = "ctrl_red_off";
			this.ctrl_red_off.Size = new System.Drawing.Size(27, 23);
			this.ctrl_red_off.TabIndex = 4;
			this.ctrl_red_off.Text = "0";
			this.ctrl_red_off.UseVisualStyleBackColor = true;
			this.ctrl_red_off.Click += new System.EventHandler(this.ctrl_red_off_Click);
			// 
			// ctrl_red_on
			// 
			this.ctrl_red_on.BackColor = System.Drawing.Color.Red;
			this.ctrl_red_on.ForeColor = System.Drawing.Color.White;
			this.ctrl_red_on.Location = new System.Drawing.Point(6, 19);
			this.ctrl_red_on.Name = "ctrl_red_on";
			this.ctrl_red_on.Size = new System.Drawing.Size(27, 23);
			this.ctrl_red_on.TabIndex = 3;
			this.ctrl_red_on.Text = "1";
			this.ctrl_red_on.UseVisualStyleBackColor = false;
			this.ctrl_red_on.Click += new System.EventHandler(this.ctrl_red_on_Click);
			// 
			// display_textbox
			// 
			this.display_textbox.Location = new System.Drawing.Point(142, 68);
			this.display_textbox.Name = "display_textbox";
			this.display_textbox.Size = new System.Drawing.Size(161, 20);
			this.display_textbox.TabIndex = 4;
			// 
			// display_btn
			// 
			this.display_btn.Location = new System.Drawing.Point(309, 66);
			this.display_btn.Name = "display_btn";
			this.display_btn.Size = new System.Drawing.Size(75, 23);
			this.display_btn.TabIndex = 5;
			this.display_btn.Text = "Display";
			this.display_btn.UseVisualStyleBackColor = true;
			this.display_btn.Click += new System.EventHandler(this.display_btn_Click);
			// 
			// receive_btn
			// 
			this.receive_btn.Location = new System.Drawing.Point(142, 95);
			this.receive_btn.Name = "receive_btn";
			this.receive_btn.Size = new System.Drawing.Size(75, 23);
			this.receive_btn.TabIndex = 6;
			this.receive_btn.Text = "ID";
			this.receive_btn.UseVisualStyleBackColor = true;
			this.receive_btn.Click += new System.EventHandler(this.receive_btn_Click);
			// 
			// receive_lbl
			// 
			this.receive_lbl.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.receive_lbl.Location = new System.Drawing.Point(223, 99);
			this.receive_lbl.Name = "receive_lbl";
			this.receive_lbl.Size = new System.Drawing.Size(161, 16);
			this.receive_lbl.TabIndex = 7;
			// 
			// serial_port
			// 
			this.serial_port.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(this.serial_port_DataReceived);
			// 
			// com_port_cbx
			// 
			this.com_port_cbx.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.com_port_cbx.FormattingEnabled = true;
			this.com_port_cbx.Location = new System.Drawing.Point(12, 13);
			this.com_port_cbx.Name = "com_port_cbx";
			this.com_port_cbx.Size = new System.Drawing.Size(121, 21);
			this.com_port_cbx.TabIndex = 8;
			// 
			// connect_btn
			// 
			this.connect_btn.Location = new System.Drawing.Point(139, 12);
			this.connect_btn.Name = "connect_btn";
			this.connect_btn.Size = new System.Drawing.Size(75, 23);
			this.connect_btn.TabIndex = 9;
			this.connect_btn.Text = "Connect";
			this.connect_btn.UseVisualStyleBackColor = true;
			this.connect_btn.Click += new System.EventHandler(this.connect_btn_Click);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(398, 137);
			this.Controls.Add(this.connect_btn);
			this.Controls.Add(this.com_port_cbx);
			this.Controls.Add(this.receive_lbl);
			this.Controls.Add(this.receive_btn);
			this.Controls.Add(this.display_btn);
			this.Controls.Add(this.display_textbox);
			this.Controls.Add(this.led_box);
			this.Controls.Add(this.reset_btn);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.Name = "MainForm";
			this.Text = "VES Serial Control";
			this.Shown += new System.EventHandler(this.MainForm_Shown);
			this.led_box.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button reset_btn;
		private System.Windows.Forms.GroupBox led_box;
		private System.Windows.Forms.Button ctrl_blue_off;
		private System.Windows.Forms.Button ctrl_blue_on;
		private System.Windows.Forms.Button ctrl_green_off;
		private System.Windows.Forms.Button ctrl_green_on;
		private System.Windows.Forms.Button ctrl_red_off;
		private System.Windows.Forms.Button ctrl_red_on;
		private System.Windows.Forms.TextBox display_textbox;
		private System.Windows.Forms.Button display_btn;
		private System.Windows.Forms.Button receive_btn;
		private System.Windows.Forms.Label receive_lbl;
		private System.IO.Ports.SerialPort serial_port;
		private System.Windows.Forms.ComboBox com_port_cbx;
		private System.Windows.Forms.Button connect_btn;
	}
}

