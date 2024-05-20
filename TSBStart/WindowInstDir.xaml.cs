using Microsoft.WindowsAPICodePack.Dialogs;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;


namespace TSBStart
{
    /// <summary>
    /// WindowInstDir.xaml 的交互逻辑
    /// </summary>
    public partial class WindowInstDir : Window
    {
        private Tools m_tool=new Tools();
        public WindowInstDir()
        {
            InitializeComponent();
        }
        
        private void refreshDiskSpaceInfo()
        {
            if(txtInsDir.Text.Trim().Length<=0)
            {
                txtDiskSpace.Text = "";
                return;
            }

            string sDriveName = txtInsDir.Text[0].ToString();
            double dgb = m_tool.GetHardDiskFreeSpace(sDriveName);
            txtDiskSpace.Text=m_tool.double2string(dgb);
        }

        private void btnStartSetup_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult= false;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            txtInsDir.Text = MainWindow.m_cfg.Inst_Dir;
            refreshDiskSpaceInfo();
        }


        public string InstDir
        {
            get { return txtInsDir.Text; }
        }
        private void btnSelDir_Click(object sender, RoutedEventArgs e)
        {

            CommonOpenFileDialog dlg = new CommonOpenFileDialog();
            dlg.IsFolderPicker = true;
            dlg.InitialDirectory = txtInsDir.Text;

            if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
            {
                txtInsDir.Text=dlg.FileName+System.IO.Path.DirectorySeparatorChar+ "1003B";
                refreshDiskSpaceInfo();
            }


        }
    }
}
