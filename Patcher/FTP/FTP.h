#pragma once

#include "Lock.h"

namespace NFTP2
{
	// ������ ������
	class CFTP_DownData
	{
	public:
		enum E_Define
		{
			E_D_DownBufferSize = 4096 * 4, // �ٿ�޴� ���� ������
		};
	public:
		CFTP_DownData(const std::string& strRootPath, const std::string& strLocalPath, unsigned long long llTotalSize = 0);
		~CFTP_DownData(); // ���� �Լ� �ƴ�

		bool OpenFile(HINTERNET hFTP);
		unsigned long RecvPart(); // �������� �޴´�. // �ٿ� ���� ������
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
		HINTERNET hPatchList = nullptr; // ftp �ٿ��
		std::ofstream m_ofsLocal; // ���� �����
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
			E_E_NoFile, // ������ ����.
			E_E_ThreadCreateFalse, // ������ ���� X
			E_E_NotWork, // �۾��� ������ ����
			E_E_NotDev, // ���� �ޱ� ����� ���� �������� �ʽ��ϴ�.
			E_E_FileWriteFalse, // �ٿ���� ������ �����Ҽ�����
			E_E_FileOpen, //!< ���� ���� ����
			E_E_HttpSendRequest, //!< HttpSendRequest() Error
		};

		// �������� �������� ������
		class CDownInfo
		{
		public:
			unsigned long m_lDownCount; // ������ ������
			unsigned long m_lDownSpeed; // �ٿ� �ӵ� (����Ʈ/��)
			unsigned long long m_llDownSize; //  �ٿ� ���� ��
			unsigned long m_lRemainTime; // �����ð�(����)(��)
			unsigned long m_lTime; // �����ð�(��)

		};

	public:
		CFTP();
		virtual ~CFTP();

		bool Init(std::shared_ptr<CFTP_Listener> sptrListener);

		bool Connect(const std::string& strServer, int iPort, const std::string& strID, const std::string& strPass);
		void Disconnect(bool bThreadOff = true);

		bool RecvAdd(const std::string& strRootPath, const std::string& strLocalDownPath); // �񵿱� �ٿ� ����Ʈ �߰�
		bool RecvBlock(const std::string& strRootPath, const std::string& strLocalDownPath, unsigned long long& OUT llSize); // ����ٿ�.
		std::shared_ptr<CPartWork> RecvPartStart(const std::string& strRootPath, const std::string& strLocalDownPath); // �Ϻξ� �ٿ�޴� ���

		bool StartWork(); // �񵿱� �ٿ� ���� ������ ����
		void ThreadExit(); // ������ ���Ḧ ��ٸ�.
		void WorkDone(); //  ������ �۾��� �����ٰ� �˸�

		static unsigned __stdcall ThreadWork(void* vpThis); // ������ ��ŷ�����忡�� ȣ���� �ּ���. // ������ ���� �����ؼ� ��߰ٴ�.
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

		double m_dDownStartTime; // �ٿ�ε� ���۽ð�
		std::deque<double> m_dDownLastTime; // �ӵ� �˻��� ������ �ð�
		long long m_llTotalSize; // �ٿ���� ��Ż ������
		long long m_llDownSize; // �ٿ���� ������
		std::deque<long long> m_llLastSize; // �ӵ��˻��� ������ �ٿ���� ������
		long m_lDownCount; //  �ٿ���� ����
		int m_iSpeedCheckRange; // �ӵ� �˻��Ҷ� �ֱ� ������� ������ ������˻��Ұ���.

		CLockData<std::deque<std::shared_ptr<CFTP_DownData>>> m_dequeDownList;

		std::shared_ptr<CFTP_DownData> m_sptrLastErrorFile; // �������� ��
		std::shared_ptr<CFTP_DownData> m_sptrDownWork; // �۾����� ��

		bool m_bIsConnect; // �������̴�.

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