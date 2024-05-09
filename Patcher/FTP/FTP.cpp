
#include "../pch.h"
#include "FTP.h"
#include <Mmsystem.h>
#include <stdlib.h>

#pragma comment(lib, "Winmm")

using namespace System;
using namespace System::Windows::Forms;

namespace NFTP2
{
	CFTP::CFTP()
		: m_bThreadLoop(false)
		, hINET(NULL)
		, hFTP(NULL)
		, hThread(NULL)
		, m_dDownStartTime(0.0)
		, m_llTotalSize(0)
		, m_llDownSize(0)
		, m_lDownCount(0)
		, m_iSpeedCheckRange(30)
		, m_bIsConnect(false)
	{
	}

	CFTP::~CFTP()
	{
		ThreadExit();
		Disconnect();
	}

	bool CFTP::Init(std::shared_ptr<CFTP_Listener> sptrListener)
	{
		m_sptrListener = sptrListener;
		return true;
	}

	bool CFTP::Connect(const std::string& strServer, int iPort, const std::string& strID, const std::string& strPass)
	{
		if (m_bIsConnect) return true;

		m_strServer = strServer;
		m_iPort = iPort;
		m_strID = strID;
		m_strPass = strPass;

		hINET = InternetOpenW(L"1003b Auto Patch", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hINET == NULL)
		{
			MessageBox::Show(L"Internet Connection Failure", L"Notice", MessageBoxButtons::OK, MessageBoxIcon::Error);
			return false;
		}

		hFTP = InternetConnectA(hINET, strServer.c_str(), iPort, NULL, NULL, INTERNET_SERVICE_HTTP, INTERNET_FLAG_PASSIVE, 0);

		if (hFTP == NULL)
		{
			ErrorCheck();
			MessageBox::Show(L"FTP Connection Failure", L"Notice", MessageBoxButtons::OK, MessageBoxIcon::Error);
			InternetCloseHandle(hINET);
			return false;
		}

		m_bIsConnect = true;
		return true;
	}

	void CFTP::Disconnect(bool bThreadOff)
	{
		m_bIsConnect = false;

		if (bThreadOff)
		{
			ThreadExit();
		}

		InternetCloseHandle(hFTP);
		InternetCloseHandle(hINET);
		hFTP = NULL;
		hINET = NULL;
	}

	bool CFTP::RecvAdd(const std::string& strRootPath, const std::string& strLocalDownPath)
	{
		auto sptrDownData = std::make_shared<CFTP_DownData>(strRootPath, strLocalDownPath);

		m_dequeDownList.Lock();
		m_dequeDownList.Get().push_back(sptrDownData);
		m_dequeDownList.Unlock();

		return true;
	}

	bool CFTP::RecvBlock(const std::string& strRootPath, const std::string& strLocalDownPath, unsigned long long& OUT llSize)
	{
		CFTP_DownData ftpDown(strRootPath, strLocalDownPath);

		if (!ftpDown.OpenFile(hFTP))
		{
			return false;
		}

		unsigned long long llDownSize = 0;

		try
		{
			llDownSize = GetRootFileSize(strRootPath);
		}
		catch (E_Error e)
		{
			if (e == E_E_NoFile)
			{
				return false;
			}
			throw e;
		}

		llSize = llDownSize;

		do
		{
			llDownSize -= ftpDown.RecvPart();
		} while (!ftpDown.IsRecvDone());

		ftpDown.CloseFile();

		if (llDownSize != 0)
		{
			return false;
		}

		return true;
	}

	std::shared_ptr<CPartWork> CFTP::RecvPartStart(const std::string& strRootPath, const std::string& strLocalDownPath)
	{
		std::shared_ptr<CPartWork> partWork(new CPartWork(strRootPath, strLocalDownPath));

		if (!partWork->_OpenFile(hFTP))
		{
			return std::shared_ptr<CPartWork>();
		}

		return partWork;
	}

	bool CFTP::StartWork()
	{
		m_bThreadLoop = true;

		//시작시간 설정
		m_dDownStartTime = timeGetTime() / 1000.0;
		m_dDownLastTime.clear();
		m_dDownLastTime.assign(m_iSpeedCheckRange, 0);// = m_dDownStartTime;
		m_llTotalSize = 0;
		m_llDownSize = 0;
		m_llLastSize.clear();
		m_llLastSize.assign(m_iSpeedCheckRange, 0);
		m_lDownCount = 0;

		unsigned long dwThreadID;
		// thread를 생성.
		hThread = (HANDLE)_beginthreadex(NULL, 0, &CFTP::ThreadWork, this, 0, (unsigned*)&dwThreadID);
		if (NULL == hThread)
		{
			throw E_E_ThreadCreateFalse;
		}

		return false;
	}

