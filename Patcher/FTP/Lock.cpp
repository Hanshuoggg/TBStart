
#include "../pch.h"

#include "Lock.h"

//////////////////////////////////////////////////////////////////////////
/////CLockLog/////////////////////////////////////////////////////////////////////

CLock::CLock()
{
	::InitializeCriticalSection(&m_csLock);
}

CLock::~CLock()
{
	::DeleteCriticalSection(&m_csLock);
}

void CLock::Lock()
{
	::EnterCriticalSection(&m_csLock);
}

void CLock::Unlock()
{
	::LeaveCriticalSection(&m_csLock);
}

/////CLockLog/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
////CLockLog//////////////////////////////////////////////////////////////////////


CLockData<std::map<CLockLog *, std::string> > *CLockLog::m_pLockSet = NULL;

CLockLog::CLockLog(const std::string& strLockDesc)
	: m_bInLock(false)
{
	if (NULL == m_pLockSet)
	{
		m_pLockSet = new CLockData<std::map<CLockLog *, std::string> >;
	}

	ChangeLockDesc(strLockDesc);
}

CLockLog::~CLockLog()
{
	bool bDeleteLockSet = false;
	m_pLockSet->Lock();

	m_pLockSet->Get().erase(this);
	if (m_pLockSet->Get().empty())
	{
		bDeleteLockSet = true;
	}

	m_pLockSet->Unlock();
	//이 아래도 락이 필요한데..
	if (bDeleteLockSet)
	{
		delete m_pLockSet;
		m_pLockSet = NULL;
	}
}

void CLockLog::Lock()
{
	CLock::Lock();

	m_bInLock = true;
}

void CLockLog::Unlock()
{
	m_bInLock = false;

	CLock::Unlock();
}

std::string CLockLog::GetFullLockStateReport()
{
	std::string strReport("\n[ CLockLog ]\n");
	m_pLockSet->Lock();

	std::map<CLockLog *, std::string>::iterator iterLock = m_pLockSet->Get().begin();
	std::map<CLockLog *, std::string>::iterator iterLockEnd = m_pLockSet->Get().end();
	for (; iterLock != iterLockEnd; ++iterLock)
	{
		strReport += iterLock->second;
		strReport += " : ";
		std::string strState("Off");
		if (iterLock->first->m_bInLock)
		{
			strState = "On";
		}
		strReport += strState;
		strReport += "\n";
	}

	m_pLockSet->Unlock();
	return strReport;
}

void CLockLog::ChangeLockDesc(const std::string& strLockDesc)
{
	m_pLockSet->Lock();

	m_pLockSet->Get()[this] = strLockDesc;

	m_pLockSet->Unlock();
}


////CLockLog//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//////CMutexLock////////////////////////////////////////////////////////////////////


CMutexLock::CMutexLock(const std::string& strMutexKey)
	: m_bGetMutex(false)
{
	m_hMutex = CreateMutexA(NULL, FALSE, strMutexKey.c_str());

}
CMutexLock::~CMutexLock()
{
	if (IsLock())
	{
		Unlock();
	}
	
	CloseHandle(m_hMutex);
	m_hMutex = NULL;
}

void CMutexLock::Lock()
{
	WaitForSingleObject(m_hMutex, INFINITE);
	m_bGetMutex = true;
}

void CMutexLock::Unlock()
{
	m_bGetMutex = false;
	ReleaseMutex(m_hMutex);
}

bool CMutexLock::IsLock()
{
	return m_bGetMutex;
}


///////CMutexLock///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
