using System;
using System.Collections.Generic;
using System.IO;
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
            string sGameExe = MainWindow.m_cfg.GameExePath;
            if (File.Exists(sGameExe) == false)
            {
                return 0;
            }
            else
            {
                return 1;
            }

        }
        public void startGame(String username, String pwd)
        {
            MainWindow.m_cfg.Account = username;
            MainWindow.m_cfg.Password = pwd;
            MainWindow.m_cfg.saveConfig();

           
        }

        public void MinWindow()
        {
            if(m_mainFrm!=null)
                m_mainFrm.WindowState=System.Windows.WindowState.Minimized;
        }

        public void CloseWindow()
        {
            if (m_mainFrm != null)
                m_mainFrm.Close();
        }

        public string appGetUuid()
        {
            string scpuid= OS.GetCPUSerialNumber();
            return scpuid;
        }

        public void set_download_url(string url)
        {
            if (null == url || url.Trim().Length <= 0)
                return;

            MainWindow.m_cfg.First_Download_Url = url;
            MainWindow.m_cfg.saveConfig();

        }

        public void btnStartOrUpdate_Click()
        {
            if (m_mainFrm != null)
                m_mainFrm.btnStartOrUpdate_Click(null, null);
        }



    }
}