	std::shared_ptr<CFTP_DownData> CFTP::GetNowWork()
	{
		m_sptrDownWork.reset();

		m_dequeDownList.Lock();
		if (m_dequeDownList.Get().empty())
		{
			m_dequeDownList.Unlock();
			return std::shared_ptr<CFTP_DownData>();
		}

		auto iterNow = std::find_if(m_dequeDownList.Get().begin(), m_dequeDownList.Get().end(), [](std::shared_ptr<CFTP_DownData> sptrDownWork) {
			return sptrDownWork->GetFileTotalSize() > sptrDownWork->GetRecvSize();
		});

		if (m_dequeDownList.Get().end() == iterNow)
		{
			//다돌았다.
			m_dequeDownList.Unlock();
			return std::shared_ptr<CFTP_DownData>();
		}

		m_sptrDownWork = *iterNow;

		m_dequeDownList.Unlock();

		return m_sptrDownWork;
	}

	unsigned CFTP::ThreadWork_Core()
	{
		m_dDownStartTime = timeGetTime() / 1000.0;
		m_dDownLastTime.clear();
		m_dDownLastTime.assign(m_iSpeedCheckRange, 0);// = m_dDownStartTime;

#ifdef DEF_Display
		g_DisplayText.Lock();
		g_DisplayText.Get() = "Wait ..";
		g_DisplayText.Unlock();
#endif // DEF_NoDisplay

		if (false == GetTotalSize())
		{
			// FTP에 없는 파일(불량)을 가져오려 했는경우.?
#ifdef DEF_Display
			g_DisplayText.Lock();
			g_DisplayText.Get().clear();
			g_DisplayText.Unlock();
			Environment::Exit(0);
#endif // DEF_Display
			m_bThreadLoop = false;
			WorkDone();
			_endthreadex(0);
			return 0;
		}

#ifdef DEF_Display
		g_DisplayText.Lock();
		g_DisplayText.Get().clear();
		g_DisplayText.Unlock();
#endif // DEF_Display

		if (m_bThreadLoop)
		{
			m_sptrListener->OnTotalSize(m_llTotalSize);
		}

		if (!m_sptrDownWork)
		{
			GetNowWork();
			if (!m_sptrDownWork)
			{
				// 파일이 없다.
				m_bThreadLoop = false;
				WorkDone();
				_endthreadex(0);
				return 0;
			}
		}

		while (m_bThreadLoop)
		{
			// 차근차근 리시브

			// 작업중인놈을 받고

			if (!m_sptrDownWork->OpenFile(hFTP))
			{
				m_sptrListener->OnError(E_E_FileOpen, (void*)&m_sptrDownWork->GetLocalPath());
#ifdef DEF_Display
				String^ rootPathManaged = gcnew String(m_sptrDownWork->GetLocalPath().c_str());
				String^ message = "Save Error\n" + rootPathManaged;
				MessageBox::Show(message, "File", MessageBoxButtons::OK, MessageBoxIcon::Error);
#endif // DEF_Display
				m_bThreadLoop = false;
				break;
			}

			m_llDownSize += m_sptrDownWork->RecvPart();

			// 덜받았다면 다음
			if (!m_sptrDownWork->IsRecvDone())
			{
				continue;
			}

			// 다받았다면 그다음놈을 설정하고
			m_sptrDownWork->CloseFile();
			m_lDownCount++;

			GetNowWork();

			// 더이상 받을게 있나 검사
			if (m_sptrDownWork)
			{
				// 더이상 받을게 있다면
				continue;
			}

			// 더이상 받을게 없다면
			break;
		}

		// 강제 종료 검사 필요

		if (m_sptrDownWork)
		{
			// 더받아야하는데 루프를 탈출한 경우라면 외부에서 종료한것.
			// 닫지 않은 파일 닫기
			m_sptrDownWork->CloseFile();
		}

		WorkDone();

		_endthreadex(0);
		return 0;
	}

	CFTP::CDownInfo CFTP::GetDownloadInfo()
	{
		CFTP::CDownInfo downInfo;
		downInfo.m_lDownCount = m_lDownCount;
		downInfo.m_llDownSize = m_llDownSize;

		double dNowTime = timeGetTime() / 1000.0;
		double dTotalTime = dNowTime - m_dDownStartTime;
		downInfo.m_lTime = (unsigned long)dTotalTime;

		m_llLastSize.pop_front();
		m_llLastSize.push_back(m_llDownSize);
		long long llDeltaSize = m_llLastSize.back() - m_llLastSize.front();

		m_dDownLastTime.pop_front();
		m_dDownLastTime.push_back(dNowTime);
		double dDeltaTime = m_dDownLastTime.back() - m_dDownLastTime.front();

		double dDeltaSpeed = llDeltaSize / max(0.001, dDeltaTime);
		downInfo.m_lDownSpeed = (unsigned long)((m_llDownSize / dTotalTime) / 1024.0);

		double dRemainTime = (m_llTotalSize - m_llDownSize) / (float)max(1.0, dDeltaSpeed);
		downInfo.m_lRemainTime = (unsigned long)dRemainTime;

		return downInfo;
	}

