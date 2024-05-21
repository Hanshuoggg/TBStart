using Microsoft.Web.WebView2.Wpf;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TSBStart
{
    public class JSFunc
    {
        Tools m_tool=new Tools();
        private WebView2 m_wv = null;
        public JSFunc(WebView2 wv) {
        
            m_wv = wv;
        }


        //通知网页当前游戏环境状态 0未就绪， 1就绪
        public async void set_game_ready(int iready)
        {
            await m_wv.EnsureCoreWebView2Async(); // 确保WebView2已加载

            try
            {
                //拼接js code
                string js = String.Format("return set_game_ready({0});", iready);
                // 执行JavaScript代码并获取结果
                string result = await m_wv.ExecuteScriptAsync(js);
             
            }
            catch (Exception ex)
            {
                // 处理异常
               m_tool.showExceptionMsgBox(ex);
            }
        }

        //设置网页中按钮状态,0 禁用，1启用
        public async void set_btn_enable(int ienable)
        {
            await m_wv.EnsureCoreWebView2Async(); // 确保WebView2已加载

            try
            {
                //拼接js code
                string js = String.Format("return set_btn_enable({0});", ienable);
                // 执行JavaScript代码并获取结果
                string result = await m_wv.ExecuteScriptAsync(js);
                
            }
            catch (Exception ex)
            {
                // 处理异常
                m_tool.showExceptionMsgBox(ex);
            }
        }

        //设置进度文本信息
        public async void set_progress_info(string stip)
        {
            await m_wv.EnsureCoreWebView2Async(); // 确保WebView2已加载

            try
            {
                //拼接js code
                string js = String.Format("return set_progress_info({0});", stip);
                // 执行JavaScript代码并获取结果
                string result = await m_wv.ExecuteScriptAsync(js);

            }
            catch (Exception ex)
            {
                // 处理异常
                m_tool.showExceptionMsgBox(ex);
            }
        }

        //设置当前进度 0 -100
        public async void set_progress_num(int inum)
        {
            await m_wv.EnsureCoreWebView2Async(); // 确保WebView2已加载

            try
            {
                //拼接js code
                string js = String.Format("return set_progress_num({0});", inum);
                // 执行JavaScript代码并获取结果
                string result = await m_wv.ExecuteScriptAsync(js);

            }
            catch (Exception ex)
            {
                // 处理异常
                m_tool.showExceptionMsgBox(ex);
            }
        }



    }
}
