#include "MansInterfacin.h"

#include <stdio.h>

MansInterfacin::MansInterfacin(unsigned int Width, unsigned int Height, void(*Callback_initedWindow)(NativeWindowSystem*), void(*Callback_initedGraphics)(GraphicsSystem*))
	: native_window_system(Width, Height, Callback_initedWindow), graphics_system(&native_window_system, Callback_initedGraphics), ui(&graphics_system){
}

bool MansInterfacin::Intialize(){
	this->native_window_system.InitWindow(); this->graphics_system.InitD3D();
	//return this->native_window_system.InitWindow() && this->graphics_system.InitD3D();
	return true;
}

void MansInterfacin::Terminate(){
	this->graphics_system.TermD3D();
	this->native_window_system.TermWindow();
}
