/**************************************************************************/
/*  native_window.hpp                                                     */
/*                                                                        */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include "native_window_windows.hpp"
#else
#include "native_window_base.hpp"
namespace godot {
	typedef NativeWindowBase NativeWindow;
}
#endif
