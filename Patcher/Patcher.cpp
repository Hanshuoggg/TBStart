#include "pch.h"

#include "Patcher.h"

#include <iostream>
#include <string.h>
#include <Windows.h>

std::string g_strParameter;
PatchStep iDownload_Step = PatchStep::eSTEP_GET_SERVER_VERSION;
bool g_bAutoLogin = false;
bool bGame_Is_Run = false;

#include <io.h>

char DOWNLOAD_VERSION_URL[MAX_PATH] = "pub/1003b/Patch/Version/version.dat";  //¹öÀüÆÄÀÏÀÇ À§Ä¡
char DOWNLOAD_GAME_EXE_URL[MAX_PATH] = "pub/1003b/Patch/Patch_Data/Patch_0.3300/game.exe"; //°ÔÀÓ½ÇÇàÆÄÀÏÀÇ À§Ä¡
char DOWNLOAD_PATCHER_URL[MAX_PATH] = "pub/1003b/Patch/Patch_Data/Patch_0.3300/1003b.exe"; //ÆÐÄ¡ ÇÁ·Î±×·¥ÀÇ À§Ä¡
char DOWNLOAD_FILEENGINE_URL[MAX_PATH] = "pub/1003b/Patch/Patch_Data/Patch_0.3300/FileEngineR.dll"; //ÆÄÀÏ¿£ÁøÀÇ À§Ä¡
char DOWNLOAD_PATCH_VERSION_LIST_URL[MAX_PATH] = "pub/1003b/Patch/Patch_List/index_list.txt"; //¹öÀü¸®½ºÆ®ÀÇ À§Ä¡
char DOWNLOAD_PATCH_URL[MAX_PATH] = "pub/1003b/Patch/Patch_List/Patch_%s.txt"; //ÆÐÄ¡ÆÄÀÏ¸®½ºÆ®ÀÇ À§Ä¡ 
char DOWNLOAD_URL[MAX_PATH] = "pub/1003b/Patch/Patch_Data/Patch_0.3300/"; //ÆÐÄ¡ÆÄÀÏ ´Ù¿î·ÎµåÀÇ À§Ä¡

char LVerStr[16] = { 0, };

#define MIN_FILE_NAME_LENGTH 4

//////////////////////////////////////////////////////////////////////////
#include <atlconv.h>
#include <list>

using namespace std;
using namespace System;
using namespace System::Windows::Forms;

WCHAR acCurrent_Folder[MAX_PATH];
vector<string> kLocal_Patch_FileList_FullPath;

vector<string> kLocal_Delete_File_List;

list<string> kDownload_Patch_File_List;

typedef list<string> LIST_STR;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static NFTP2::CFTP g_ftpWork;

std::shared_ptr<CFTPWorkListener> g_sptrFTPIstener(new CFTPWorkListener(CFTPWorkListener::E_DownStep::E_DS_GetList));

unsigned long long g_llTotalSize = 0;

