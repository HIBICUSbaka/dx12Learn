//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

// 可以对细粒度的事件同样实例化作为一个定时器
// 例如到达五秒后出发某某事件
class GameTimer
{
public:
	GameTimer();

	float TotalTime()const; // in seconds
	float DeltaTime()const; // in seconds

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

private:
	double mSecondsPerCount;
	double mDeltaTime;		// 帧间差值(以秒为单位)

	__int64 mBaseTime;		// 游戏程序开始时刻时间计数
	__int64 mPausedTime;	// 所有暂停时间计数之和
	__int64 mStopTime;		// 游戏程序暂停时刻时间计数
	__int64 mPrevTime;		// 上一帧时间计数
	__int64 mCurrTime;		// 本帧时间计数

	bool mStopped;			// 是否处于暂停状态
};

#endif // GAMETIMER_H