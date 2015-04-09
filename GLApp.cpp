///============================================================================
///@file	GLApp.cpp
///@brief	OpenGL Application Class Implementation
///
///@author	Héctor Morales Piloni
///@date	November 13, 2006
///============================================================================

#include "GLApp.h"

///----------------------------------------------------------------------------
///Default constructor.
///----------------------------------------------------------------------------
GLApp::GLApp()
{
	//call overloaded constructor with default values
	GLApp("", 640, 480);
}

///----------------------------------------------------------------------------
///Overloaded constructor.
///@param	windowTitle - window title
///@param	width		- window width
///@param	height		- window height
///----------------------------------------------------------------------------
GLApp::GLApp(LPSTR windowTitle, USHORT width, USHORT height)
{
	//clear all required values
	m_hWnd	= NULL;
	m_hDC	= NULL;
	m_hRC	= NULL;

	//clear Pbuffer object
	m_Pbuffer.hDC = NULL;
	m_Pbuffer.hRC = NULL;
	m_Pbuffer.hPBuffer = NULL;

	//set all required values
	m_WindowTitle	= windowTitle;
	m_Width			= width;
	m_Height		= height;
}

///----------------------------------------------------------------------------
///Default destructor.
///----------------------------------------------------------------------------
GLApp::~GLApp()
{
	//perform clean-up
	ShutDown();
}

///----------------------------------------------------------------------------
///Sets the current rendering context
///@param	context to be drawn on.
///----------------------------------------------------------------------------
void GLApp::SetContext(int context)
{
	HDC hDC;
	HGLRC hRC;

	if(context == GLAPP_WINDOW_CONTEXT)
	{
		hDC = m_hDC;
		hRC = m_hRC;
	}
	else if(context == GLAPP_PBUFFER_CONTEXT)
	{
		hDC = m_Pbuffer.hDC;
		hRC = m_Pbuffer.hRC;
	}
	
	if(wglMakeCurrent(hDC, hRC) == FALSE)
	{
		MessageBox(NULL, 
				   "Could not set the current window context!", 
				   "ERROR", 
				   MB_OK | MB_ICONEXCLAMATION);

		//exit the program with an error code
		exit(-1);
	}
}


///----------------------------------------------------------------------------
///Initialize OpenGL graphic device context.
///----------------------------------------------------------------------------
void GLApp::InitGraphics()
{
	GLuint PixelFormat;			//this will hold the actual pixel format
	PIXELFORMATDESCRIPTOR pfd;	//structure to maintain pixel format information 
								//about our GL window

	//clear our PIXELFORMATDESCRIPTOR structure
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);	//the size of this data structure
	pfd.nVersion = 1;							//this value should be set to 1!
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |			//the buffer can draw to a window or DC
				  PFD_SUPPORT_OPENGL |			//the buffer supports OpenGL drawing 
				  PFD_DOUBLEBUFFER;				//the buffer is double-buffered
	pfd.iPixelType = PFD_TYPE_RGBA;				//RGBA pixels
	pfd.cColorBits = 32;						//number of color bitplanes in each color buffer
	pfd.cDepthBits = 24;						//depth of the deph (z-axis) buffer

	//get the window device context
	m_hDC = GetDC(m_hWnd);

	//we should select a pixel format in the device 
	//context before calling wglCreateContext 
	PixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	SetPixelFormat(m_hDC, PixelFormat, &pfd);
	DescribePixelFormat(m_hDC, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	
	//creates a new OpenGL rendering context, which is 
	//suitable for drawing on the device referenced by m_hDC
	m_hRC = wglCreateContext(m_hDC);
	
	//set the current rendering context
	SetContext(GLAPP_WINDOW_CONTEXT);

	//initialize the viewport
	Reshape(m_Width, m_Height);

	//set lights, materials & textures
	GLfloat lightPos[3] = {-5.0, 10.0, 6.0};
	m_Geometry.SetLights(lightPos);
	m_Geometry.SetMaterials();
	m_Geometry.SetShadowTexture();

	//set camera position
	GLfloat cameraPos[3] = {5.0, 5.0, 5.0};
	m_Geometry.SetCameraPosition(cameraPos);

	//calculate & save matrices for later use, 
	//select modelview as temporal matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	{
		//camera projection matrix
		glLoadIdentity();
		gluPerspective(45.0f, (float)m_Width/m_Height, 1.0f, 100.0f);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_CameraProjectionMatrix);

		//camera modelview matrix
		glLoadIdentity();
		gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_CameraViewMatrix);

		//light projection matrix
		glLoadIdentity();
		gluPerspective(45.0f, 1.0f, 1.0, 100.0f);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_LightProjectionMatrix);
		
		//light modelview matrix
		GLfloat LightPos[4];
		glGetLightfv(GL_LIGHT0, GL_POSITION, LightPos);
		glLoadIdentity();
		gluLookAt(lightPos[0], lightPos[1], lightPos[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_LightViewMatrix);
	}
	glPopMatrix();

	//enable needed states
    glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//initialize OpenGL extensions
	InitExtensions();

	InitPbuffer();

	//set our Pbuffer as the current rendering context and initialize some states
	SetContext(GLAPP_PBUFFER_CONTEXT);
	glEnable(GL_DEPTH_TEST);

	//create vertex & pixel shaders
	m_Shader.CreateShader();
	m_Shader.AttachObject(new ShaderObject("ShadowMapping.vert", GL_VERTEX_SHADER));
	m_Shader.AttachObject(new ShaderObject("ShadowMapping.frag", GL_FRAGMENT_SHADER));
	m_Shader.Link();
}

