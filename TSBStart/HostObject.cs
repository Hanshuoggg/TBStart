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
        public HostObject(MainWindow mw) {
        
            m_mainFrm = mw;
        }

        private MainWindow m_mainFrm = null;
        //登录成功
        public void LoginOK()
        {


        }
        //游戏是否存在,存在返回1，不存在返回0
        public int existGame()
        {
            return 0;

        }
        public void startGame(String username, String pwd)
        {
            MessageBox.Show("登录成功,用户名:" + username + " 密码:" + pwd);
            MainWindow.m_cfg.Account = username;
            MainWindow.m_cfg.Password = pwd;

            MainWindow.m_cfg.saveConfig();

           
        }

        public void MinWindow()
        {
            MessageBox.Show("最小化");
            if(m_mainFrm!=null)
                m_mainFrm.WindowState=System.Windows.WindowState.Minimized;
        }

        public void CloseWindow()
        {
            MessageBox.Show("关闭");
            if (m_mainFrm != null)
                m_mainFrm.Close();
        }

        public string appGetUuid()
        {
            return "abc";
        }

        public void btnStartOrUpdate_Click()
        {

        }



    }
}
