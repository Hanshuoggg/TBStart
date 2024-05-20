using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TSBStart
{
    public class HostObject
    {
        public void LoginOK(String username, String pwd)
        {
            MessageBox.Show("登录成功,用户名:" + username + " 密码:" + pwd);
            MainWindow.m_cfg.Account = username;
            MainWindow.m_cfg.Password = pwd;

            MainWindow.m_cfg.saveConfig();

           
        }

        public void MinWindow()
        {
           
        }

        public void CloseWindow()
        {

        }

        public void btnStartOrUpdate_Click()
        {

        }

    }
}
