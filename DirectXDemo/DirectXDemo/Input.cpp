#include "Input.h"
#include "dxerr.h"

Input::Input() 
{
}

Input::~Input()
{
	if (m_keyboardDevice)
	{
		m_keyboardDevice->Unacquire();
		m_keyboardDevice->Release();
	}
	if (m_directInput) m_directInput->Release();
}

bool Input::init(HINSTANCE _hInstance, HWND _hwnd)
{
	HRESULT result;
	ZeroMemory(&m_keys, sizeof(m_keys));
	ZeroMemory(&m_prevKeys, sizeof(m_prevKeys));

	result = DirectInput8Create(_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create direct input object.");
		return false;
	}
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboardDevice, NULL);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create keyboard object.");
		return false;
	}
	result = m_keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to set keyboard data format.");
		return false;
	}
	result = m_keyboardDevice->SetCooperativeLevel(_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to set keyboard cooperation level.");
		return false;
	}
	result = m_keyboardDevice->Acquire();
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to aquire keyboard.");
		return false;
	}
	return true;
}

void Input::update()
{
	memcpy(m_prevKeys, m_keys, sizeof(m_keys));
	//clear out previous state
	ZeroMemory(&m_keys, sizeof(m_keys));
	// Read the keyboard device.
	HRESULT hr = m_keyboardDevice->GetDeviceState(sizeof(m_keys), (LPVOID)&m_keys);
	if (FAILED(hr))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			m_keyboardDevice->Acquire();
		}
	}
}
