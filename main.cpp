///============================================================================
///@file	main.cpp
///@brief	Soft Shadows Demo.
///			Implementation of the Single Sample Soft Shadows using Depth Maps
///			by Stefan Brabec
///
///@author	Héctor Morales Piloni
///@date	November 13, 2006
///============================================================================

#include <windows.h>
#include "GLApp.h"

GLApp *myApp;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int iCmdShow)
{
	int retCode;

	//create a new 800x600 window application
	myApp = new GLApp("Soft Shadows Demo", 800, 600);
	
	//initilize the application
	if(!myApp->InitInstance(hInstance, lpCmdLine, iCmdShow)) 
	{
		delete myApp;
		return 0;
	}

	//start the application
	retCode = myApp->StartApp();

	//clean-up
	delete myApp;

	return retCode;
}