///============================================================================
///@file	Geometry.cpp
///@brief	Class used for rendering the geometry in the scene.
///
///@author	Héctor Morales Piloni
///@date	November 18, 2006
///============================================================================

#include "Geometry.h"

///----------------------------------------------------------------------------
///Default constructor
///----------------------------------------------------------------------------
Geometry::Geometry() : m_DepthMap(0)
{
}

///----------------------------------------------------------------------------
///Draw the objects in the scene
///@param	angle - used to animate part of the geometry
///----------------------------------------------------------------------------
void Geometry::Draw(GLfloat angle)
{
	//Display lists for the objects
	static GLuint BaseList=0, SphereList=0, ConeList=0, TorusList=0;

	//create Base display list if needed
	if(!BaseList)
	{
		BaseList = glGenLists(1);
		glNewList(BaseList, GL_COMPILE);
		{
			glColor3f(0.0, 0.0, 1.0);
			glPushMatrix();

			glScalef(7.0, 0.3, 7.0);
			glutSolidCube(1.0);

			glPopMatrix();
		}
		glEndList();
	}

	//Create torus display list if needed
	if(!TorusList)
	{
		TorusList=glGenLists(1);
		glNewList(TorusList, GL_COMPILE);
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			glPushMatrix();

			glTranslatef(0.0f, 1.0f, 0.0f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			glutSolidTorus(0.3, 1.0, 24, 48);

			glPopMatrix();
		}
		glEndList();
	}

	//create Spheres display list if needed
	if(!SphereList)
	{
		SphereList = glGenLists(1);
		glNewList(SphereList, GL_COMPILE);
		{
			glColor3f(0.0, 1.0, 0.0);
			glPushMatrix();

			glTranslatef(0.5f, 2.0f, 0.5f);
			glutSolidSphere(0.2, 24, 24);

			glTranslatef(-1.0f, 0.0f, 0.0f);
			glutSolidSphere(0.2, 24, 24);

			glTranslatef(0.0f, 0.0f,-1.0f);
			glutSolidSphere(0.2, 24, 24);

			glTranslatef(1.0f, 0.0f, 0.0f);
			glutSolidSphere(0.2, 24, 24);

			glPopMatrix();
		}
		glEndList();
	}

	//create Cone display list if needed
	if(!ConeList)
	{
		ConeList = glGenLists(1);
		glNewList(ConeList, GL_COMPILE);
		{
			glColor3f(1.0, 1.0, 0.0);
			glPushMatrix();

			glTranslatef(2.0, 0.0, 2.0);
			glRotatef(-90, 1.0, 0.0, 0.0);
			glutSolidCone(0.3, 2.0, 25, 25);

			glTranslatef(-4.0, 0.0, 0.0);
			glutSolidCone(0.3, 2.0, 25, 25);

			glTranslatef(0.0, 4.0, 0.0);
			glutSolidCone(0.3, 2.0, 25, 25);

			glTranslatef(4.0, 0.0, 0.0);
			glScalef(1.0, 1.0, 1.5);
			glutSolidCone(0.3, 2.0, 25, 25);

			glPopMatrix();
		}
		glEndList();
	}

	//draw objects
	glCallList(BaseList);
	glCallList(TorusList);
	glCallList(ConeList);

	//animate and draw spheres
	glPushMatrix();
	glRotatef(angle, 0.0, 1.0, 0.0);
	glCallList(SphereList);
	glPopMatrix();

}

///----------------------------------------------------------------------------
///Set the lights in the scene
///----------------------------------------------------------------------------
void Geometry::SetLights(GLfloat pos[])
{
	//define the light position
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	//enable lighting and light0
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

///----------------------------------------------------------------------------
///Set the materials of the objects in the scene
///----------------------------------------------------------------------------
void Geometry::SetMaterials()
{
	GLfloat white[4] = {1.0, 1.0, 1.0};

	//set object color as material color
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	//enable color material state
	glEnable(GL_COLOR_MATERIAL);

	//enable specular lighting
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 20.f);
}

///----------------------------------------------------------------------------
///Sets the camera position.
///@param	pos[] - the camera position (x,y,z)
///----------------------------------------------------------------------------
void Geometry::SetCameraPosition(GLfloat pos[])
{
	m_Camera[0] = pos[0];
	m_Camera[1] = pos[1];
	m_Camera[2] = pos[2];
}

///----------------------------------------------------------------------------
///Gets the camera position.
///@param	pos[] - the returned camera position (x,y,z)
///----------------------------------------------------------------------------
void Geometry::GetCameraPosition(GLfloat *pos) const
{
	pos[0] = m_Camera[0];
	pos[1] = m_Camera[1];
	pos[2] = m_Camera[2];
}

///----------------------------------------------------------------------------
///Gets the light position.
///@param	pos[] - the returned light position (x,y,z)
///----------------------------------------------------------------------------
void Geometry::GetLightPosition(GLfloat *pos) const
{
	pos[0] = m_Light[0];
	pos[1] = m_Light[1];
	pos[2] = m_Light[2];
}