///----------------------------------------------------------------------------
///Initializes our Pbuffer object
///----------------------------------------------------------------------------
void GLApp::InitPbuffer()
{
	int PixelFormat = 0;
	unsigned int count;
    m_Pbuffer.hPBuffer = NULL;

    //Define the minimum pixel format requirements we will need for our 
    //p-buffer. A p-buffer is just like a frame buffer, it can have a depth 
    //buffer associated with it and it can be double buffered    
    const int attr[] =
    {
        WGL_SUPPORT_OPENGL_ARB,			TRUE,	//Pbuffer will be used with OpenGL
        WGL_DRAW_TO_PBUFFER_ARB,		TRUE,   //Enable render to p-buffer
        WGL_BIND_TO_TEXTURE_RGBA_ARB,	TRUE,	//Pbuffer will be used as a texture
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,	//Pbuffer will use RGBA
        WGL_DOUBLE_BUFFER_ARB,			FALSE,  //We don't require double buffering
		WGL_DEPTH_BITS_ARB,				24,		//32 bits depth buffer
        0										//Zero terminates the list
    };

    wglChoosePixelFormat(m_hDC, attr, NULL, 1, &PixelFormat, &count );

	//Create the Pbuffer
	m_Pbuffer.hPBuffer = wglCreatePbuffer(m_hDC, PixelFormat, m_Geometry.DEPTH_MAP_WIDTH, m_Geometry.DEPTH_MAP_HEIGHT, NULL);
    m_Pbuffer.hDC = wglGetPbufferDC(m_Pbuffer.hPBuffer);
    m_Pbuffer.hRC = wglCreateContext(m_Pbuffer.hDC);

	//Enable multiple OpenGL rendering contexts to share a single display-list space
	wglShareLists(m_hRC, m_Pbuffer.hRC);
}

///----------------------------------------------------------------------------
///Clean up resources.
///----------------------------------------------------------------------------
bool GLApp::ShutDown()
{
	//destroy shader program
	m_Shader.DestroyShader();

	if(m_hRC)
	{
		//make current rendering context NULL 
		wglMakeCurrent(NULL, NULL);
		
		//delete GL context
		wglDeleteContext(m_hRC);	
	}

	//clean-up pBuffer object
	if(m_Pbuffer.hRC)
	{
		wglMakeCurrent(m_Pbuffer.hDC, m_Pbuffer.hRC);
		wglDeleteContext(m_Pbuffer.hRC);
		wglReleasePbufferDC(m_Pbuffer.hPBuffer, m_Pbuffer.hDC);
		wglDestroyPbuffer(m_Pbuffer.hPBuffer);
	}

	//release the device context
	if(m_hWnd && m_hDC) ReleaseDC(m_hWnd, m_hDC);
	if(m_hWnd && m_Pbuffer.hDC) ReleaseDC(m_hWnd, m_Pbuffer.hDC);

	//destroy window explicitly
	if(m_hWnd) DestroyWindow(m_hWnd);

	//reset all required values
	m_hDC = NULL;
	m_hRC = NULL;
	m_hWnd = NULL;

	m_Pbuffer.hRC = NULL;
	m_Pbuffer.hDC = NULL;

	return true;
}

