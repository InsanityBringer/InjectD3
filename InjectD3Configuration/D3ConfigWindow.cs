using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace InjectD3Configuration
{
    public partial class D3ConfigWindow : Form
    {
        public D3ConfigWindow()
        {
            InitializeComponent();
        }

        public void SetConfiguration(Descent3RegistryConfig config)
        {
            InternetSpeedComboBox.Text = config.ConnectionSpeed;
            CHFlightstickCheckBox.Checked = config.EnableCHFlight;
            FFCheckBox.Checked = config.EnableJoystickFF;
            MousemanCheckBox.Checked = config.EnableMouseman;
            DisableFindFastCheckBox.Checked = config.FindFastDisable;
            LanguageComboBox.SelectedIndex = config.LanguageType;
            KeyboardComboBox.Text = config.KeyboardType;
            ConnectionComboBox.Text = config.NetworkConnection;
            DetailComboBox.SelectedIndex = config.PredefDetailSetting;
            RendererComboBox.SelectedIndex = config.PreferredRenderer;
            ProfanityCheckBox.Checked = config.ProfanityPrevention;
            VSyncCheckBox.Checked = config.RSVsync;
        }

        public Descent3RegistryConfig GenerateConfiguration()
        {
            Descent3RegistryConfig config;

            config.ConnectionSpeed = InternetSpeedComboBox.Text;
            config.EnableCHFlight = CHFlightstickCheckBox.Checked;
            config.EnableJoystickFF = FFCheckBox.Checked;
            config.EnableMouseman = MousemanCheckBox.Checked;
            config.FindFastDisable = DisableFindFastCheckBox.Checked;
            config.LanguageType = LanguageComboBox.SelectedIndex;
            config.KeyboardType = KeyboardComboBox.Text;
            config.NetworkConnection = ConnectionComboBox.Text;
            config.PredefDetailSetting = DetailComboBox.SelectedIndex;
            config.PreferredRenderer = RendererComboBox.SelectedIndex;
            config.ProfanityPrevention = ProfanityCheckBox.Checked;
            config.RSVsync = VSyncCheckBox.Checked;
            config.SoundMixer = 1;

            return config;
        }
    }
}