	unsigned CFTP::ThreadWork(void* vpThis)
	{
		if (NULL == vpThis)
		{
			_endthreadex(-1);
			return -1;
		}

		CFTP* pThis = (CFTP*)vpThis;

		return pThis->ThreadWork_Core();
	}

	void CFTP::WorkDone()
	{
		m_sptrDownWork.reset();

		m_dequeDownList.Lock();
		m_dequeDownList.Get().clear();
		m_dequeDownList.Unlock();

		if (m_bThreadLoop)
		{
			m_sptrListener->OnRecvDone();
		}
	}

	void CFTP::ThreadExit()
	{
		if (hThread)
		{
			m_bThreadLoop = false;

			WaitForSingleObject(hThread, INFINITE);

			CloseHandle(hThread);

			hThread = NULL;
		}
	}

	bool CFTP::GetTotalSize()
	{
		int iCount = 0;

		m_dequeDownList.Lock();
		auto dequeDownList = m_dequeDownList.Get();
		std::for_each(dequeDownList.begin(), dequeDownList.end(), [this, &iCount](std::shared_ptr<CFTP_DownData> sptrDownData) {

#ifdef DEF_Display
			iCount++;
			int iLimitCount = (iCount % 10) + 1;
			g_DisplayText.Lock();
			g_DisplayText.Get() = "Wait ";
			for (int ixPoint = 0; ixPoint < iLimitCount; ++ixPoint)
			{
				g_DisplayText.Get() += ".";
			}
			g_DisplayText.Unlock();
#endif // DEF_Display

			unsigned long long llSize = 0;
			try
			{
				llSize = GetRootFileSize(sptrDownData->GetRootPath());
			}
			catch (E_Error e)
			{
				if (e == E_E_NoFile)
				{
					m_sptrLastErrorFile = sptrDownData;
					String^ rootPathManaged = gcnew String(sptrDownData->GetRootPath().c_str());
					String^ message = "BadFile\n" + rootPathManaged;
					MessageBox::Show(message, "FtpConn", MessageBoxButtons::OK, MessageBoxIcon::Error);
					return;
				}
				throw e;
			}
			sptrDownData->SetFileTotalSize(llSize);
			m_llTotalSize += llSize;
		});


		m_dequeDownList.Unlock();

		return !((bool)m_sptrLastErrorFile);
	}

	bool CFTP::ErrorCheck()
	{
		unsigned long errora = GetLastError();
		if (errora == ERROR_INTERNET_CONNECTION_RESET)
		{
			return true;
		}
		else if (errora == ERROR_INTERNET_EXTENDED_ERROR)
		{
			char pszBuff[2048];
			DWORD dwBuffLen = 2048;
			// ERROR_INTERNET_TIMEOUT
			InternetGetLastResponseInfoA(&errora, pszBuff, &dwBuffLen);

			if (strstr(pszBuff, "350"))
			{ // 성공..

			}
			else
			{ // 실패..

			}
		}

		if (IsDebuggerPresent())
		{
			//DebugBreak();
		}

		throw E_E_NoFile;
	}

