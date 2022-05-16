/*-----------------------------------------------------------------------------
*
 *  Copyright (c) 2021 SaladBadger
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
-----------------------------------------------------------------------------*/

using System;
using System.ComponentModel;
using System.Globalization;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace InjectD3Configuration
{
    public enum ScreenModeType
    {
        Original,
        Windowed,
        NewFullscreen
    }
    public class D3Configuration
    {
        [Category("Graphics")]
        [DisplayName("Screen mode")]
        [Description("[GL ONLY] Changes the screen mode. Original is the original fullscreen mode. Windowed runs the game in a window. NewFullscreen runs the game at your desktop resolution.")]
        [DefaultValue(ScreenModeType.Windowed)]
        public ScreenModeType ScreenMode { get; set; } = ScreenModeType.Windowed;
        [Category("Graphics")]
        [DisplayName("Field of view")]
        [Description("Changes the field of view.")]
        [DefaultValue(72.0f)]
        public float DefaultFov { get; set; } = 72.0f;
        [Category("Graphics")]
        [DisplayName("Display adaptor")]
        [Description("The number of the display adaptor to use. -1 for default.")]
        [DefaultValue(-1)]
        public int DisplayNum { get; set; } = -1;
        [Category("Input")]
        [DisplayName("New mouse library")]
        [Description("Patches the DirectInput mouse code to a new raw input system. Vital for any of the new screen modes, but should also work in the original.")]
        [DefaultValue(true)]
        public bool PatchMouse { get; set; } = true;
        [Category("Input")]
        [DisplayName("Mouse prescalar")]
        [Description("Scales mouse movement by this multiplier before being read by the game.")]
        [DefaultValue(0.5f)]
        public float MousePrescalar { get; set; } = 0.5f;
        [Category("Audio")]
        [DisplayName("OpenAL sound code")]
        [Description("Patches the sound code to use OpenAL. This can fix crackling sounds and results in more involved positional audio. This is experimental and possibly buggy at the moment, but should be usable.")]
        [DefaultValue(false)]
        public bool PatchSoundSystem { get; set; } = false;
        [Category("Audio")]
        [DisplayName("OpenAL reverbs")]
        [Description("Makes the OpeanAL sound code use reverbs.")]
        [DefaultValue(true)]
        public bool UseReverbs { get; set; } = true;
        [Category("Graphics")]
        [DisplayName("Force -z32")]
        [Description("Uses 32-bit depth buffer. This is implied if using the new fullscreen mode.")]
        [DefaultValue(true)]
        public bool ForceZ32 { get; set; } = true;
        [Category("Graphics")]
        [DisplayName("Force 32-bit color")]
        [Description("Forces the game to run in 32-bit color mode.")]
        [DefaultValue(true)]
        public bool Force32BitColor { get; set; } = true;
        [Category("Graphics")]
        [DisplayName("Multisample count")]
        [Description("Enables multisampling with the specified number of samples. 1 = no multisampling.")]
        [DefaultValue(1)]
        public int MultiSampleCount { get; set; } = 1;
        [Category("General")]
        [DisplayName("Use HKEY_CURRENT_USER")]
        [Description("Makes the game store registry configuration in HKEY_CURRENT_USER, rather than HKEY_LOCAL_MACHINE. Makes config per-user and avoids needing adminstrator rights.")]
        [DefaultValue(false)]
        public bool UseHKEYCurrentUser { get; set; } = false;
        [Category("Graphics")]
        [DisplayName("Always use SSE")]
        [Description("Forces the use of SEE features, even on non-Intel processors. This enables features like powerup sparkles.")]
        [DefaultValue(true)]
        public bool AlwaysKatmai { get; set; } = true;
        [Category("Graphics")]
        [DisplayName("Enable OpenGL specular highlights")]
        [Description("Patches the specular mapping feature to work in OpenGL.")]
        [DefaultValue(true)]
        public bool OpenGLSpecular { get; set; } = true;
        [Category("Graphics")]
        [DisplayName("UI Framerate")]
        [Description("Sets the maximum framerate of the game's UI.")]
        [DefaultValue(20f)]
        public float UIFrameRate { get; set; } = 20f;

        [Category("Audio")]
        [DisplayName("OpenAL doppler factor")]
        [Description("Changes the doppler factor used by OpenAL. Higher values will result in greater pitch shifts while moving. Set to 0 to disable.")]
        [DefaultValue(0.6f)]
        public float DopplerFactor { get; set; } = 0.6f;

        public void ParseConfig(string filename)
        {
            StreamReader sr;
            try
            {
                sr = new StreamReader(filename);
            }
            catch (FileNotFoundException)
            {
                return;
            }
            while (!sr.EndOfStream)
            {
                try
                {
                    string str = sr.ReadLine();

                    if (str.Contains('='))
                    {
                        string[] parts = str.Split('=');
                        //TODO: make better?
                        switch (parts[0])
                        {
                            case "ScreenMode":
                                ScreenMode = (ScreenModeType)int.Parse(parts[1]);
                                break;
                            case "NewMouse":
                                PatchMouse = int.Parse(parts[1]) != 0;
                                break;
                            case "MouseScalar":
                                MousePrescalar = float.Parse(parts[1], CultureInfo.InvariantCulture.NumberFormat);
                                break;
                            case "NewSoundSystem":
                                PatchSoundSystem = int.Parse(parts[1]) != 0;
                                break;
                            case "NewSoundSystemReverbs":
                                UseReverbs = int.Parse(parts[1]) != 0;
                                break;
                            case "Force32Bit":
                                Force32BitColor = int.Parse(parts[1]) != 0;
                                break;
                            case "ForceZ32":
                                ForceZ32 = int.Parse(parts[1]) != 0;
                                break;
                            case "MultisampleCount":
                                MultiSampleCount = int.Parse(parts[1]);
                                break;
                            case "DisplayNum":
                                DisplayNum = int.Parse(parts[1]);
                                break;
                            case "FieldOfView":
                                DefaultFov = float.Parse(parts[1], CultureInfo.InvariantCulture.NumberFormat);
                                break;
                            case "UseUserRegistry":
                                UseHKEYCurrentUser = int.Parse(parts[1]) != 0;
                                break;
                            case "AlwaysKatmai":
                                AlwaysKatmai = int.Parse(parts[1]) != 0;
                                break;
                            case "OpenGLSpecular":
                                OpenGLSpecular = int.Parse(parts[1]) != 0;
                                break;
                            case "UIFrameRate":
                                UIFrameRate = float.Parse(parts[1], CultureInfo.InvariantCulture.NumberFormat);
                                break;
                            case "DopplerFactor":
                                DopplerFactor = float.Parse(parts[1], CultureInfo.InvariantCulture.NumberFormat);
                                break;
                        }
                    }
                }
                catch (Exception exc)
                {
                    sr.Close();
                    throw exc;
                }
            }
            sr.Close();
        }

        public void WriteToFile(string filename)
        {
            StreamWriter sw = new StreamWriter(filename);
            sw.WriteLine("ScreenMode={0}", (int)ScreenMode);
            sw.WriteLine("NewMouse={0}", PatchMouse ? 1 : 0);
            sw.WriteLine("MouseScalar={0}", MousePrescalar.ToString("G", CultureInfo.InvariantCulture));
            sw.WriteLine("NewSoundSystem={0}", PatchSoundSystem ? 1 : 0);
            sw.WriteLine("NewSoundSystemReverbs={0}", UseReverbs ? 1 : 0);
            sw.WriteLine("Force32Bit={0}", Force32BitColor ? 1 : 0);
            sw.WriteLine("ForceZ32={0}", ForceZ32 ? 1 : 0);
            sw.WriteLine("MultisampleCount={0}", MultiSampleCount);
            sw.WriteLine("DisplayNum={0}", DisplayNum);
            sw.WriteLine("FieldOfView={0}", DefaultFov.ToString("G", CultureInfo.InvariantCulture));
            sw.WriteLine("UseUserRegistry={0}", UseHKEYCurrentUser ? 1 : 0);
            sw.WriteLine("AlwaysKatmai={0}", AlwaysKatmai ? 1 : 0);
            sw.WriteLine("OpenGLSpecular={0}", OpenGLSpecular ? 1 : 0);
            sw.WriteLine("UIFrameRate={0}", UIFrameRate.ToString("G", CultureInfo.InvariantCulture));
            sw.WriteLine("DopplerFactor={0}", DopplerFactor.ToString("G", CultureInfo.InvariantCulture));

            sw.Close();
            sw.Dispose();
        }
    }
}
