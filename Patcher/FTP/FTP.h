#pragma once

#include "Lock.h"

namespace NFTP2
{
	// 관리용 데이터
	class CFTP_DownData
	{
	public:
		enum E_Define
		{
			E_D_DownBufferSize = 4096 * 4, // 다운받는 버퍼 사이즈
		};
	public:
		CFTP_DownData(const std::string& strRootPath, const std::string& strLocalPath, unsigned long long llTotalSize = 0);
		~CFTP_DownData(); // 가상 함수 아님

		bool OpenFile(HINTERNET hFTP);
		unsigned long RecvPart(); // 한조각을 받는다. // 다운 받은 사이즈
		void CloseFile();
		bool IsRecvDone();

		unsigned long long GetFileTotalSize() const { return m_llTotalSize; }
		unsigned long long GetRecvSize() const { return m_llRecvSize; }

		void SetFileTotalSize(unsigned long long llSize);

		const std::string& GetLocalPath() const { return m_strLocalPath; }
		const std::string& GetRootPath() const { return m_strRootPath; }

	private:
		std::string m_strRootPath;
		std::string m_strLocalPath;
		unsigned long long m_llTotalSize = 0;
		unsigned long long m_llRecvSize = 0;
		bool m_bIsOpen = false;
		HINTERNET hPatchList = nullptr; // ftp 다운용
		std::ofstream m_ofsLocal; // 로컬 저장용
	};

	class CPartWork : protected CFTP_DownData
	{
	public:
		CPartWork(const std::string& strRootPath, const std::string& strLocalPath);
		virtual ~CPartWork();

		bool IsRecvDone();
		bool _OpenFile(HINTERNET hFTP);

	protected:
	};

	class CFTP_Listener;

	class CFTP
	{
	public:
		enum E_Error
		{
			E_E_NoFile, // 파일이 없다.
			E_E_ThreadCreateFalse, // 쓰래드 생성 X
			E_E_NotWork, // 작업할 파일이 없다
			E_E_NotDev, // 폴더 받기 기능은 아직 지원하지 않습니다.
			E_E_FileWriteFalse, // 다운받은 파일을 저장할수없다
			E_E_FileOpen, //!< 파일 생성 실패
			E_E_HttpSendRequest, //!< HttpSendRequest() Error
		};

		// 유저에게 보여지는 데이터
		class CDownInfo
		{
		public:
			unsigned long m_lDownCount; // 성공한 받은수
			unsigned long m_lDownSpeed; // 다운 속도 (바이트/초)
			unsigned long long m_llDownSize; //  다운 받은 양
			unsigned long m_lRemainTime; // 남은시간(예상)(초)
			unsigned long m_lTime; // 지난시간(초)

		};

	public:
		CFTP();
		virtual ~CFTP();

		bool Init(std::shared_ptr<CFTP_Listener> sptrListener);

		bool Connect(const std::string& strServer, int iPort, const std::string& strID, const std::string& strPass);
		void Disconnect(bool bThreadOff = true);

		bool RecvAdd(const std::string& strRootPath, const std::string& strLocalDownPath); // 비동기 다운 리스트 추가
		bool RecvBlock(const std::string& strRootPath, const std::string& strLocalDownPath, unsigned long long& OUT llSize); // 동기다운.
		std::shared_ptr<CPartWork> RecvPartStart(const std::string& strRootPath, const std::string& strLocalDownPath); // 일부씩 다운받는 방법

		bool StartWork(); // 비동기 다운 시작 스래드 생성
		void ThreadExit(); // 스래드 종료를 기다림.
		void WorkDone(); //  스래드 작업이 끝났다고 알림

		static unsigned __stdcall ThreadWork(void* vpThis); // 적당한 워킹스레드에서 호줄해 주세요. // 스래드 직접 생성해서 써야겟다.
		unsigned ThreadWork_Core();

		CDownInfo GetDownloadInfo();

		static bool ErrorCheck();

		unsigned long long GetRootFileSize(const std::string& strFTPPath);

	protected:

		bool GetTotalSize();


		std::shared_ptr<CFTP_DownData> GetNowWork();

	protected:
		std::shared_ptr<CFTP_Listener> m_sptrListener;

		HINTERNET hINET;
		HINTERNET hFTP;

		HANDLE hThread;
		bool m_bThreadLoop;

		double m_dDownStartTime; // 다운로드 시작시간
		std::deque<double> m_dDownLastTime; // 속도 검사한 마지막 시간
		long long m_llTotalSize; // 다운받을 토탈 사이즈
		long long m_llDownSize; // 다운받은 사이즈
		std::deque<long long> m_llLastSize; // 속도검사할 마지막 다운받은 사이즈
		long m_lDownCount; //  다운받은 개수
		int m_iSpeedCheckRange; // 속도 검사할때 최근 몇개까지의 정보를 가지고검사할건지.

		CLockData<std::deque<std::shared_ptr<CFTP_DownData>>> m_dequeDownList;

		std::shared_ptr<CFTP_DownData> m_sptrLastErrorFile; // 에러나는 놈
		std::shared_ptr<CFTP_DownData> m_sptrDownWork; // 작업중인 놈

		bool m_bIsConnect; // 접속중이다.

		std::string m_strServer;
		int m_iPort;
		std::string m_strID;
		std::string m_strPass;
	};

	class CFTP_Listener
	{
	public:
		CFTP_Listener() {};
		virtual ~CFTP_Listener() {};

		virtual void OnRecvDone() = 0;
		virtual void OnTotalSize(unsigned long long llTotalSize) = 0;

		virtual void OnError(CFTP::E_Error eError, const void* vpErrorData) = 0;
	};
}