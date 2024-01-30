#ifndef  _TOOLKITUI_HPP
#define _TOOLKITUI_HPP

#include <Windows.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>

namespace ui
{
	class ToolKitUI
	{
	private:
		HWND wnd;
        bool show_demo_window = true;
	public:
		void Initialize(HWND wnd);
		void BeginRender();
		void EndRender();
		void Shutdown();
		void ApplyTheme();
	};

	inline ToolKitUI g_UI;
}

#endif // ! _TOOLKITUI_HPP