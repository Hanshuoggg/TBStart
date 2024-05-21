using System;
using System.Collections.Generic;
using System.Linq;
using System.Management;
using System.Text;
using System.Threading.Tasks;

namespace TSBStart
{
    public class OS
    {
        public static string GetCPUSerialNumber()
        {
            string cpuSerialNumber = string.Empty;
            ManagementClass mc = new ManagementClass("Win32_Processor");
            ManagementObjectCollection moc = mc.GetInstances();
            foreach (ManagementObject mo in moc)
            {
                cpuSerialNumber = mo.Properties["ProcessorId"].Value.ToString();
                break;
            }
            return cpuSerialNumber;
        }

        public static string GetMotherboardSerialNumber()
        {
            string motherboardSerialNumber = string.Empty;
            ManagementClass mc = new ManagementClass("Win32_BaseBoard");
            ManagementObjectCollection moc = mc.GetInstances();
            foreach (ManagementObject mo in moc)
            {
                motherboardSerialNumber = mo.Properties["SerialNumber"].Value.ToString();
                break;
            }
            return motherboardSerialNumber;
        }

        public static string GetHardDiskSerialNumber()
        {
            string hardDiskSerialNumber = string.Empty;
            ManagementClass mc = new ManagementClass("Win32_PhysicalMedia");
            ManagementObjectCollection moc = mc.GetInstances();
            foreach (ManagementObject mo in moc)
            {
                hardDiskSerialNumber = mo.Properties["SerialNumber"].Value.ToString();
                break;
            }
            return hardDiskSerialNumber;
        }


        public static string GetBiosSerialNumber()
        {
            string biosSerialNumber = string.Empty;
            ManagementClass mc = new ManagementClass("Win32_BIOS");
            ManagementObjectCollection moc = mc.GetInstances();
            foreach (ManagementObject mo in moc)
            {
                biosSerialNumber = mo.Properties["SerialNumber"].Value.ToString();
                break;
            }
            return biosSerialNumber;
        }



    }
}
