using Downloader;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows;
using System.Windows.Interop;
using Patcher;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.StartPanel;
using System.Xml.XPath;
using System.Runtime.InteropServices;
using Microsoft.Web.WebView2.Wpf;
using Microsoft.Web.WebView2.WinForms;

namespace TSBStart
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private string m_sConnServerTip = "正在连接服务器...";
        private enum PatchStep
        {
            eSTEP_GET_SERVER_VERSION,
            eSTEP_GET_PATCH_VERSION_LIST,
            eSTEP_GET_FILE_LIST,
            eSTEP_GET_PATCH_FILES,
            eSTEP_PATCH_DONE,
        };

        public static  Config m_cfg = new();
        private Tools m_tool=new();
        private Zip7z m_z7 = new();

        private System.Timers.Timer? aPatchTimer;
        private System.Timers.Timer? aUnzipTimer;

        private bool bGame_Is_Run;
        private PatchStep iDownload_Step = PatchStep.eSTEP_GET_SERVER_VERSION;
        bool _isVersionChecked = false;
        bool _isPatchStarted = false;
        float _lastDownloadPercent = 0.0f;

        private string? m_slocalZipFile;

        public MainWindow()
        {
            InitializeComponent();

            // 윈도우 핸들이 생성된 후 메시지 처리기를 추가합니다.
            this.Loaded += MainWindow_Loaded;
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            WindowInteropHelper helper = new(this);
            if (helper.Handle == IntPtr.Zero)
            {
                return;
            }
            HwndSource source = HwndSource.FromHwnd(helper.Handle);
            source.AddHook(WndProc);
        }

        private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            const int WM_USER = 0x0400;
            const int WM_USER_GetListDone = WM_USER + 131;
            const int WM_USER_GetFTPDone = WM_USER + 132;

            switch (msg)
            {
                case WM_USER_GetListDone:
                    HandleGetListDone();
                    handled = true;
                    break;
                case WM_USER_GetFTPDone:
                    // 메시지 처리 로직
                    HandleGetFTPDone();
                    handled = true;
                    break;
            }
            return IntPtr.Zero;
        }

        private void HandleGetListDone()
        {
            if (!_isVersionChecked)
            {
                FTPWorker.DisconnetPatchServer();
            }

            if (!_isVersionChecked && FTPWorker.CompareVersion() == 1)
            {
                iDownload_Step = PatchStep.eSTEP_PATCH_DONE;
                UpdateUIForGameStart();
            }
            else
            {
                btnStartOrUpdate.Content = "开始升级";
            }

            _isVersionChecked = true;
            StartPatchIfRequired();

            UpdateUIWhenPatchDone();


            if(btnStartOrUpdate.Content.Equals("开始升级"))
            {
                btnStartOrUpdate_Click(null, null);
            }
        }

        private void HandleGetFTPDone()
        {
            btnStartOrUpdate.Content = "开始游戏";
            if (aPatchTimer != null) aPatchTimer.Stop();
        }

        private void UpdateUIForGameStart()
        {
            btnStartOrUpdate.Content = "开始游戏";
            btnStartOrUpdate.IsEnabled = true;
            if (aPatchTimer != null) aPatchTimer.Stop();
        }

        private void StartPatchIfRequired()
        {
            if (_isPatchStarted && iDownload_Step < PatchStep.eSTEP_PATCH_DONE)
            {
                iDownload_Step++;
                FTPWorker.PatchStart((int)iDownload_Step);
            }
        }

        private void UpdateUIWhenPatchDone()
        {
            if (iDownload_Step >= PatchStep.eSTEP_PATCH_DONE)
            {
                iDownload_Step = PatchStep.eSTEP_PATCH_DONE;
                btnStartOrUpdate.Content = "开始游戏";
                btnStartOrUpdate.IsEnabled = true;
                if (aPatchTimer != null) aPatchTimer.Stop();
            }
        }

        private void startUpdate()
        {
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                btnStartOrUpdate.IsEnabled = false;

            });

            try
            {
                if (_isVersionChecked && !_isPatchStarted)
                {
                    FTPWorker.ConnectPatchServer();
                    _isPatchStarted = true;
                    FTPWorker.PatchStart((int)++iDownload_Step);
                    if (null == aPatchTimer)
                    {
                        aPatchTimer = new System.Timers.Timer(2000);
                        aPatchTimer.AutoReset = true; // 设置为 false，以便只触发一次。
                        aPatchTimer.Enabled = true;
                    }
                    aPatchTimer.Elapsed += ATimer_Patch;

                }
            }
            catch (Exception ex)
            {
                m_tool.showExceptionMsgBox(ex);
            }
        }

        private void ATimer_Patch(object? sender, System.Timers.ElapsedEventArgs e)
        {
            ulong m_llDownSize = FTPWorker.GetDownloadSize();
            ulong m_lDownSpeed = FTPWorker.GetDownloadSpeed();
            
            float fDownPercent = m_llDownSize / (float)FTPWorker.GetTotalSize();
            if (_lastDownloadPercent == fDownPercent)
            {
                return;
            }
            _lastDownloadPercent = fDownPercent;
            string testText = (m_lDownSpeed / 1024.0f) + "MB/s [ " + fDownPercent * 100 + " ]";
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                labInfo.Content = testText;
            });
        }

        private void startGame()
        {
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                btnStartOrUpdate.IsEnabled = false;
            });

            try
            {
                if (_isVersionChecked == true && _isPatchStarted == true)
                {
                    FTPWorker.UpdateVersion(true);
                    _isPatchStarted = false;
                }
                else
                {
                    FTPWorker.UpdateVersion(false);
                }

                ProcessStartInfo p = new ProcessStartInfo();
                m_cfg.readConfig();
                p.FileName = m_cfg.GameExePath;
                p.WorkingDirectory = m_cfg.Inst_Dir;
                string parameters = m_cfg.Account + " " + m_cfg.Password;

                // 조건에 따라 파라미터 추가
                {
                    p.Arguments = parameters;
                }

                //调用厂商接口（启动游戏）：  此处的启动游戏进程应改为调用厂商DLL接口， 通过传入用游戏帐号和密码，直接启动游戏无需玩家二次输入
                Process process = Process.Start(p);
                process.WaitForExit();
            }
            catch(Exception ex)
            {
                m_tool.showExceptionMsgBox(ex);
            }

            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                btnStartOrUpdate.IsEnabled = true;
            });
        }

        public void btnStartOrUpdate_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (btnStartOrUpdate.Content.Equals("开始安装"))
                {
                    //选择安装目录 
                    WindowInstDir wInstDir = new();
                    wInstDir.Owner = this;
                    bool? bret = (bool?)wInstDir.ShowDialog();
                    if (false == bret)
                    {
                        return;
                    }

                    m_cfg.Inst_Dir = wInstDir.InstDir;
                    m_cfg.saveConfig();

                    string sfilename = System.IO.Path.GetFileName(m_cfg.First_Download_Url);
                    m_slocalZipFile = m_cfg.Inst_Dir + System.IO.Path.DirectorySeparatorChar + sfilename;

                    if (Directory.Exists(m_cfg.Inst_Dir) == false)
                    {
                        Directory.CreateDirectory(m_cfg.Inst_Dir);
                    }

                    if (File.Exists(m_slocalZipFile) == true)
                        File.Delete(m_slocalZipFile);

                    if (m_cfg.First_Download_Url != null)
                    {
                        downFirstZip(m_cfg.First_Download_Url, m_slocalZipFile);
                    }
                }
                else if (btnStartOrUpdate.Content.Equals(m_sConnServerTip))
                {
                    MessageBox.Show("正在连接游戏服务器...请勿关闭登录器程序!");
                }
                else if (btnStartOrUpdate.Content.Equals("开始升级"))
                {
                    //杀死所有运行的游戏进程,再进行升级
                    killGameExe();
                    Thread.Sleep(1500);
                    Thread thStartUpdate = new Thread(new ThreadStart(startUpdate));
                    thStartUpdate.Start();
                }
                else if (btnStartOrUpdate.Content.Equals("开始游戏"))
                {
                    Thread thStartGame = new Thread(new ThreadStart(startGame));
                    thStartGame.Start();
                }
                else if(btnStartOrUpdate.Content.Equals("取消安装"))
                {
                    if (downloader != null)
                    {
                        downloader.Pause();
                    }
                    btnStartOrUpdate.Content = "继续安装";
                }
                else if(btnStartOrUpdate.Content.Equals("继续安装"))
                {
                    if (downloader != null)
                    {
                        downloader.Resume();
                    }
                    btnStartOrUpdate.Content = "取消安装";
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void TryConnectPatchServer()
        {
            _lastDownloadPercent = 0;

            if (m_cfg.Inst_Dir != null)
            {
                FTPWorker.GetCurrentExeDir(m_cfg.Inst_Dir);
            }

            bGame_Is_Run = FTPWorker.CheckGameRunning();
            if (bGame_Is_Run == false)
            {
                FTPWorker.CheckExistPrePatch();
            }

            // download Start
            FTPWorker.LoadINI();
            FTPWorker.ConnectPatchServer();
            iDownload_Step = PatchStep.eSTEP_GET_SERVER_VERSION;
            FTPWorker.PatchStart((int)iDownload_Step);
            //调用厂商接口（是否需要升级），此处需要调用厂商接口，查询当前游戏是否需要升级，是则返回true,否则返回false
        }

        //调用游戏开发商提供的API检测当前是否需要升级
        private bool isNeedUpdate()
        {
            if (FTPWorker.CompareVersion() == 1)
            {
                return false;
            }
            else
                return true;
            //调用厂商接口（是否需要升级），此处需要调用厂商接口，查询当前游戏是否需要升级，是则返回true,否则返回false
        }

        private void  UnZip()
        {
            try
            {
                string? sZipFile = m_slocalZipFile;
                string? UnZipDir = m_cfg.Inst_Dir;

                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
                {
                    labInfo.Content = "开始解压缩游戏文件，请耐心等待解压完成...";
                    btnStartOrUpdate.IsEnabled = false;

                });

                if (sZipFile != null && UnZipDir != null)
                {
                    m_z7.ExtractFile(sZipFile, UnZipDir);
                }

                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
                {
                    labInfo.Content = "安装完成";
                    //btnStartOrUpdate.Content = "开始游戏";
                    btnStartOrUpdate.IsEnabled = true;
                });
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void M_z7_Extracting(string sfilename)
        {
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                labInfo.Content = string.Format("正在解压文件:{0}...，请耐心等待",sfilename);
            });
        }

        private DownloadConfiguration? downloadOpt = null;
        private DownloadService? downloader = null;
        private async void downFirstZip(string surlFile,string slocalFile)
        {
            try
            {
                downloadOpt = new DownloadConfiguration
                {
                    BufferBlockSize = 10240,
                    ChunkCount = 8,
                    MaximumBytesPerSecond = 1024 * 1024,
                    // 更多配置项...
                };

                downloader = new DownloadService(downloadOpt);
                // Provide `FileName` and `TotalBytesToReceive` at the start of each download
                downloader.DownloadStarted += Downloader_DownloadStarted;

                // Provide any information about chunker downloads, 
                // like progress percentage per chunk, speed, 
                // total received bytes and received bytes array to live streaming.
                downloader.ChunkDownloadProgressChanged += Downloader_ChunkDownloadProgressChanged;

                // Provide any information about download progress, 
                // like progress percentage of sum of chunks, total speed, 
                // average speed, total received bytes and received bytes array 
                // to live streaming.
                downloader.DownloadProgressChanged += Downloader_DownloadProgressChanged;

                // Download completed event that can include errors or 
                // canceled or download completed successfully.
                downloader.DownloadFileCompleted += Downloader_DownloadFileCompleted;

                m_progress.Visibility = Visibility.Visible;
                m_progress.Minimum = 0;
                m_progress.Maximum = 100;

                await downloader.DownloadFileTaskAsync(surlFile, slocalFile);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void Downloader_DownloadFileCompleted(object? sender, System.ComponentModel.AsyncCompletedEventArgs e)
        {
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                labInfo.Content = "下载完成";
                m_progress.Value = 0;
                m_progress.Visibility = Visibility.Hidden;

                //解压缩下载后的文件，到游戏安装目录 
                // 创建一个间隔为两秒的定时器。
                if (null == aUnzipTimer)
                {
                    aUnzipTimer = new System.Timers.Timer(2000);
                    // 钩住定时器的 Elapsed 事件。
                    aUnzipTimer.Elapsed += timer_unzip_and_check_update;
                }
                aUnzipTimer.AutoReset = false; // 设置为 false，以便只触发一次。
                aUnzipTimer.Enabled = true;
            });
        }

        private void timer_unzip_and_check_update(object? sender, System.Timers.ElapsedEventArgs e)
        {
            
            //解压游戏
            UnZip();
            //检测游戏是否存在，以及是否需要升级
            checkGameExeExists();
        }

        private void Downloader_DownloadProgressChanged(object? sender, DownloadProgressChangedEventArgs e)
        {
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                double dReceivedMB = e.ReceivedBytesSize / 1024 / 1024;
                double dTotalMB = e.TotalBytesToReceive / 1024 / 1024;
                double dResidueBytes = e.TotalBytesToReceive - e.ReceivedBytesSize;
                double dResidueSeconds = dResidueBytes / e.AverageBytesPerSecondSpeed;

                string sspeed = "";
                double dSpeedKBPerSecond = e.AverageBytesPerSecondSpeed / 1024;
                double dSpeedMBPerSecond = dSpeedKBPerSecond / 1024;

                if (Math.Floor(dSpeedMBPerSecond) > 0)
                {
                    sspeed = m_tool.double2string(dSpeedMBPerSecond) + "MB";
                }
                else
                {
                    sspeed = m_tool.double2string(dSpeedKBPerSecond) + "KB";
                }

                double dResidueMinutes = dResidueSeconds / 60;
                double dResidueHours = dResidueMinutes / 60;

                string sResidueTimes = "";
                if(Math.Floor(dResidueHours) > 0)
                {
                    sResidueTimes = m_tool.double2string(dResidueHours)+"小时";
                }
                else if(Math.Floor(dResidueMinutes) > 0)
                {
                    sResidueTimes = m_tool.double2string(dResidueMinutes)+"分钟";
                }
                else
                {
                    sResidueTimes = m_tool.double2string(dResidueSeconds)+"秒";
                }

                string s = String.Format("已下载{0}MB/{1}MB,下载速度{2}/秒,下载进度:{3}%,剩余时间:{4}",m_tool.double2string(dReceivedMB),m_tool.double2string(dTotalMB), sspeed, m_tool.double2string(e.ProgressPercentage),sResidueTimes);
                labInfo.Content = s;

                m_progress.Value = e.ProgressPercentage;
            });

        }

        private void Downloader_ChunkDownloadProgressChanged(object? sender, DownloadProgressChangedEventArgs e)
        {
          
        }

        private void Downloader_DownloadStarted(object? sender, DownloadStartedEventArgs e)
        {
            this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate
            {
                btnStartOrUpdate.Content = "取消安装";
            });
        }

        //检测游戏game.exe是否存在，是否需要升级，并设置开始游戏按钮的标题
        private void checkGameExeExists()
        {
            try
            {
                string sGameExe = m_cfg.GameExePath;
                if(File.Exists(sGameExe)==false) {

                    btnStartOrUpdate.Content = "开始安装";

                }
                else
               {
                    // Connect Patch Server
                    TryConnectPatchServer();
                    btnStartOrUpdate.Content = m_sConnServerTip;
                    //需要升级
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        //当前是否已登录

        private bool hasLogin()
        {
            if (m_cfg.Account == null || m_cfg.Account.Trim().Length == 0 || m_cfg.Password == null || m_cfg.Password.Trim().Length == 0)
            {
                return false;
            }

            //调用厂商API校验帐号和密码是否有效
            return m_tool.verifyLogin(m_cfg.Account, m_cfg.Password);
        }
     
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            m_progress.Visibility = Visibility.Hidden;
            m_z7.Extracting += M_z7_Extracting;

            checkGameExeExists();

            webBg.EnsureCoreWebView2Async();
            webBg.CoreWebView2InitializationCompleted += WebBg_CoreWebView2InitializationCompleted;
            webBg.NavigationCompleted += WebBg_NavigationCompleted;



        }

    

        private void WebBg_CoreWebView2InitializationCompleted(object sender, Microsoft.Web.WebView2.Core.CoreWebView2InitializationCompletedEventArgs e)
        {
            if (e.IsSuccess)
            {
               webBg.DefaultBackgroundColor = System.Drawing.Color.Transparent;
               webBg.Source = new Uri(m_cfg.AccountVerifyUrl);
               
            }
        }

        private async void web_setButtonText(string sText)
        {
            await webBg.EnsureCoreWebView2Async(null); // 确保WebView2已加载

            try
            {
                //拼接js code
                string js = String.Format("return set_btn_text({0});", sText);
                // 执行JavaScript代码并获取结果
                string result = await webBg.ExecuteScriptAsync(js);
                // 处理结果
                MessageBox.Show(result);

                if (result.Trim().Length >= 1 && result.Trim()[0] == '1')
                {

                }
            }
            catch (Exception ex)
            {
                // 处理异常
                MessageBox.Show(ex.Message);
            }
        }
        private async void web_login()
        {
            await webBg.EnsureCoreWebView2Async(null); // 确保WebView2已加载

            try
            {
                //拼接js code
                string js = String.Format("return login({0},{1});",m_cfg.Account,m_cfg.Password);
                // 执行JavaScript代码并获取结果
                string result = await webBg.ExecuteScriptAsync(js);
                // 处理结果
                MessageBox.Show(result);

                if(result.Trim().Length>=1 && result.Trim()[0]=='1')
                {
                   
                }
            }
            catch (Exception ex)
            {
                // 处理异常
                MessageBox.Show(ex.Message);
            }
        }

        private void WebBg_NavigationCompleted(object sender, Microsoft.Web.WebView2.Core.CoreWebView2NavigationCompletedEventArgs e)
        {
            if (e.IsSuccess)
            {
                webBg.CoreWebView2.AddHostObjectToScript("host", new HostObject(this)); //向网页注册回调函数

                web_login();
            }
        }

        private void btnTest_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                m_slocalZipFile = "C:\\1003B\\1003BXBB.exe";

                //解压缩下载后的文件，到游戏安装目录 
                // 创建一个间隔为两秒的定时器。
                if (null == aUnzipTimer)
                {
                    aUnzipTimer = new System.Timers.Timer(2000);
                    // 钩住定时器的 Elapsed 事件。
                    aUnzipTimer.Elapsed += timer_unzip_and_check_update;
                }
                aUnzipTimer.AutoReset = false; // 设置为 false，以便只触发一次。
                aUnzipTimer.Enabled = true;
            }
            catch(Exception ex)
            {
                m_tool.showExceptionMsgBox(ex);
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            m_z7.Cancel();
            FTPWorker.UpdateVersion(false);
            if (aPatchTimer != null) aPatchTimer.Stop();
        }

        private void killGameExe()
        {
            try
            {
                // 查找名为 game.exe 的所有进程
                Process[] processes = Process.GetProcessesByName("game");

                // 杀死这些进程
                foreach (Process process in processes)
                {
                    process.Kill();

                }
            }
            catch (Exception ex)
            {
                m_tool.showExceptionMsgBox(ex);
            }
        }

        private void Window_MouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            // 实现窗口拖动
            this.DragMove();
        }
    }
}