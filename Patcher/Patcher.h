#pragma once

#include "FTP/FTP.h"

#define DEF_FTPAddress "dl2.1003b.56a.com"
#define DEF_FTPPort 80

#define DOWNLOAD_FOLDERNAME			L"download"

#define VERSION_REMOTE_FILENAME	L"version_R.dat"
#define LOCAL_VERSION_FILENAME	L"version.dat"

#define PATCHER_REMOTE_FILENAME	L"1003b.exer"
#define PATCHER_FILENAME  L"1003b.exe"
#define GAME_EXE_PATCH_FILENAME	L"game.exer"
#define GAME_EXE_FILENAME	L"game.exe"

#define FILEENGINE_FILENAME L"FileEngineR.dll"
#define FILEENGINE_PATCH_FILENAME L"FileEngineR.dllr"
#define VERSION_LIST_INDEX_FILENAME L"index_list.txt"
#define  PATCH_FILELIST_NAME L"Patch_%s.txt"

class CFTPWorkListener : public NFTP2::CFTP_Listener
{
public:
	enum class E_DownStep
	{
		E_DS_GetList,
		E_DS_GetFull,
	};

	CFTPWorkListener(E_DownStep eStep);

	virtual void OnRecvDone() override;
	virtual void OnTotalSize(unsigned long long llTotalSize) override;
	virtual void OnError(NFTP2::CFTP::E_Error eError, const void* vpErrorData) override;
	HWND FindPatchWindow() const;

private:
	E_DownStep m_eStep;
};

extern std::shared_ptr<CFTPWorkListener> g_sptrFTPIstener;
extern unsigned long long g_llTotalSize;

void Patch_Start(int iStep);
void Connect_PatchServer();
void Disconnet_PatchServer();
int Compare_Version();
void Clear_Works();
void Update_Version(bool bUpdate_Success);
void Check_Exist_PrePatch();
bool Check_Game_Running();

void Load_INI(WCHAR* pcFileName);
void Step_Get_ServerVersion();
void Step_Get_Patch_Version_List();
void Step_Get_File_List();
void Step_Get_Patch_Files();
void Step_Patch_Done();
void Get_Current_Exe_Dir(const WCHAR* pcDir);
NFTP2::CFTP::CDownInfo GetDownloadInfo();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Patcher {

	public ref class FTPWorker
	{
	public:
		static void PatchStart(int iStep)
		{
			Patch_Start(iStep);
		}

		static void ConnectPatchServer()
		{
			Connect_PatchServer();
		}

		static void DisconnetPatchServer()
		{
			Disconnet_PatchServer();
		}
		
		static int CompareVersion()
		{
			return Compare_Version();
		}

		static void ClearWorks()
		{
			Clear_Works();
		}

		static void UpdateVersion(bool bUpdate_Success)
		{
			Update_Version(bUpdate_Success);
		}

		static void CheckExistPrePatch()
		{
			Check_Exist_PrePatch();
		}

		static bool CheckGameRunning()
		{
			return Check_Game_Running();
		}

		static void LoadINI()
		{
			#define INI_NAME L".\\Patch_URL.ini"
			Load_INI(INI_NAME);
		}

		static void StepGetPatchFiles()
		{
			Step_Get_Patch_Files();
		}

		static void StepPatchDone()
		{
			Step_Patch_Done();
		}

		static void GetCurrentExeDir(System::String^ managedString) {
			// System::String^을 std::wstring으로 변환
			msclr::interop::marshal_context context;
			std::wstring unmanagedString = context.marshal_as<std::wstring>(managedString);

			// 변환된 std::wstring을 사용하여 기본 C++ 함수 호출
			Get_Current_Exe_Dir(unmanagedString.c_str());
		}

		static unsigned long long GetDownloadSize()
		{
			return GetDownloadInfo().m_llDownSize;
		}

		static unsigned long GetDownloadSpeed()
		{
			return GetDownloadInfo().m_lDownSpeed;
		}

		static unsigned long long GetTotalSize()
		{
			return g_llTotalSize;
		}
	};
}
