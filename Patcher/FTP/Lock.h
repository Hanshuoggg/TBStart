//--------------------------------------------------------------------------------------
// FileName		: Lock.h
// Description	: ��
//--------------------------------------------------------------------------------------

#pragma once

#include <map>

//////////////////////////////////////////////////////////////////////////
/////CLock/////////////////////////////////////////////////////////////////////

//�⺻��
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

//lock�� ����ϴ� ������
//TypeData�� �⺻ �����ڸ� ����Ѵ�
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
	bool m_bLockOff; //����׿� ������ Ȯ��
#endif // _DEBUG
	CLock m_lock;
	TypeData m_typeData;
};

/////CLockData/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
/////CLockLog/////////////////////////////////////////////////////////////////////

//���� �αױ�� �߰�
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

//���� �αױ�� �߰���
//lock�� ����ϴ� ������
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
	bool m_bLockOff; //����׿� ������ Ȯ��
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

	HANDLE m_hMutex; //! ���ؽ� �ڵ�
	bool m_bGetMutex; //! �ڽ��� ���ؽ��� ��������
};


///////CMutexLock///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
