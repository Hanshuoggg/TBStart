using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TSBStart
{
    public delegate void DeExtracting(string sfilename);
    public class Zip7z
    {


        public event DeExtracting? Extracting = null;
        private Tools m_tool=new Tools();
        public void CreateZip(string sourceName, string targetArchive)
        {
            string s7zexe = m_tool.get7zipPath() + System.IO.Path.DirectorySeparatorChar + "7za.exe";

            try
            {
                ProcessStartInfo p = new ProcessStartInfo();
                p.FileName = s7zexe;
                p.Arguments = string.Format("a -tgzip \"{0}\" \"{1}\" -mx=9", targetArchive, sourceName);
                //p.WindowStyle = ProcessWindowStyle.Hidden;
                //p.UseShellExecute = false;
                //p.RedirectStandardOutput = true;
                //p.RedirectStandardInput = true;
                //p.RedirectStandardError = true;
                //p.CreateNoWindow = true;


                m_process = Process.Start(p);

                m_process.WaitForExit();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message); 
            }
        }

        private Process? m_process;
        public void ExtractFile(string sourceArchive, string destination)
        {
            string s7zexe = m_tool.get7zipPath() + System.IO.Path.DirectorySeparatorChar + "7za.exe";
            try
            {
                ProcessStartInfo p = new ProcessStartInfo();
                //p.WindowStyle = ProcessWindowStyle.Hidden;
                p.FileName = s7zexe;
                p.Arguments = string.Format("x \"{0}\" -y -bb3 -o\"{1}\"", sourceArchive, destination);
                p.UseShellExecute = false;
                p.RedirectStandardOutput = true;
                //p.RedirectStandardInput = true;
                //p.RedirectStandardError = true;
                p.CreateNoWindow = true;


               
                m_process = Process.Start(p);
                m_process.OutputDataReceived += M_process_OutputDataReceived;
                m_process.BeginOutputReadLine();
                m_process.WaitForExit();
            }
            catch (System.Exception ex)
            {
                m_tool.showExceptionMsgBox(ex);
            }
        }

        private void M_process_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if(e.Data != null)
            {
                if(Extracting!=null)
                    Extracting(e.Data);
            }
        }

        public void Cancel()
        {
            try
            {
                if(m_process!=null)
                    m_process.Kill();
                m_process = null;
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }


    }
}
