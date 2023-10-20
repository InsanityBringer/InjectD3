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
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;

namespace InjectD3Configuration
{
    public partial class Form1 : Form
    {
        D3Configuration config = new D3Configuration();
        Descent3Registry registry = new Descent3Registry();
        bool hasChanges = false;
        public Form1()
        {
            InitializeComponent();
            propertyGrid1.SelectedObject = config;
            try
            {
                config.ParseConfig("InjectD3.cfg");
            }
            catch (Exception exc)
            {
                StringBuilder msg = new StringBuilder();
                msg.Append("Failed to parse config file! Using default settings.\n\nException message:\n\n");
                msg.Append(exc.Message);
                msg.Append("\n\nStack trace:\n\n");
                msg.Append(exc.StackTrace);
                MessageBox.Show(msg.ToString());
            }
            Descent3RegistryConfig registryConfig = registry.GetConfig();
            textBox1.Text = registryConfig.CommandLine;
        }

        private void ExitButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void LaunchButton_Click(object sender, EventArgs e)
        {
            config.WriteToFile("InjectD3.cfg");
            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = "InjectD3.exe";
            info.Arguments = textBox1.Text;

            try
            {
                Process.Start(info);
            }
            catch (Exception exc)
            {
                MessageBox.Show(string.Format("Failed to start InjectD3: {0}.", exc.Message));
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            D3ConfigWindow window = new D3ConfigWindow();
            window.SetConfiguration(registry.GetConfig());
            if (window.ShowDialog() == DialogResult.OK)
            {
                Descent3RegistryConfig newConfig = window.GenerateConfiguration();
                newConfig.CommandLine = textBox1.Text;
                registry.SetConfig(newConfig);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (hasChanges)
            {
                DialogResult res = MessageBox.Show("Do you want to save the current settings?", "", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                if (res == DialogResult.Yes)
                {
                    config.WriteToFile("InjectD3.cfg");
                    Descent3RegistryConfig registryConfig = registry.GetConfig();
                    registryConfig.CommandLine = textBox1.Text;
                    registry.SetConfig(registryConfig);
                }

                if (res == DialogResult.Cancel)
                    e.Cancel = true;
            }

            if (!e.Cancel)
            {
                //On close, always write out the registry even if changes weren't made.
                //This is to ensure that the version key will always be written. 
                Descent3RegistryConfig registryConfig = registry.GetConfig();
                registry.SetConfig(registryConfig);
            }
        }

        private void propertyGrid1_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
        {
            hasChanges = true;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            hasChanges = true;
        }
    }
}
