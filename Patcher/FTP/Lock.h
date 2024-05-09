//--------------------------------------------------------------------------------------
// FileName		: Lock.h
// Description	: 락
//--------------------------------------------------------------------------------------

#pragma once

#include <map>

//////////////////////////////////////////////////////////////////////////
/////CLock/////////////////////////////////////////////////////////////////////

//기본락
class CLock
{
public:
	CLock();
	virtual ~CLock();
	void Lock();
	void Unlock();

private:
	CRITICAL_SECTION m_csLock;

};

/////CLock/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/////CLockData/////////////////////////////////////////////////////////////////////

//lock을 사용하는 데이터
//TypeData의 기본 생성자를 사용한다
template<typename TypeData>
class CLockData
{
public:
	CLockData()
#ifdef _DEBUG
		: m_bLockOff(true)
#endif // _DEBUG
	{

	}

	virtual ~CLockData()
	{

	}

	void Lock()
	{
		m_lock.Lock();
#ifdef _DEBUG
		m_bLockOff = false;
#endif // _DEBUG
	}

	TypeData &Get()
	{
#ifdef _DEBUG
		if (m_bLockOff)
		{
			DebugBreak();
		}
#endif // _DEBUG
		return m_typeData;
	}

	void Unlock()
	{
#ifdef _DEBUG
		m_bLockOff = true;
#endif // _DEBUG
		m_lock.Unlock();
	}

private:
#ifdef _DEBUG
	bool m_bLockOff; //디버그용 락상태 확인
#endif // _DEBUG
	CLock m_lock;
	TypeData m_typeData;
};

/////CLockData/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
/////CLockLog/////////////////////////////////////////////////////////////////////

//상태 로그기능 추가
class CLockLog: public CLock
{
public:
	CLockLog(const std::string &strLockDesc);
	virtual ~CLockLog();

	void Lock();
	void Unlock();

	static std::string GetFullLockStateReport();
	void ChangeLockDesc(const std::string &strLockDesc);

private:
	bool m_bInLock;
	static CLockData<std::map<CLockLog *, std::string> > *m_pLockSet;
};

/////CLockLog/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/////CLockDataLog/////////////////////////////////////////////////////////////////////

//상태 로그기능 추가한
//lock을 사용하는 데이터
template<typename TypeData>
class CLockDataLog
{
public:
	CLockDataLog(const std::string &strLockDesc)
		: m_pLock(strLockDesc)
#ifdef _DEBUG
		, m_bLockOff(true)
#endif // _DEBUG
	{
	}

	virtual ~CLockDataLog()
	{
	}

	void Lock()
	{

		m_pLock.Lock();
#ifdef _DEBUG
		m_bLockOff = false;
#endif // _DEBUG
	}

	TypeData &Get()
	{
#ifdef _DEBUG
		if (m_bLockOff)
		{
			DebugBreak();
		}
#endif // _DEBUG
		return m_typeData;
	}

	void Unlock()
	{
#ifdef _DEBUG
		m_bLockOff = true;
#endif // _DEBUG
		m_pLock.Unlock();
	}

private:
#ifdef _DEBUG
	bool m_bLockOff; //디버그용 락상태 확인
#endif // _DEBUG
	CLockLog m_pLock;
	TypeData m_typeData;
};

/////CLockDataLog/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////CMutexLock////////////////////////////////////////////////////////////////////

class CMutexLock
{
public:
	CMutexLock(const std::string& strMutexKey);
	virtual ~CMutexLock();

	void Lock();
	void Unlock();

	bool IsLock();

private:

	HANDLE m_hMutex; //! 뮤텍스 핸들
	bool m_bGetMutex; //! 자신이 뮤텍스를 가졌는지
};


///////CMutexLock///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
