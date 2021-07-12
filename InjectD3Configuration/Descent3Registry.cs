using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Win32;

namespace InjectD3Configuration
{
    public struct Descent3RegistryConfig
    {
        public string ConnectionSpeed;
        public bool EnableCHFlight;
        public bool EnableJoystickFF;
        public bool EnableMouseman;
        public bool FindFastDisable;
        public string KeyboardType;
        public int LanguageType;
        public string NetworkConnection;
        public int PredefDetailSetting;
        public int PreferredRenderer;
        public bool ProfanityPrevention;
        public bool RSVsync;
        public int SoundMixer;
    }
    public class Descent3Registry
    {
        RegistryKey Descent3Key;

        public Descent3Registry()
        {
            RegistryKey root = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, RegistryView.Registry32);
            RegistryKey softwareKey = root.OpenSubKey("SOFTWARE", true);
            RegistryKey outrageKey = softwareKey.OpenSubKey("Outrage", true);
            if (outrageKey == null)
            {
                softwareKey.CreateSubKey("Outrage");
                outrageKey = softwareKey.OpenSubKey("Outrage", true);
            }
            softwareKey.Close();
            Descent3Key = outrageKey.OpenSubKey("Descent3", true);
            if (Descent3Key == null)
            {
                outrageKey.CreateSubKey("Descent3");
                Descent3Key = outrageKey.OpenSubKey("Descent3", true);
                InitDefaultOptions();
            }
        }

        public void InitDefaultOptions()
        {
            Descent3Key.SetValue("ConnectionSpeed", "Fast");
            Descent3Key.SetValue("EnableCHFlight", 0);
            Descent3Key.SetValue("EnableJoystickFF", 0);
            Descent3Key.SetValue("EnableMouseman", 0);
            Descent3Key.SetValue("FindFastDisable", 1);
            Descent3Key.SetValue("KeyboardType", "United States");
            Descent3Key.SetValue("LanguageType", 0);
            Descent3Key.SetValue("NetworkConnection", "LAN");
            Descent3Key.SetValue("PredefDetailSetting", 3);
            Descent3Key.SetValue("PreferredRenderer", 2);
            Descent3Key.SetValue("ProfanityPrevention", 0);
            Descent3Key.SetValue("RS_vsync", 0);
            Descent3Key.SetValue("SoundMixer", 1);
        }

        public Descent3RegistryConfig GetConfig()
        {
            Descent3RegistryConfig config;
            config.ConnectionSpeed = (string)Descent3Key.GetValue("ConnectionSpeed", "Fast");
            config.EnableCHFlight = (int)Descent3Key.GetValue("EnableCHFlight", 0) == 1;
            config.EnableJoystickFF = (int)Descent3Key.GetValue("EnableJoystickFF", 0) == 1;
            config.EnableMouseman = (int)Descent3Key.GetValue("EnableMouseman", 0) == 1;
            config.FindFastDisable = (int)Descent3Key.GetValue("FindFastDiable", 1) == 1;
            config.KeyboardType = (string)Descent3Key.GetValue("KeyboardType", "United States");
            config.LanguageType = (int)Descent3Key.GetValue("LanguageType", 0);
            config.NetworkConnection = (string)Descent3Key.GetValue("KeyboardType", "LAN");
            config.PredefDetailSetting = (int)Descent3Key.GetValue("PredefDetailSetting", 3);
            config.PreferredRenderer = (int)Descent3Key.GetValue("PreferredRenderer", 2);
            config.ProfanityPrevention = (int)Descent3Key.GetValue("ProfanityPrevention", 1) == 1;
            config.RSVsync = (int)Descent3Key.GetValue("RS_vsync", 1) == 1;
            config.SoundMixer = (int)Descent3Key.GetValue("SoundMixer", 1);

            return config;
        }

        public void SetConfig(Descent3RegistryConfig config)
        {
            Descent3Key.SetValue("ConnectionSpeed", config.ConnectionSpeed);
            Descent3Key.SetValue("EnableCHFlight", config.EnableCHFlight ? 1 : 0);
            Descent3Key.SetValue("EnableJoystickFF", config.EnableJoystickFF ? 1 : 0);
            Descent3Key.SetValue("EnableMouseman", config.EnableMouseman ? 1 : 0);
            Descent3Key.SetValue("FindFastDisable", config.FindFastDisable ? 1 : 0);
            Descent3Key.SetValue("KeyboardType", config.KeyboardType);
            Descent3Key.SetValue("LanguageType", config.LanguageType);
            Descent3Key.SetValue("NetworkConnection", config.NetworkConnection);
            Descent3Key.SetValue("PredefDetailSetting", config.PredefDetailSetting);
            Descent3Key.SetValue("PreferredRenderer", config.PreferredRenderer);
            Descent3Key.SetValue("ProfanityPrevention", config.ProfanityPrevention ? 1 : 0);
            Descent3Key.SetValue("RS_vsync", config.RSVsync ? 1 : 0);
            Descent3Key.SetValue("SoundMixer", config.SoundMixer);
        }
    }
}