	unsigned long long CFTP::GetRootFileSize(const std::string& strFTPPath)
	{
		HINTERNET hRemote = HttpOpenRequestA(hFTP, "HEAD", strFTPPath.c_str(), "HTTP / 1.1", nullptr, nullptr, INTERNET_FLAG_RELOAD, 0);
		if (!hRemote)
		{
			if (ErrorCheck())
			{
				throw E_E_NoFile;
			}
			throw std::runtime_error("HTTP 요청을 열 수 없습니다.");
		}

		auto bSuccess = HttpSendRequest(hRemote, nullptr, 0, nullptr, 0);
		if (FALSE == bSuccess)
		{
			if (ErrorCheck())
			{
				InternetCloseHandle(hRemote);
				throw E_E_HttpSendRequest;
			}
		}

		unsigned long dwSize = 0;
		BOOL lResult = InternetQueryDataAvailable(hRemote, &dwSize, 0, 0);
		if (lResult != TRUE)
		{
			if (ErrorCheck())
			{
				InternetCloseHandle(hRemote);
				throw E_E_NoFile;
			}
		}

		unsigned long long llReturnSize = dwSize;

		if (0 >= llReturnSize)
		{
			char chBuffer[128] = { 0 };
			DWORD dwBufLen = sizeof(chBuffer);
			lResult = ::HttpQueryInfoA(hRemote, HTTP_QUERY_CONTENT_LENGTH, chBuffer, &dwBufLen, NULL);
			if (lResult == TRUE)
			{
				sscanf_s(chBuffer, "%lld", &llReturnSize);

				if (llReturnSize == 166)
				{
					InternetCloseHandle(hRemote);
					throw E_E_NoFile;
				}
			}
			else
			{
				if (ErrorCheck())
				{
					InternetCloseHandle(hRemote);
					throw E_E_NoFile;
				}
			}
		}
		return llReturnSize;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	CFTP_DownData::CFTP_DownData(const std::string& strRootPath, const std::string& strLocalPath, unsigned long long llTotalSize)
		: m_strRootPath(strRootPath)
		, m_strLocalPath(strLocalPath)
		, m_llTotalSize(llTotalSize)
		, m_llRecvSize(0)
		, m_bIsOpen(false)
		, hPatchList(nullptr)
	{

	}

	CFTP_DownData::~CFTP_DownData()
	{
		CloseFile();
	}

	bool CFTP_DownData::OpenFile(HINTERNET hFTP)
	{
		if (m_bIsOpen)
		{
			// 이미 열어놨다면 다음
			return true;
		}

		// 파일 열기
#ifdef DEF_HTTP
		hPatchList = HttpOpenRequestA(hFTP, "GET", m_strRootPath.c_str(), "HTTP / 1.1", nullptr, nullptr, INTERNET_FLAG_RELOAD, 0);
#else // DEF_HTTP
		hPatchList = FtpOpenFileA(hFTP, m_strRootPath.c_str(), GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 0);
#endif // DEF_HTTP
		if (NULL == hPatchList)
		{
			CFTP::ErrorCheck();
			return false;
		}

#ifdef DEF_HTTP
		auto bSuccess = HttpSendRequest(hPatchList, nullptr, 0, nullptr, 0);
		if (FALSE == bSuccess)
		{
			if (CFTP::ErrorCheck())
			{

				throw CFTP::E_E_HttpSendRequest;
			}
		}
#endif // DEF_HTTP

		m_ofsLocal.open(m_strLocalPath, std::ios_base::binary);

		if (!m_ofsLocal.good())
		{
			unsigned long error = GetLastError();
			m_ofsLocal.close();
			return false;
		}

		m_bIsOpen = true;
		return true;
	}

	unsigned long CFTP_DownData::RecvPart()
	{
		if (!m_bIsOpen)
		{
			return 0;
		}

		unsigned long dwRecvSize = 0;
		unsigned long dwRecvSizeReq = 0;

#ifdef DEF_HTTP
		BOOL lResult = InternetQueryDataAvailable(hPatchList, &dwRecvSize, 0, 0);
		if (lResult != TRUE)
		{
			if (CFTP::ErrorCheck())
			{
				return 0;
			}
		}
		char* chDownBuffer = new char[dwRecvSize];
		dwRecvSizeReq = dwRecvSize;
#else // DEF_HTTP
		char chDownBuffer[E_D_DownBufferSize];
		dwRecvSizeReq = E_D_DownBufferSize;
#endif // DEF_HTTP

		InternetReadFile(hPatchList, chDownBuffer, dwRecvSizeReq, &dwRecvSize);
		m_ofsLocal.write(chDownBuffer, dwRecvSize);

#ifdef DEF_HTTP
		delete[] chDownBuffer;
#endif // DEF_HTTP

		m_llRecvSize += dwRecvSize;

		return dwRecvSize;
	}

	bool CFTP_DownData::IsRecvDone()
	{
		return m_llRecvSize >= m_llTotalSize;
	}

	void CFTP_DownData::CloseFile()
	{
		if (m_bIsOpen)
		{
			m_ofsLocal.close();
			InternetCloseHandle(hPatchList);
			hPatchList = nullptr;
			m_bIsOpen = false;
		}
	}

	void CFTP_DownData::SetFileTotalSize(unsigned long long llSize)
	{
		m_llTotalSize = llSize;
	}

	//////////////////////////////////////////////////////////////////////////
	/////CPartWork/////////////////////////////////////////////////////////////////////


	CPartWork::CPartWork(const std::string& strRootPath, const std::string& strLocalPath)
		: CFTP_DownData(strRootPath, strLocalPath)
	{

	}

	CPartWork::~CPartWork()
	{
	}

	bool CPartWork::IsRecvDone()
	{
		auto bIsDone = CFTP_DownData::IsRecvDone();

		if (bIsDone)
		{
			CloseFile();
		}
		return bIsDone;
	}

	bool CPartWork::_OpenFile(HINTERNET hFTP)
	{
		return CFTP_DownData::OpenFile(hFTP);
	}

	////////CPartWork//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
}
