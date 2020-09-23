//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

// ���Զ�ϸ���ȵ��¼�ͬ��ʵ������Ϊһ����ʱ��
// ���絽����������ĳĳ�¼�
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
	double mDeltaTime;		// ֡���ֵ(����Ϊ��λ)

	__int64 mBaseTime;		// ��Ϸ����ʼʱ��ʱ�����
	__int64 mPausedTime;	// ������ͣʱ�����֮��
	__int64 mStopTime;		// ��Ϸ������ͣʱ��ʱ�����
	__int64 mPrevTime;		// ��һ֡ʱ�����
	__int64 mCurrTime;		// ��֡ʱ�����

	bool mStopped;			// �Ƿ�����ͣ״̬
};

#endif // GAMETIMER_H