using IniParser;
using IniParser.Model;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TSBStart
{
    public class Config
    {
        private Tools m_tool=new Tools();
        private string m_sIniFile = "";
        private FileIniDataParser? m_iniParse = null;
        private IniData? m_ini = null;

        public Config() {

            m_sIniFile = m_tool.getStartPath() + Path.DirectorySeparatorChar+"TSBStart.ini";
            readConfig();
        }

        public void readConfig()
        {
            m_iniParse= new FileIniDataParser(); 
            m_ini = m_iniParse.ReadFile(m_sIniFile);
        }

        public void saveConfig()
        {
            if (m_iniParse != null && m_sIniFile != null && m_ini != null)
            {
            	m_iniParse.WriteFile(m_sIniFile, m_ini);
        	}
        }

        public string? First_Download_Url
        {
            get 
            {
                if (m_ini != null)
                {
        	        string s = m_ini.Sections["GAME OPTION"]["First_Download_Url"];
    	            return s;
	            }
                else
                    return null;
            }
        }

        public string? Inst_Dir
        {
            get
            {
                if (m_ini != null)
                {
                	string s = m_ini.Sections["GAME OPTION"]["Inst_Dir"];
                	return s;
	            }
                else
                    return null;
            }
            set
            {
                if (m_ini != null)
                {
                	m_ini.Sections["GAME OPTION"]["Inst_Dir"] = value;
	            }
	        }
        }

        public string AccountVerifyUrl
        {
            get
            {
                if (m_ini != null)
                {
                    string s = m_ini.Sections["GAME OPTION"]["AccountVerifyUrl"];
                    return s;
                }
                else
                    return null;
            }
            set
            {
                if (m_ini != null)
                {
                    m_ini.Sections["GAME OPTION"]["AccountVerifyUrl"] = value;
                }
            }
        }

        public string GameExePath
        {
            get
            {
                return Inst_Dir + Path.DirectorySeparatorChar + "game.exe";
            }
        }

        public string UpdateExePath
        {
            get
            {
                return Inst_Dir + Path.DirectorySeparatorChar + "1003b.exe";
            }
        }

        public string? Account
        {
            get
            {
                if (m_ini != null)
                {
	                string s = m_ini.Sections["GAME OPTION"]["Account"];
    	            return s;
        	    }
                else
                    return null;
            }
            set
            {
                if (m_ini != null)
                {
            	    m_ini.Sections["GAME OPTION"]["Account"] = value;
	            }
    	    }
        }

        public string? Password
        {
            get
            {
                if (m_ini != null)
                {
                	string s = m_ini.Sections["GAME OPTION"]["Password"];
	                return s;
                }
                else
                    return null;

            }
            set
            {
                if (m_ini != null)
                {
                	m_ini.Sections["GAME OPTION"]["Password"] = value;
                }
            }
        }
    }
}