///----------------------------------------------------------------------------
///Set textures for shadow maps
///----------------------------------------------------------------------------
void Geometry::SetShadowTexture()
{
	//generate the texture name
	glGenTextures(1, &m_DepthMap);

	//the first time we call this function initializes the 2D texture 
	//object with NULL texture image
	glBindTexture(GL_TEXTURE_2D, m_DepthMap);

	//specify the 2D texture image
	glTexImage2D(GL_TEXTURE_2D,			//target must be GL_TEXTURE_2D
				 0,						//LOD number
				 GL_DEPTH_COMPONENT,	//number of color components
				 DEPTH_MAP_WIDTH,		//width
				 DEPTH_MAP_HEIGHT,		//height
				 0,						//border
				 GL_DEPTH_COMPONENT,	//format of pixel data
				 GL_UNSIGNED_BYTE,		//type of pixel data
				 NULL);					//pointer to image data in memory

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	//enable needed states
	glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
}

///----------------------------------------------------------------------------
///GetShadowTextObj()
///@returns the current shadow texture objectID
///----------------------------------------------------------------------------
GLuint Geometry::GetShadowTexObj() const
{
	return m_DepthMap;
}

///----------------------------------------------------------------------------
///Computes the transposed matrix of the given 4x4 matrix
///@param	m - an array of floats representing the matrix (16 values)
///----------------------------------------------------------------------------
void Geometry::Transpose4x4Matrix(GLdouble M[16])
{
    GLdouble  tmp;
	#define Swap(a, b)	tmp = a; a = b; b = tmp
		Swap(M[1],  M[4]);
		Swap(M[2],  M[8]);
		Swap(M[3],  M[12]);
		Swap(M[6],  M[9]);
		Swap(M[7],  M[13]);
		Swap(M[11], M[14]);
	#undef Swap
}

///----------------------------------------------------------------------------
///Computes the inverted matrix of the given 4x4 matrix
///@param	m - an array of floats representing the matrix (16 values)
///@return	false if the matrix is not invertible
///----------------------------------------------------------------------------
GLboolean Geometry::Invert4x4Matrix(GLdouble M[16])
{
	int i,j,r;

	//define the adjoint matrix
	GLdouble Madj[4][8] = {{M[0], M[4], M[8], M[12], 1, 0, 0, 0},
						   {M[1], M[5], M[9], M[13], 0, 1, 0, 0},
						   {M[2], M[6], M[10],M[14], 0, 0, 1, 0},
						   {M[3], M[7], M[11],M[15], 0, 0, 0, 1}};


	//loop through columns 1 to 4
	for(j=0; j<4; j++)
	{
		//find the row r with r >= j such that Madj[r,j] has the largest absolute value
		GLdouble largest = 0.0;
		for(i=j; i<4; i++)
		{
			GLdouble value = abs(Madj[i][j]);
			if(value > largest)
			{
				r = i;
				largest = value;
			}
		}

		//if no such row exists for which Madj[i,j] != 0 then M is not invertible
		if(largest == 0.0) 
			return false;

		//if r != j then exchange rows r & j. This is the pivot operation necessary to
		//remove zeros from the main diagonal and provide numerical stability
		if(r != j)
		{
			GLdouble tmp;
			#define Swap(a, b)	tmp = a; a = b; b = tmp
				Swap(Madj[r][0], Madj[j][0]);
				Swap(Madj[r][1], Madj[j][1]);
				Swap(Madj[r][2], Madj[j][2]);
				Swap(Madj[r][3], Madj[j][3]);
				Swap(Madj[r][4], Madj[j][4]);
				Swap(Madj[r][5], Madj[j][5]);
				Swap(Madj[r][6], Madj[j][6]);
				Swap(Madj[r][7], Madj[j][7]);
			#undef Swap
		}

		//Multiply row j by 1/Madj[j,j]. This sets the (j,j) entry of Madj to 1
		GLdouble f = 1/Madj[j][j];
		Madj[j][0] *= f;
		Madj[j][1] *= f;
		Madj[j][2] *= f;
		Madj[j][3] *= f;
		Madj[j][4] *= f;
		Madj[j][5] *= f;
		Madj[j][6] *= f;
		Madj[j][7] *= f;

		//For each row r where 1 <= r <= n and r != j add -Madj[r,j] times row j to row r.
		//This step clears each entry above and below row r in column j to zero.
		for(r=0; r<4; r++)
		{
			if(r != j)
			{
				f = -Madj[r][j];
				Madj[r][0] += f * Madj[j][0];
				Madj[r][1] += f * Madj[j][1];
				Madj[r][2] += f * Madj[j][2];
				Madj[r][3] += f * Madj[j][3];
				Madj[r][4] += f * Madj[j][4];
				Madj[r][5] += f * Madj[j][5];
				Madj[r][6] += f * Madj[j][6];
				Madj[r][7] += f * Madj[j][7];
			}
		}	
	}

	//copy right-side of adjoint matrix (which contains the inverse matrix) to M
	M[0]  = Madj[0][4];
	M[1]  = Madj[1][4];
	M[2]  = Madj[2][4];
	M[3]  = Madj[3][4];
	M[4]  = Madj[0][5];
	M[5]  = Madj[1][5];
	M[6]  = Madj[2][5];
	M[7]  = Madj[3][5];
	M[8]  = Madj[0][6];
	M[9]  = Madj[1][6];
	M[10] = Madj[2][6];
	M[11] = Madj[3][6];
	M[12] = Madj[0][7];
	M[13] = Madj[1][7];
	M[14] = Madj[2][7];
	M[15] = Madj[3][7];

	return true;
}