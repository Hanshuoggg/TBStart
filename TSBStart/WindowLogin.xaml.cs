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
    /// WindowLogin.xaml 的交互逻辑
    /// </summary>
    public partial class WindowLogin : Window
    {

        private Config m_cfg=new();
        private Tools m_tool=new();
        public WindowLogin()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {

            txtAccount.Text=m_cfg.Account;
            txtPassword.Password=m_cfg.Password;


        }

        private void btnLogin_Click(object sender, RoutedEventArgs e)
        {
            if(txtAccount.Text.Trim().Length==0)
            {
                m_tool.showSimpleErrorMsgBox("帐号不可为空！");
                return;
            }
            if(txtPassword.Password.Trim().Length==0)
            {
                m_tool.showSimpleErrorMsgBox("密码不可为空！");
                return;

            }

            //调用厂商API校验帐号和密码是否有效
            bool bret= m_tool.verifyLogin(txtAccount.Text, txtPassword.Password);
            if (true==bret)
            {
                //有效则将帐号和密码 保存到ini配置文件
                m_cfg.Account=txtAccount.Text;
                m_cfg.Password=txtPassword.Password;
                m_cfg.saveConfig();

                //关闭登录框
                DialogResult = true;

            }
            else
            {
                m_tool.showSimpleErrorMsgBox("无法登录，您可能输入了错误的帐号名和密码，请检查后再次重试！");
            }

           
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult=false;
        }
    }
}