///----------------------------------------------------------------------------
///This function handles messages for the GLApp object
///@param	hWnd - handle to window
///@param	Msg - the message sent
///@param	wParam - message parameter
///@param	lParam - message parameter
///----------------------------------------------------------------------------
LRESULT GLApp::DisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	GLfloat CameraPos[3];

	switch(Msg)
	{
		case WM_CREATE:
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SIZE:
			//store new viewport sizes
			m_Width  = LOWORD(lParam);
			m_Height = HIWORD(lParam);

			Reshape(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_CHAR:
			switch(wParam)
			{
				case '+':
					Zoom(-0.1);
					break;

				case '-':
					Zoom(0.1);
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}

///----------------------------------------------------------------------------
///Draws some text in the scene (i.e. FPS, etc)
///----------------------------------------------------------------------------
void GLApp::RenderText(LPTSTR text)
{
}

///----------------------------------------------------------------------------
///Creates the shadow map texture based on light's point of view.
///----------------------------------------------------------------------------
void GLApp::CreateShadowMap(GLfloat angle)
{
	//render the shadow map to Pbuffer
	SetContext(GLAPP_PBUFFER_CONTEXT);

	//clear the depth and color buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//disable lighting and textures
	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);

	//render from light's point of view
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	{
		glLoadMatrixd(m_LightProjectionMatrix);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			//disable writing on frame-buffer color components
			glColorMask(0,0,0,0);

			//Pushes the scene back by a small epsilon in the z-buffer to avoid z-fighting 
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.1, 4.0);

			glLoadMatrixd(m_LightViewMatrix);
			m_Geometry.Draw(angle);
			
			glDisable(GL_POLYGON_OFFSET_FILL);

			//restore color component writing
			glColorMask(1,1,1,1);
		}
		glPopMatrix();
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	//subsequent calls to glBindTexture activates the texture object
	glBindTexture(GL_TEXTURE_2D, m_Geometry.GetShadowTexObj());

	//Copy the depth buffer into the depth map texture object
	glCopyTexImage2D(GL_TEXTURE_2D,					//target, must be GL_TEXTURE_2D
					 0,								//LOD number, 0 is the base
					 GL_DEPTH_COMPONENT,			//Internal format for texture data
					 0,								//The window coordinates of the lower-left corner
					 0,								//of the rectangular region of pixels to be copied
					 m_Geometry.DEPTH_MAP_WIDTH,	//width of texture image (must be power of 2)
					 m_Geometry.DEPTH_MAP_HEIGHT,	//height of texture image (must be power of 2)
					 0);							//border width

	//restore render states
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);

	SetContext(GLAPP_WINDOW_CONTEXT);
}

///----------------------------------------------------------------------------
///We need texture coordinates as if the light source were the eye point;
///this matrix takes us from eye space to the light's clip space
///by concatenating the following matrices: T = P.V.Ci
///where T = Texture matrix
///		 P = Light's position projection matrix
///		 V = Light's position modelview matrix
///		 Ci= Camera's "Inverse" view matrix
///----------------------------------------------------------------------------
void GLApp::CreateTextureMatrix()
{
	GLdouble tmpMatrix[16];

	//get current camera view matrix
	glGetDoublev(GL_MODELVIEW_MATRIX, tmpMatrix);

	//invert camera view matrix
	if(!m_Geometry.Invert4x4Matrix(tmpMatrix))
	{
		//it should never get here!
		MessageBox(NULL, 
				   "Camera view matrix is not invertible!", 
				   "ERROR", 
				   MB_OK | MB_ICONEXCLAMATION);
	}

	//following opperations affect texture 1, we must make it active!
	glActiveTexture(GL_TEXTURE1);

	//set texture matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	//we need to clamp our coordinates from [-1,1] to [0,1]
	glTranslatef(0.5, 0.5, 0.5);
	glScalef(0.5, 0.5, 0.5);

	//concatenate matrices
	glMultMatrixd(m_LightProjectionMatrix);
	glMultMatrixd(m_LightViewMatrix);
	glMultMatrixd(tmpMatrix);
}

///----------------------------------------------------------------------------
///Overriden Render function (draws the scene).
///----------------------------------------------------------------------------
void GLApp::Render()
{
	static GLfloat angle = 0.0;

	//lock the framerate to 60 FPS
	m_Timer.Tick(60.0f);

	//update the angle for animation
	angle += 50.0f * m_Timer.GetTimeElapsed();

	//1st pass, create shadow map & texture coordinates
	CreateShadowMap(angle);
	CreateTextureMatrix();

	//2nd pass, render from camera point of view
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0,0, m_Width, m_Height);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(m_CameraProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(m_CameraViewMatrix);

	//bind texture object
	glBindTexture(GL_TEXTURE_2D, m_Geometry.GetShadowTexObj());

	//enable programmable pipeline
	m_Shader.EnableShader();
	
	//set uniform variables
	m_Shader.SetUniform("shadowMap", 1);

	//and draw the scene...
    m_Geometry.Draw(angle);

	//disable programmable pipeline
	m_Shader.DisableShader();

	SwapBuffers(m_hDC);
}

///----------------------------------------------------------------------------
///Reset the viewport when window size changes
///@param	w - window width
///@param	h - window height
///----------------------------------------------------------------------------
void GLApp::Reshape(int w, int h)
{
	//set the viewport
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	{
		//calculate the new projection matrix
		glLoadIdentity();
		gluPerspective(45.0f, (float)w/h, 1.0f, 100.0f);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_CameraProjectionMatrix);
	}
	glPopMatrix();
}

///----------------------------------------------------------------------------
///Zoom in/out the camera
///@param	zoomFactor - how much zoom are we going to apply
///			if positive the camera gets closer to the scene,
///			if negative the camera gets further.
///----------------------------------------------------------------------------
void GLApp::Zoom(GLfloat zoomFactor)
{
	//set new camera position
	GLfloat cameraPos[3];
	
	m_Geometry.GetCameraPosition(cameraPos);
	cameraPos[0] += zoomFactor;
	cameraPos[1] += zoomFactor;
	cameraPos[2] += zoomFactor;
	m_Geometry.SetCameraPosition(cameraPos);

	//calculate the new modelview matrix
	glLoadIdentity();
	gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glGetDoublev(GL_MODELVIEW_MATRIX, m_CameraViewMatrix);
}
