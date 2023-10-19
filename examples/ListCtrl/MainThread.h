#pragma once

#include "resource.h"

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

#include <memory>

//线程ID
enum ThreadId
{
	kThreadUI,
	kThreadWorker
};

/** 工作线程
*/
class WorkerThread : public nbase::FrameworkThread
{
public:
	WorkerThread(ThreadId threadID, const char* name)
		: FrameworkThread(name)
		, m_threadID(threadID) {}

	~WorkerThread(void) {}

private:
	/**
	* 虚函数，初始化线程
	* @return void	无返回值
	*/
	virtual void Init() override;

	/**
	* 虚函数，线程退出时，做一些清理工作
	* @return void	无返回值
	*/
	virtual void Cleanup() override;

private:

	/** 线程ID
	*/
	ThreadId m_threadID;
};

/** 主线程
*/
class MainThread : public nbase::FrameworkThread
{
public:
	MainThread() : nbase::FrameworkThread("MainThread") {}
	virtual ~MainThread() {}

private:
	/**
	* 虚函数，初始化主线程
	* @return void	无返回值
	*/
	virtual void Init() override;

	/**
	* 虚函数，主线程退出时，做一些清理工作
	* @return void	无返回值
	*/
	virtual void Cleanup() override;

private:
	/** 工作线程
	*/
	std::unique_ptr<WorkerThread> m_workerThread;
};


