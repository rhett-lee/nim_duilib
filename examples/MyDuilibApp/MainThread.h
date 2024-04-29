#ifndef UI_MAINTHREAD_H_
#define UI_MAINTHREAD_H_

#pragma once

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

//线程ID
enum ThreadId
{
	kThreadUI,		//主线程ID
	kThreadWorker	//工作线程ID
};

/** 工作线程
*/
class WorkerThread : public nbase::FrameworkThread
{
public:
	WorkerThread(ThreadId threadID, const char* name);
	virtual ~WorkerThread();

private:
	/** 初始化线程
	*/
	virtual void Init() override;

	/** 线程退出时，做一些清理工作
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
	MainThread();
	virtual ~MainThread();

private:
	/** 初始化主线程
	*/
	virtual void Init() override;

	/** 主线程退出时，做一些清理工作
	*/
	virtual void Cleanup() override;

private:
	/** 工作线程(如果不需要多线程处理业务，可以移除工作线程的代码)
	*/
	std::unique_ptr<WorkerThread> m_workerThread;
};

#endif // UI_MAINTHREAD_H_
