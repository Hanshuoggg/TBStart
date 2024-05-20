using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Transactions;
using System.Windows;

namespace TSBStart
{
   
    public class Tools
    {
 
        public string get7zipPath()
        {
            return getStartPath() + Path.DirectorySeparatorChar + "dll" + Path.DirectorySeparatorChar + "7zip";
        }
        public string getStartPath()
        {
            return System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
        }

        //调用厂商提供的DLL API校验帐号和密码是否有效
        public bool verifyLogin(string saccount,string spwd)
        {
            //调用厂商接口（验证帐户名密码），如果验证通过，则返回true，验证失败，则返回false
            return true;  //先返回true允许登录成功，后续需要替换为对厂商API的真实调用
        }

        public void showSimpleErrorMsgBox(string smsg)
        {
            MessageBox.Show(smsg, "错误");

        }
        public void showExceptionMsgBox(Exception ex)
        {
            MessageBox.Show(ex.ToString(), "异常");

        }

        /// <summary>  

        /// 获取指定驱动器的剩余空间总大小(单位为GB)  

        /// </summary>  

        /// <param name=”HardDiskName”>只需输入代表驱动器的字母即可 </param>  

        /// <returns> </returns>  

        public  double GetHardDiskFreeSpace(string HardDiskName)

        {

            double freeSpace=0;

            HardDiskName = HardDiskName + ":\\";
            HardDiskName= HardDiskName.ToUpper();

            System.IO.DriveInfo[] drives = System.IO.DriveInfo.GetDrives();

            foreach (System.IO.DriveInfo drive in drives)

            {

                if (drive.Name == HardDiskName)

                {

                    freeSpace = (double)drive.TotalFreeSpace / 1024 /1024 /1024;

                }

            }

            return freeSpace;

        }

        public string double2string(double dvalue, int max_decimal_places = 2)
        {

            string s = dvalue.ToString();
            int idotIndex = s.IndexOf('.');
            if (idotIndex == -1)
            {
                return s;
            }

            //截断为2位小数
            string svalue = s.Substring(0, idotIndex+ max_decimal_places+1);            

            //从后向前移除为0的小数
            for(int i= max_decimal_places - 1;i>=0;i--)
            {
                if (svalue[svalue.Length - 1] == '0')
                    svalue = svalue.Remove(svalue.Length - 1);
                else
                    break;

            }

            //如果小数位都删除了，则删除小数点
            if (svalue[svalue.Length - 1] == '.')
            {
                svalue = svalue.Remove(svalue.Length - 1);
            }
           
            return svalue;
        }


    }
}
