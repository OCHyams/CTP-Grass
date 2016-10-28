#pragma once
#include <dinput.h>


class Input
{
public:
	Input();
	virtual ~Input();
	/////////////////////////////////////////////////
	/// initialise directx input
	/////////////////////////////////////////////////
	bool init(HINSTANCE, HWND);
	/////////////////////////////////////////////////
	/// return whether Key _key is currently pressed
	/////////////////////////////////////////////////
	inline bool getKey(unsigned char _key) const  { return (m_keys[_key] & 0x80); }
	/////////////////////////////////////////////////
	/// return whether Key _key has been pressed this frame
	/////////////////////////////////////////////////
	inline bool getKeyDown(unsigned char _key) const { return (m_keys[_key] & 0x80) && !(m_prevKeys[_key] & 0x80); }
	/////////////////////////////////////////////////
	/// update the keyboard state
	/////////////////////////////////////////////////
	void update();

private:
	LPDIRECTINPUT8 m_directInput;
	LPDIRECTINPUTDEVICE8 m_keyboardDevice;

	unsigned char m_keys[256];
	unsigned char m_prevKeys[256];
};