#include "Framework.h"
#include "Time.h"

using namespace std;

bool Time::isTimerStopped = true;
float Time::timeElapsed = 0.0f;

Time::Time(void) :
	ticksPerSecond(0), currentTime(0), lastTime(0), lastFPSUpdate(0), fpsUpdateInterval(0),
	frameCount(0), runningTime(0), framePerSecond(0)
{
	
}

Time::~Time(void)
{

}

Time* Time::Get()
{
	static Time instance;
	return &instance;
}

void Time::Initialize()
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond);
	fpsUpdateInterval = ticksPerSecond >> 1;

	/*TwBar* bar = TweakBar::Get()->GetBar();
	TwAddVarRO(bar, "Time", TW_TYPE_FLOAT, &framePerSecond, "");*/
}

void Time::Destroy()
{
}

void Time::Update()
{
	if (isTimerStopped) return;

	//1. 현재시간을 가져와 시간 간격 및 진행 시간을 계산한다.
	QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
	timeElapsed = (float)(currentTime - lastTime) / (float)ticksPerSecond;
	runningTime += timeElapsed;


	//2. FPS Update
	frameCount++;
	if (currentTime - lastFPSUpdate >= fpsUpdateInterval)
	{
		float tempCurrentTime = (float)currentTime / (float)ticksPerSecond;
		float tempLastTime = (float)lastFPSUpdate / (float)ticksPerSecond;
		framePerSecond = (float)frameCount / (tempCurrentTime - tempLastTime);

		lastFPSUpdate = (INT64)currentTime;
		frameCount = 0;
	}

	lastTime = currentTime;
}

void Time::Render()
{
	using Vector3 = D3DXVECTOR3;

	string str = string("FrameRate : ") + to_string(ImGui::GetIO().Framerate);
	Gui::Get()->RenderText(5, 5, 1, 1, 1, str);

	Vector3 R;
	Camera::Get()->RotationDegree(&R);

	Vector3 P;
	Camera::Get()->Position(&P);

	str = "Camera Rotation : ";
	str += to_string((int)R.x) + ", " + to_string((int)R.y) + ", " + to_string((int)R.z);
	Gui::Get()->RenderText(5, 20, 1, 1, 1, str);

	str = "Camera Position : ";
	str += to_string((int)P.x) + ", " + to_string((int)P.y) + ", " + to_string((int)P.z);
	Gui::Get()->RenderText(5, 35, 1, 1, 1, str);
}

void Time::Print()
{

}

void Time::Start()
{
	if (!isTimerStopped)
		assert(false);

	QueryPerformanceCounter((LARGE_INTEGER *)&lastTime);
	isTimerStopped = false;
}

void Time::Stop()
{
	if (isTimerStopped)
		assert(false);

	INT64 stopTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER *)&stopTime);
	runningTime += (float)(stopTime - lastTime) / (float)ticksPerSecond;
	isTimerStopped = true;
}

///////////////////////////////////////////////////////////////////////////////

Timer::Timer()
{
	bComplete = false;

	count = 0;
}

Timer::~Timer()
{

}

void Timer::Start(function<void()> func, int milliSec, UINT repeat)
{
	assert(bComplete == false);

	bComplete = false;
	thread t([=]()
	{
		while (true)
		{
			if (repeat > 0 && count == repeat)
				break;

			if (bComplete == true)
				break;

			count++;
			Sleep(milliSec);

			if (bComplete == true)
				break;

			func();
		}

		Stop();
	});
	t.detach();
}

void Timer::Stop()
{
	count = 0;

	bComplete = true;
}

///////////////////////////////////////////////////////////////////////////////

Performance::Performance()
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&tick);
}

void Performance::Start()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&start);
}

float Performance::End()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&end);

	return (float)((double)(end - start) / tick * 1000.0f);
}
