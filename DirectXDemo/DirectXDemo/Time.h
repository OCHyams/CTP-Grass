#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct Time
{
	double time;
	double deltaTime;
}; 

//https://books.google.co.uk/books?id=GY-AAwAAQBAJ&pg=PA8&lpg=PA8&dq=directx+time&source=bl&ots=q9YPQklXDB&sig=xGWZLvpnItZcbmYaVLumhup2Oh4&hl=en&sa=X&ved=0ahUKEwjskq_qgIXQAhXDJsAKHa9HAJ0Q6AEILjAD#v=onepage&q=time&f=false
//Real-Time 3D Rendering with DirectX and HLSL
class Clock
{
public:	

	Clock();

	const LARGE_INTEGER& getStart() const;
	const LARGE_INTEGER& getLast() const;
	const LARGE_INTEGER& getCurrent() const;
	double getFrequency() const;

	void reset();
	void getTime(LARGE_INTEGER& time) const;
	void updateGameTime(Time&);

private:

	LARGE_INTEGER m_start;
	LARGE_INTEGER m_last;
	LARGE_INTEGER m_current;
	double m_frequency;
};