void Get_Current_Exe_Dir(const WCHAR* pcDir)
{
	wcsncpy_s(acCurrent_Folder, MAX_PATH, pcDir, _TRUNCATE);
	
	// ÇöÀç µð·ºÅÍ¸® ¼³Á¤
	::SetCurrentDirectory(acCurrent_Folder);

	// ´Ù¿î·Îµå Æú´õ »ý¼º (¹Ýµå½Ã À¯´ÏÄÚµå ¹®ÀÚ¿­À» Ã³¸®ÇÒ ¼ö ÀÖ°Ô CreateDirectoryW¸¦ »ç¿ëÇØ¾ß ÇÔ)
	CreateDirectoryW(DOWNLOAD_FOLDERNAME, NULL);

	return;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Connect_PatchServer()
{
	kLocal_Delete_File_List.clear();
	g_ftpWork.Init(g_sptrFTPIstener);
	if (g_ftpWork.Connect(DEF_FTPAddress, DEF_FTPPort, "", "") == false)
	{
		MessageBox::Show(L"Internet Connection Failure", L"Notice", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Environment::Exit(0);
	}
}

void Disconnet_PatchServer()
{
	g_ftpWork.Disconnect();
	Clear_Works();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Clear_Works()
{
	for (unsigned int i = 0; i < kLocal_Delete_File_List.size(); i++)
	{
		remove(kLocal_Delete_File_List[i].c_str());
		//index_list.txt -> del
		//Patch_0.7506.txt -> del
	}
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Load_INI(WCHAR* pcFileName)
{
	USES_CONVERSION;
	int iExist_INI = _access(W2A(pcFileName), 0);

	if (iExist_INI == 0)
	{
		WCHAR pcTemp[255] = { 0, };

		GetPrivateProfileString(L"VERSION", L"URL", L"pub/1003b/Patch/Version/version.dat", pcTemp, MAX_PATH, pcFileName);			//DOWNLOAD_VERSION_URL
		strcpy_s(DOWNLOAD_VERSION_URL, W2A(pcTemp));

		GetPrivateProfileString(L"EXECUTE", L"URL", L"pub/1003b/Patch/Patch_Data/Patch_0.3300/game.exe", pcTemp, MAX_PATH, pcFileName);			//DOWNLOAD_GAME_EXE_URL
		strcpy_s(DOWNLOAD_GAME_EXE_URL, W2A(pcTemp));

		GetPrivateProfileString(L"FILEENGINE", L"URL", L"pub/1003b/Patch/Patch_Data/Patch_0.3300/FileEngineR.dll", pcTemp, MAX_PATH, pcFileName);			//DOWNLOAD_FILEENGINE_URL
		strcpy_s(DOWNLOAD_FILEENGINE_URL, W2A(pcTemp));

		GetPrivateProfileString(L"PATCHER", L"URL", L"pub/1003b/Patch/Patch_Data/Patch_0.3300/1003b.exe", pcTemp, MAX_PATH, pcFileName);			//DOWNLOAD_PATCHER_URL
		strcpy_s(DOWNLOAD_PATCHER_URL, W2A(pcTemp));

		GetPrivateProfileString(L"INDEXLIST", L"URL", L"pub/1003b/Patch/Patch_List/index_list.txt", pcTemp, MAX_PATH, pcFileName); //DOWNLOAD_PATCH_VERSION_LIST_URL
		strcpy_s(DOWNLOAD_PATCH_VERSION_LIST_URL, W2A(pcTemp));

		GetPrivateProfileString(L"PATCHLIST", L"URL", L"pub/1003b/Patch/Patch_List/Patch_%s.txt", pcTemp, MAX_PATH, pcFileName);				//DOWNLOAD_PATCH_URL
		strcpy_s(DOWNLOAD_PATCH_URL, W2A(pcTemp));

		GetPrivateProfileString(L"DOWNLOAD", L"URL", L"pub/1003b/Patch/Patch_Data/Patch_0.3300/", pcTemp, MAX_PATH, pcFileName);					//DOWNLOAD_PATCH_URL
		strcpy_s(DOWNLOAD_URL, W2A(pcTemp));

		int idebug = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Trim_String(string pkStr, char* pcToken)
{
	pkStr.erase(0, pkStr.find_first_not_of(pcToken));
	pkStr.erase(pkStr.find_last_not_of(pcToken) + 1);

	for (unsigned int i = 0; i < pkStr.size(); i++)
	{
		if (pkStr[i] == *pcToken)
		{
			pkStr.erase(pkStr.find(pcToken), 1);
		}
	}
}

bool FileExists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Update_Version(bool bUpdate_Success)
{
	USES_CONVERSION;

	if (bUpdate_Success)
	{
		if (FileExists(W2A(VERSION_REMOTE_FILENAME)))
		{
			remove(W2A(LOCAL_VERSION_FILENAME)); ////version.dat -> del
			rename(W2A(VERSION_REMOTE_FILENAME), W2A(LOCAL_VERSION_FILENAME)); ////version_R.dat -> version.dat
		}
	}
	else
	{
		remove(W2A(VERSION_REMOTE_FILENAME)); ////version_R.dat -> del
	}
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
int Compare_Version()
{
	Sleep(10);

	memset(LVerStr, 0, sizeof(LVerStr));

	char RVerStr[16] = { 0, };

	char ac_LocalVersion[256] = { 0, };
	char ac_RemoteVersion[256] = { 0, };

	DWORD dwRead = 0;
	DWORD fileSize = 0;

	HANDLE hLocal_Version_Handle;
	HANDLE hRemote_Version_Handle;

	hLocal_Version_Handle = CreateFile(L"version.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hLocal_Version_Handle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	fileSize = GetFileSize(hLocal_Version_Handle, NULL);

	BOOL bResult;

	if (fileSize < 8)
	{
		bResult = ReadFile(hLocal_Version_Handle, ac_LocalVersion, 256, &dwRead, NULL);
		if (!bResult)
		{
			CloseHandle(hLocal_Version_Handle);
			return 0;
		}

		for (DWORD i = 1; i < dwRead; i++)
		{
			if (i == 2)
			{
				sprintf_s(LVerStr, "%s%c%d", LVerStr, '.', ac_LocalVersion[i]);
			}
			else
			{
				sprintf_s(LVerStr, "%s%d", LVerStr, ac_LocalVersion[i]);
			}
		}
		CloseHandle(hLocal_Version_Handle);
	}
	else if (fileSize > 10)
	{
		bResult = ReadFile(hLocal_Version_Handle, ac_LocalVersion, 256, &dwRead, NULL);
		if (!bResult)
		{
			CloseHandle(hLocal_Version_Handle);
			return 0;
		}

		for (int i = 1; i < 6; i++)
		{
			if (i == 2)
			{
				sprintf_s(LVerStr, "%s%c%d", LVerStr, '.', ac_LocalVersion[i]);
			}
			else
			{
				sprintf_s(LVerStr, "%s%d", LVerStr, ac_LocalVersion[i]);
			}
		}
		CloseHandle(hLocal_Version_Handle);
	}


	hRemote_Version_Handle = CreateFile(L"version_R.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hRemote_Version_Handle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	fileSize = GetFileSize(hRemote_Version_Handle, NULL);

	if (fileSize < 8)
	{
		bResult = ReadFile(hRemote_Version_Handle, ac_RemoteVersion, 256, &dwRead, NULL);
		if (!bResult)
		{
			CloseHandle(hRemote_Version_Handle);
			return 0;
		}

		for (DWORD i = 1; i < dwRead; i++)
		{
			if (i == 2)
			{
				sprintf_s(RVerStr, "%s%c%d", RVerStr, '.', ac_RemoteVersion[i]);
			}
			else
			{
				sprintf_s(RVerStr, "%s%d", RVerStr, ac_RemoteVersion[i]);
			}
		}
		CloseHandle(hRemote_Version_Handle);
	}
	else if (fileSize > 10)
	{
		bResult = ReadFile(hRemote_Version_Handle, ac_RemoteVersion, 256, &dwRead, NULL);
		if (!bResult)
		{
			CloseHandle(hRemote_Version_Handle);
			return 0;
		}

		for (int i = 1; i < 6; i++)
		{
			if (i == 2)
			{
				sprintf_s(RVerStr, "%s%c%d", RVerStr, '.', ac_RemoteVersion[i]);
			}
			else
			{
				sprintf_s(RVerStr, "%s%d", RVerStr, ac_RemoteVersion[i]);
			}
		}
		CloseHandle(hRemote_Version_Handle);
	}



	if (strcmp(LVerStr, RVerStr) == 0) //¹öÀüÀÌ °°À¸¸é...
	{
		return 1;
	}

	return 0; // ¹öÀüÀÌ Æ²¸²...
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Step_Patch_Done()
{
	g_ftpWork.ThreadExit(); //ÀÌ»óÀÌ ¾øÀ¸¸é ½º·¹µå¸¦ Á¾·áÇÏ°í ½ÇÇàÇÑ´Ù.
	g_ftpWork.Disconnect();
	Clear_Works();
	Update_Version(true);
//	Excute_Game();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Step_Get_Patch_Version_List()
{
	g_ftpWork.ThreadExit();

	wstring strLocalPath;
	strLocalPath = wstring(acCurrent_Folder) + wstring(L"\\") + wstring(VERSION_LIST_INDEX_FILENAME);
	USES_CONVERSION;
	CHAR* pcTempStr_1 = W2A(strLocalPath.c_str());
	g_ftpWork.RecvAdd(DOWNLOAD_PATCH_VERSION_LIST_URL, string(pcTempStr_1));
	kLocal_Delete_File_List.push_back(pcTempStr_1);

	if (bGame_Is_Run)
	{
		strLocalPath = wstring(acCurrent_Folder) + wstring(L"\\") + wstring((DOWNLOAD_FOLDERNAME)) + wstring(L"\\") + wstring(GAME_EXE_PATCH_FILENAME);
	}
	else
	{
		strLocalPath = wstring(acCurrent_Folder) + wstring(L"\\") + wstring(GAME_EXE_FILENAME);
	}

	CHAR* pcTempStr_2 = W2A(strLocalPath.c_str());
	g_ftpWork.RecvAdd(string(DOWNLOAD_GAME_EXE_URL), string(pcTempStr_2));

	strLocalPath = wstring(acCurrent_Folder) + wstring(L"\\") + wstring((DOWNLOAD_FOLDERNAME)) + wstring(L"\\") + wstring(PATCHER_REMOTE_FILENAME);
	CHAR* pcTempStr_3 = W2A(strLocalPath.c_str());
	g_ftpWork.RecvAdd(string(DOWNLOAD_PATCHER_URL), string(pcTempStr_3));

	strLocalPath = wstring(acCurrent_Folder) + wstring(L"\\") + wstring((DOWNLOAD_FOLDERNAME)) + wstring(L"\\") + wstring(FILEENGINE_PATCH_FILENAME);
	CHAR* pcTempStr_4 = W2A(strLocalPath.c_str());
	g_ftpWork.RecvAdd(string(DOWNLOAD_FILEENGINE_URL), string(pcTempStr_4));

	g_ftpWork.StartWork();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Check_Exist_PrePatch()
{
	WIN32_FIND_DATA fd;
	HANDLE hFile = FindFirstFile(L"download\\*.*", &fd);

	while (hFile != INVALID_HANDLE_VALUE)
	{
		TCHAR sPatchFilePath[MAX_PATH] = { 0, };
		TCHAR sTargetFilePath[MAX_PATH] = { 0, };

		bool bFind = false; //1Àº ÀÏ¹Ý ÆÄÀÏ 2´Â ¾ÐÃà ÆÄÀÏ - ÇÏÁö¸¸ 1¹Û¿¡ ¾È¾¸.

		//download Æú´õ¿¡ ÀÖ´Â ÆÄÀÏÀÌ¸§À» º¹»ç.
		TCHAR sPatchFileName[MAX_PATH] = { 0, };
		_tcscpy_s(sPatchFileName, fd.cFileName);

		size_t nLength = _tcslen(sPatchFileName);
		const TCHAR cFileCommand = nLength > 1 ? sPatchFileName[nLength - 1] : 0;

		switch (tolower(cFileCommand))
		{
		case 'r': // Root À§Ä¡ - Client Image À§Ä¡.
		{
			wsprintf(sPatchFilePath, L"%s\\download\\%s", acCurrent_Folder, sPatchFileName);
			wsprintf(sTargetFilePath, L"%s\\%s", acCurrent_Folder, sPatchFileName);
			sTargetFilePath[_tcslen(sTargetFilePath) - 1] = 0;
			bFind = 1;
		}break;
		}
		if (bFind)
		{
			SetFileAttributes(sTargetFilePath, FILE_ATTRIBUTE_NORMAL);

			if (CopyFile(sPatchFilePath, sTargetFilePath, FALSE))
				DeleteFile(sPatchFilePath);
		}

		if (!FindNextFile(hFile, &fd))
			break;
	}
	FindClose(hFile);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
bool Check_Game_Running()
{
	HANDLE      hFile = INVALID_HANDLE_VALUE;

	WCHAR acFullPath[MAX_PATH]; // ÀüÃ¼ °æ·Î ¹× ÆÄÀÏ ÀÌ¸§À» ÀúÀåÇÕ´Ï´Ù.

	// ÀüÃ¼ °æ·Î »ý¼º
	swprintf(acFullPath, MAX_PATH, L"%s\\%s", acCurrent_Folder, GAME_EXE_FILENAME);
	
	hFile = CreateFileW(acFullPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return true;
	}

	CloseHandle(hFile);

	return false;


}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Step_Get_ServerVersion()
{
	wstring strLocalPath; //¹öÀüÁ¤º¸¿Í ÇÊ¼öÆÄÀÏµéÀ» ´Ù¿î·ÎµåÇÑ´Ù.

	USES_CONVERSION;

	strLocalPath = wstring(acCurrent_Folder) + wstring(L"\\") + wstring(VERSION_REMOTE_FILENAME);
	CHAR* pcTempStr_1 = W2A(strLocalPath.c_str());
	g_ftpWork.RecvAdd(string(DOWNLOAD_VERSION_URL), string(pcTempStr_1));

	g_ftpWork.StartWork();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Step_Get_File_List()
{
	g_ftpWork.ThreadExit();
	kLocal_Patch_FileList_FullPath.clear();

	vector<string> kVerList_String;

	USES_CONVERSION;
	std::ifstream fin;
	fin.open(W2A(VERSION_LIST_INDEX_FILENAME), ifstream::in); //¹öÀü¸®½ºÆ®¸¦ ´Ù¿î¹Þ¾Æ ÆÄ½Ì, ÆÄÀÏ¸®½ºÆ®¸¦ ¸¸µç´Ù. 

	if (fin.is_open())
	{
		char acTempLine[255] = { 0, };
		while (fin.good())
		{
			fin.getline(acTempLine, 255);

			Trim_String(string(acTempLine), " ");

			if (strlen(acTempLine) > MIN_FILE_NAME_LENGTH)
				kVerList_String.push_back(acTempLine);

		}
		fin.close();
	}
	else
	{
		MessageBox::Show(L"Can't find version Index file at Server.", L"Notice", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Environment::Exit(0);

		return;
	}

	string kCuffent_Ver_Str = string(LVerStr);

	bool bFind_CurrentVersion = false;

	for (unsigned int i = 1; i < kVerList_String.size(); i++)
	{
		if (kCuffent_Ver_Str == kVerList_String[i]) //¹öÀüÀÌ °°À¸¸é...
		{
			bFind_CurrentVersion = true;
			break;
		}
	}

	for (unsigned int i = 1; i < kVerList_String.size(); i++)
	{
		char acPatch_Version_Filename_Remote[MAX_PATH] = { 0, };
		char acPatch_Version_Filename_Local[MAX_PATH] = { 0, };
		WCHAR acPatch_Filename[MAX_PATH] = { 0, };
		USES_CONVERSION;

		if (kCuffent_Ver_Str < kVerList_String[i] || bFind_CurrentVersion == false)
		{
			wsprintf(acPatch_Filename, L"Patch_%s.txt", A2W(kVerList_String[i].c_str()));

			wstring kTempLocalPath = acCurrent_Folder + wstring(L"\\") + acPatch_Filename;

			CHAR* pcTempStr = W2A(kTempLocalPath.c_str());

			kLocal_Patch_FileList_FullPath.push_back(string(pcTempStr));
			kLocal_Delete_File_List.push_back(string(pcTempStr));

			sprintf_s(acPatch_Version_Filename_Remote, DOWNLOAD_PATCH_URL, kVerList_String[i].c_str());
			g_ftpWork.RecvAdd(string(acPatch_Version_Filename_Remote), string(pcTempStr));
		}
	}
	g_ftpWork.StartWork();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Step_Get_Patch_Files()
{
	g_ftpWork.ThreadExit(); //ÆÄÀÏ¸®½ºÆ®¸¦ ÆÄ½ÌÇØ ´Ù¿î·Îµå ¹Þ´Â´Ù. 
	kDownload_Patch_File_List.clear();

	int iBuffer_Count = 0;

	size_t iSize = kLocal_Patch_FileList_FullPath.size();

	for (size_t i = 0; i < iSize; i++)
	{
		std::ifstream fin;
		fin.open(kLocal_Patch_FileList_FullPath[i].c_str(), ifstream::in);
		if (fin.is_open())
		{
			char acTempLine[255] = { 0, };
			while (fin.good())
			{
				fin.getline(acTempLine, 255);
				Trim_String(string(acTempLine), " ");

				if (strlen(acTempLine) > MIN_FILE_NAME_LENGTH)
				{
					LIST_STR::iterator FOUND_Begin = find(kDownload_Patch_File_List.begin(), kDownload_Patch_File_List.end(), string(acTempLine));
					LIST_STR::iterator FOUND_End = kDownload_Patch_File_List.end();

					if (FOUND_Begin == FOUND_End)
						kDownload_Patch_File_List.push_back(acTempLine);
				}

			}
			fin.close();
		}
		else
		{
			MessageBox::Show(L"Can't find version file at Server.", L"Notice", MessageBoxButtons::OK, MessageBoxIcon::Error);
			Environment::Exit(0);
			return;
		}
	}

	for (auto i = kDownload_Patch_File_List.begin(); i != kDownload_Patch_File_List.end(); i++)
	{
		char acDownload_Data_FileName_Remote[255] = { 0, };

		USES_CONVERSION;
		if (strcmp(W2A(GAME_EXE_FILENAME), i->c_str()) == 0 || strcmp(W2A(PATCHER_FILENAME), i->c_str()) == 0 || strcmp(W2A(FILEENGINE_FILENAME), i->c_str()) == 0)
			continue;

		sprintf_s(acDownload_Data_FileName_Remote, "%s\\%s", DOWNLOAD_URL, i->c_str());
		wstring kDownload_Data_FileName_Local = acCurrent_Folder + wstring(L"\\") + (DOWNLOAD_FOLDERNAME) + wstring(L"\\") + A2W(i->c_str());
		CHAR* pcTempStr = W2A(kDownload_Data_FileName_Local.c_str());

		g_ftpWork.RecvAdd(string(acDownload_Data_FileName_Remote), string(pcTempStr));
	}

	if (kDownload_Patch_File_List.size() > 0)
	{
		g_ftpWork.StartWork();
	}
	else
	{
		iDownload_Step = PatchStep::eSTEP_PATCH_DONE;
		Patch_Start(iDownload_Step);
	}
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Patch_Start(int iStep)
{
	switch (iStep)
	{
	case PatchStep::eSTEP_GET_SERVER_VERSION:
	{
		Step_Get_ServerVersion();
	}
	break;

	case PatchStep::eSTEP_GET_PATCH_VERSION_LIST:
	{
		Step_Get_Patch_Version_List();
	}
	break;

	case PatchStep::eSTEP_GET_FILE_LIST:
	{
		Step_Get_File_List();
	}
	break;

	case PatchStep::eSTEP_GET_PATCH_FILES:
	{
		Step_Get_Patch_Files();
	}
	break;

	case PatchStep::eSTEP_PATCH_DONE:
	{
		Step_Patch_Done();
	}
	break;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
NFTP2::CFTP::CDownInfo GetDownloadInfo()
{
	return g_ftpWork.GetDownloadInfo();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
CFTPWorkListener::CFTPWorkListener(E_DownStep eStep)
	: m_eStep(eStep)
{

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void CFTPWorkListener::OnRecvDone()
{
	HWND hWnd = FindPatchWindow();

	if (hWnd != nullptr) {
		// »ç¿ëÀÚ Á¤ÀÇ ¸Þ½ÃÁö º¸³»±â
		// WM_USER + 1À» »ç¿ëÀÚ ÁöÁ¤ ¸Þ½ÃÁö·Î »ç¿ë
		// WPARAM°ú LPARAM¿¡´Â Ãß°¡ Á¤º¸¸¦ Àü´ÞÇÒ ¼ö ÀÖÀ¸¸ç, ÀÌ ¿¹Á¦¿¡¼­´Â ´Ü¼øÈ­¸¦ À§ÇØ 0À» »ç¿ë
		switch (m_eStep)
		{
		case E_DownStep::E_DS_GetList:
			PostMessage(hWnd, WM_USER_GetListDone, 0, 0);
			break;
		case E_DownStep::E_DS_GetFull:
			PostMessage(hWnd, WM_USER_GetFTPDone, 0, 0);
			break;
		default:
			break;
		}
	}
}

void CFTPWorkListener::OnTotalSize(unsigned long long llTotalSize)
{
	g_llTotalSize = llTotalSize;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void CFTPWorkListener::OnError(NFTP2::CFTP::E_Error eError, const void* vpErrorData)
{
	if (eError == NFTP2::CFTP::E_Error::E_E_FileOpen) {
		std::string* strErrorData = static_cast<std::string*>(const_cast<void*>(vpErrorData));
		String^ errorMessage = gcnew String(("Save Error\n" + *strErrorData).c_str());
		MessageBox::Show(errorMessage, "File", MessageBoxButtons::OK, MessageBoxIcon::Error);

		HWND hWnd = FindPatchWindow();
		if (hWnd != nullptr) {
			PostMessage(hWnd, WM_USER_ErrorQuit, 0, 0);
		}
	}
}

HWND CFTPWorkListener::FindPatchWindow() const
{
	HWND hWnd = FindWindow(nullptr, TEXT("ÐÂÌìÉÏ±®µÇÂ¼Æ÷"));

	if (hWnd == nullptr) {
		MessageBox::Show(TEXT("Cannot find the target window."), TEXT("Error"), MessageBoxButtons::OK, MessageBoxIcon::Error);
	}

	return hWnd;
}

///////CFTPWorkListener///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////