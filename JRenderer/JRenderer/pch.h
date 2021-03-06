// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Use the C++ standard templated min/max
#define NOMINMAX

// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h> //Unicode 심볼이 있을떄 wchar_t로 정의해줌.
#include <stdio.h>
#include <pix.h>

#include <d3d11_3.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <wrl.h>
//Same with Unreal Engine data-type
/*
typedef signed int int32;
typedef unsigned int uint32;*/


//#include "Audio.h"
//#include "CommonStates.h"
//#include "DDSTextureLoader.h"
//#include "Effects.h"
//#include "GamePad.h"
//#include "GeometricPrimitive.h"
//#include "Keyboard.h"
//#include "Model.h"
//#include "Mouse.h"
//#include "PrimitiveBatch.h"
//#include "SimpleMath.h"
//#include "SpriteBatch.h"
//#include "SpriteFont.h"
//#include "VertexTypes.h"

namespace DX
{
	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}

		virtual const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
			return s_str;
		}

	private:
		HRESULT result;
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw com_exception(hr);
		}
	}
}
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
