// Assignment_016
// ShapesOnGraph

//header
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "vmath.h"

//namespaces
using namespace std;
using namespace vmath;

//global variable declarations
bool bFullscreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gGLXFBConfig;
GLXContext gGLXContext;

GLuint gShaderProgramObject;
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0,
};
//vao-shapes
//vbo-color,texture,....
GLuint vao_graph;
GLuint vbo_graph_position;
GLuint vbo_graph_color;

GLuint vao_triangle;
GLuint vbo_position_triangle;
GLuint vbo_color_triangle;

GLuint vao_square;
GLuint vbo_position_square;
GLuint vbo_color_square;

GLuint vao_circle;
GLuint vbo_circle_position;
GLuint vbo_circle_color;
GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

const int numberOfCircleLines = 5000;
const int numberOfCircleVertices = numberOfCircleLines * 2 * 3;
const int numberOfTriangleVertices = 3 * 3;
const int numberOfSquareVertices = 4 * 3;
const int numberOfGraphVertices = 1024;
// 21 lines with 2 verticle and horizontal 3 line with 2 middle lines = (21*2*3)+2 = 1024
const int PI = 3.14;
int giNumberOfGraphLines = 0;
int giTotalNumberOfLines = 0;

void resize(int, int);

//entry point function
int main(void)
{
	//function prototypes
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void uninitialize();
	void initialize(void);
	void resize(int, int);
	void display(void);

    //variable declarations
    int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;
	char keys[26];
	bool bDone = false;

	XEvent event;
	KeySym keysym;
	
	//code
    CreateWindow();

    initialize();
	
	//Game Loop
    while(bDone == false)
    {
        while(XPending(gpDisplay))
        {
        	XNextEvent(gpDisplay, &event);
			switch(event.type)
			{
				case MapNotify:
					break;
				case KeyPress:
					keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
					switch(keysym)
					{
						case XK_Escape:
							bDone = true;					
						default:
							break;
					}
					XLookupString(&event.xkey,keys, sizeof(keys), NULL, NULL);
					switch(keys[0])
					{
						case 'F':
						case 'f':
							if(bFullscreen == false)
							{
								ToggleFullscreen();
								bFullscreen = true;
							}
							else
							{
								ToggleFullscreen();
								bFullscreen = false;
							}
							break;
					}
					break;
				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1:		//Left Mouse button
							break;
						case 2:		//Middle Mouse button
							break;
						case 3:		//Right Mouse button
							break;
						case 4:		//Mouse Wheel Up
							break;
						case 5:		//Mouse Wheel Down
							break;
						default:
							break;
					}
					break;
				case MotionNotify:		//WM_MOUSE_MOVE
					break;
				case ConfigureNotify:	//WM_SIZE
					winWidth = event.xconfigure.width;
					winHeight = event.xconfigure.height;
					resize(winWidth, winHeight);
					break;
				case Expose:			//WM_PAINT
					break;
				case DestroyNotify:		//WM_DESTORY
					break;
				case 33:				//WM_DESTORY
					bDone = true;
				default:
					break;
			}
        }
        //Update call..
        display();
	}
	
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	//function prototypes
	void uninitialize(void);
	
	//variable declarations
    XSetWindowAttributes winAttribs;
    int defaultScreen;
    int styleMask;
    GLXFBConfig *pGLXFBConfig = NULL;
    GLXFBConfig bestGLXFBConfig;
    XVisualInfo *pTempXVisualInfo = NULL;
    int iNumberOfFBConfigs = 0;

    //1: Initialise Frame Buffer Attributes.
    static int frameBufferAttributes[] = {
    	GLX_X_RENDERABLE, True,
    	GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    	GLX_RENDER_TYPE, GLX_RGBA_BIT,
    	GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };
	
	//code
    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        printf("Error: Unable to open XDisplay.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }

    defaultScreen = XDefaultScreen(gpDisplay);

    //2: Reterive all 'FBConfig's, driver has.
    pGLXFBConfig = glXChooseFBConfig(gpDisplay, defaultScreen, frameBufferAttributes, &iNumberOfFBConfigs);
    printf("Their are %d matching FBConfigs.\n", iNumberOfFBConfigs);

    int bestFrameBufferConfig = -1;
    int bestNumberOfSamples = -1;
    int worstFrameBufferConfig = -1;
    int worstNumberOfSamples = 999;
    for(int i = 0; i<iNumberOfFBConfigs; i++)
    {
    	//3: For each optained FBConfigs get TempVisualInfo.
    	//Its use is just to check the capability of calling following two calls.
    	pTempXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfig[i]);
    	if(pTempXVisualInfo)
    	{
    		//4: Get Number of samplerBuffer from respective FBConfigs.
    		int sampleBuffer, samples;
    		glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLE_BUFFERS, &sampleBuffer);
    		//5: Get Number of samples from respective FBConfigs.
            glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLES, &samples);

            //6: More the Number of samples and sampleBuffer, More Elligible FBConfig is.
            if(bestFrameBufferConfig < 0 || sampleBuffer && samples > bestNumberOfSamples)
            {
            	bestFrameBufferConfig = i;
            	bestNumberOfSamples = samples;
            }
            if(worstFrameBufferConfig < 0 || !sampleBuffer || samples < worstNumberOfSamples)
            {
            	worstFrameBufferConfig = i;
            	worstNumberOfSamples = samples;
            }
    	}
    	XFree(pTempXVisualInfo);
    }

    //7: Now Assign the found best one.
    bestGLXFBConfig = pGLXFBConfig[bestFrameBufferConfig];

    //8: Now Assign the same best to global one.
    gGLXFBConfig = bestGLXFBConfig;

    //9: Free the optained GLXFBConfig Array.
    XFree(pGLXFBConfig);

    //10: Accordingly now get the best Visual.
    gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, bestGLXFBConfig);

    if(gpXVisualInfo == NULL)
    {
        printf("Error: Unable to choose visual.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }

    winAttribs.border_pixel = 0;
    winAttribs.border_pixmap = 0;
    winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
    winAttribs.background_pixmap = 0;
    winAttribs.colormap = XCreateColormap(gpDisplay, XRootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone);
    winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;

    styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
    gColormap = winAttribs.colormap;

    gWindow = XCreateWindow(gpDisplay, XRootWindow(gpDisplay, gpXVisualInfo->screen), 0, 0, giWindowWidth, giWindowHeight, 0, gpXVisualInfo->depth, InputOutput, gpXVisualInfo->visual, styleMask, &winAttribs);

    if(!gWindow)
    {
        printf("Error: Failed to create main window.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }

    XStoreName(gpDisplay, gWindow, "Rohan V. Waghmode - XWindow");

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);
	
    XMapWindow(gpDisplay, gWindow);
}

void initialize(void)
{
    GLuint gVertexShaderObject;
    GLuint gFragmentShaderObject;

	//function prototypes
	void uninitialize(void);

	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");

    if(glXCreateContextAttribsARB == NULL)
    {
        printf("Error: Unable to get glXCreateContextAttribsARBProc address.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }

    GLint attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);

    // If not optained the  Higher one , Specify the lowest one;
    // And it will give you the highest one possible/known to it.
    if(!gGLXContext)
    {
        GLint attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            None
        };

        gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);
    }

    // Check whether the optained context is really hardware rendering context or not...
    if(!glXIsDirect(gpDisplay, gGLXContext))
    {
        printf("The optained context is not hardware rendering context.\n");
    }
    else
    {
        printf("The optained context is hardware rendering context.\n");
    }

    // Make the Optained Context as Current OpenGL Context.
    glXMakeCurrent(gpDisplay, gWindow, gGLXContext);

    GLenum result = glewInit();
	if (result != GLEW_OK)
	{
		printf("Cannot initialize GLEW.\n");
		uninitialize();
		exit(0);
	}

    ///////////////////////////////////////////////////////////////////////////////////////////////Vertex Shader
    //Define Vertex Shader Object
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    //Write Vertex Shader Object
    const GLchar* vertexShaderSourceCode =
        "#version 450 core" \
        "\n" \
        "in vec4 vPosition;" \
        "in vec4 vColor;" \
        "uniform mat4 u_mvp_matrix;" \
        "out vec4 out_color;" \
        "void main(void)" \
        "{" \
        "   gl_Position = u_mvp_matrix * vPosition;" \
        "   out_color = vColor;" \
        "}";
    //Specifying Above code to the VertexShaderObject
    glShaderSource(gVertexShaderObject, 1, (const char**)& vertexShaderSourceCode, NULL);
    //Compile the Vertex Shader
    glCompileShader(gVertexShaderObject);
    //Error Checking for Vertex Shader Compiling
    GLint iShaderCompileStatus = 0;
    GLint iInfoLogLength = 0;
    GLchar* szInfoLog = NULL;
    glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
                printf("Vertex Shader Compile Log:\n%s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////Fragment Shader
    //Define Fragment Shader Object
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    //Write Fragment Shader Object
    const GLchar* fragmentShaderSourceCode =
        "#version 450 core" \
        "\n" \
        "in vec4 out_color;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "   FragColor = out_color;" \
        "}";
    //Specifying Above code to the FragmentShaderObject
    glShaderSource(gFragmentShaderObject, 1, (const char**)& fragmentShaderSourceCode, NULL);
    //Compile the fragment Shader
    glCompileShader(gFragmentShaderObject);
    //Error Checking for Fragment Shader Compiling
    iShaderCompileStatus = 0;
    iInfoLogLength = 0;
    szInfoLog = NULL;
    glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
                printf("Fragment Shader Compile Log:\n%s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////Shader Program
    //Create Shader Program Object
    gShaderProgramObject = glCreateProgram();
    //Attach Vertex Shader to Shader Program
    glAttachShader(gShaderProgramObject, gVertexShaderObject);
    //Attach Fragment Shader to Shader Program
    glAttachShader(gShaderProgramObject, gFragmentShaderObject);
    //preLinking Binding to Vertex Attributes
    glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
    //Link The Shader Program
    glLinkProgram(gShaderProgramObject);
    //Error Checking for Shader Program Linking
    GLint iProgramLinkStatus = 0;
    iInfoLogLength = 0;
    szInfoLog = NULL;
    glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
    if (iProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
                printf("Shader Program Link Log:\n%s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    //postLinking retriving uniform location
    mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
    ///////////////////////////////////////////////////////////////////////////////////////////////
// Graph
    GLfloat graphVertices[numberOfGraphVertices] = { 0 };
    GLfloat graphColor[numberOfGraphVertices] = { 0 };
    int iArrayIndex = 0;
    float fDifference = 1.0f / 20.0f;
    for (float fIterator = -1.0f; fIterator <= 1.05f; fIterator += fDifference)
    {
        graphColor[iArrayIndex] = 0;
        graphColor[iArrayIndex + 1] = 0;
        graphColor[iArrayIndex + 2] = 1;

        graphVertices[iArrayIndex++] = -1.0f;
        graphVertices[iArrayIndex++] = fIterator;
        graphVertices[iArrayIndex++] = 0.0f;

        graphColor[iArrayIndex] = 0;
        graphColor[iArrayIndex + 1] = 0;
        graphColor[iArrayIndex + 2] = 1;

        graphVertices[iArrayIndex++] = 1.0f;
        graphVertices[iArrayIndex++] = fIterator;
        graphVertices[iArrayIndex++] = 0.0f;

        graphColor[iArrayIndex] = 0;
        graphColor[iArrayIndex + 1] = 0;
        graphColor[iArrayIndex + 2] = 1;

        graphVertices[iArrayIndex++] = fIterator;
        graphVertices[iArrayIndex++] = -1.0;
        graphVertices[iArrayIndex++] = 0.0f;

        graphColor[iArrayIndex] = 0;
        graphColor[iArrayIndex + 1] = 0;
        graphColor[iArrayIndex + 2] = 1;

        graphVertices[iArrayIndex++] = fIterator;
        graphVertices[iArrayIndex++] = 1.0;
        graphVertices[iArrayIndex++] = 0.0f;
    }

    graphColor[iArrayIndex] = 1;
    graphColor[iArrayIndex + 1] = 0;
    graphColor[iArrayIndex + 2] = 0;

    graphVertices[iArrayIndex++] = -1.0f;
    graphVertices[iArrayIndex++] = 0.0f;
    graphVertices[iArrayIndex++] = 0.0f;

    graphColor[iArrayIndex] = 1;
    graphColor[iArrayIndex + 1] = 0;
    graphColor[iArrayIndex + 2] = 0;

    graphVertices[iArrayIndex++] = 1.0f;
    graphVertices[iArrayIndex++] = 0.0f;
    graphVertices[iArrayIndex++] = 0.0f;

    graphColor[iArrayIndex] = 0;
    graphColor[iArrayIndex + 1] = 1;
    graphColor[iArrayIndex + 2] = 0;

    graphVertices[iArrayIndex++] = 0.0f;
    graphVertices[iArrayIndex++] = -1.0f;
    graphVertices[iArrayIndex++] = 0.0f;

    graphColor[iArrayIndex] = 0;
    graphColor[iArrayIndex + 1] = 1;
    graphColor[iArrayIndex + 2] = 0;

    graphVertices[iArrayIndex++] = 0.0f;
    graphVertices[iArrayIndex++] = 1.0f;
    graphVertices[iArrayIndex++] = 0.0f;

    giNumberOfGraphLines = iArrayIndex / 2;

    // Triangle
    const GLfloat triangleVertices[] =
    {
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    const GLfloat triangleColor[] =
    {
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0
    };

    // Square
    const GLfloat squareVertices[] =
    {
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };

    const GLfloat squareColor[] =
    {
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0
    };
    
    glLineWidth(2.0f);

    // Circle
    GLfloat circleVertices[numberOfCircleLines * 2 * 3] = { 0 };
    GLfloat circleColor[numberOfCircleLines * 2 * 3] = { 0 };

    iArrayIndex = 0;
    for (int iIterator = 0; iIterator < numberOfCircleLines; iIterator++)
    {
        circleColor[iArrayIndex] = 1.0;
        circleColor[iArrayIndex + 1] = 1.0;
        circleColor[iArrayIndex + 2] = 0.0;
        circleVertices[iArrayIndex++] = (GLfloat)cos((5.0f * PI * iIterator) / numberOfCircleLines);
        circleVertices[iArrayIndex++] = (GLfloat)sin((5.0f * PI * iIterator) / numberOfCircleLines);
        circleVertices[iArrayIndex++] = 0.0f;

        iIterator++;

        circleColor[iArrayIndex] = 1.0;
        circleColor[iArrayIndex + 1] = 1.0;
        circleColor[iArrayIndex + 2] = 0.0;
        circleVertices[iArrayIndex++] = (GLfloat)cos((5.0f * PI * iIterator) / numberOfCircleLines);
        circleVertices[iArrayIndex++] = (GLfloat)sin((5.0f * PI * iIterator) / numberOfCircleLines);
        circleVertices[iArrayIndex++] = 0.0f;
    }
    
    //Create vao_graph
    glGenVertexArrays(1, &vao_graph);
    glBindVertexArray(vao_graph);
    //Position
    glGenBuffers(1, &vbo_graph_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_graph_position);

    glBufferData(GL_ARRAY_BUFFER, sizeof(graphVertices), graphVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Color
    glGenBuffers(1, &vbo_graph_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_graph_color);

    glBufferData(GL_ARRAY_BUFFER, sizeof(graphColor), graphColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    //Create vao_triangle
    glGenVertexArrays(1, &vao_triangle);
    glBindVertexArray(vao_triangle);
    //Position
    glGenBuffers(1, &vbo_position_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_triangle);

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Color
    glGenBuffers(1, &vbo_color_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_triangle);

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    //Create vao_square
    glGenVertexArrays(1, &vao_square);
    glBindVertexArray(vao_square);
    //Position
    glGenBuffers(1, &vbo_position_square);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_square);

    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Color
    glGenBuffers(1, &vbo_color_square);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_square);

    glBufferData(GL_ARRAY_BUFFER, sizeof(squareColor), squareColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    //Create vao_circle
    glGenVertexArrays(1, &vao_circle);
    glBindVertexArray(vao_circle);
    //Position
    glGenBuffers(1, &vbo_circle_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_circle_position);

    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Color
    glGenBuffers(1, &vbo_circle_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_circle_color);

    glBufferData(GL_ARRAY_BUFFER, sizeof(circleColor), circleColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    perspectiveProjectionMatrix = mat4::identity();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    resize(giWindowWidth, giWindowHeight);
}

void resize(int width, int height)
{
	if (height == 0)
    {
        height = 1;
    }
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix = perspective(45.0f, GLfloat(width) / GLfloat(height), 0.1f, 100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgramObject);

    //Declaration of Matrices
    mat4 modelViewMatrix;
    mat4 translateMatrix;
    mat4 scaleMatrix;
    mat4 modelViewProjectionMatrix;

    //////GRAPH//////

    //Initialize above Matrices to identity
    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    translateMatrix = mat4::identity();
    scaleMatrix = mat4::identity();

    //Do necessary Trasformation
    translateMatrix = translate(0.0f, 0.0f, -3.0f);

    //Do necessary Matrix Multiplication
    modelViewMatrix = modelViewMatrix * translateMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    //Send necessary Matrix to Shader in respective Uniform
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    //Bind with vao_graph
    glBindVertexArray(vao_graph);

    //simillar Bind with Texture if any


    //Draw necessary Scence
    glDrawArrays(GL_LINES, 0, giNumberOfGraphLines);

    //Unbind with vao_graph
    glBindVertexArray(0);
    
        //////TRIANGLE//////

        //Initialize above Matrices to identity
        modelViewMatrix = mat4::identity();
        modelViewProjectionMatrix = mat4::identity();
        translateMatrix = mat4::identity();
        scaleMatrix = mat4::identity();

        //Do necessary Trasformation
        translateMatrix = translate(-0.5f, 0.5f, -3.0f);
        scaleMatrix = scale(0.5f, 0.5f, 0.0f);

        //Do necessary Matrix Multiplication
        modelViewMatrix = modelViewMatrix * translateMatrix;
        modelViewMatrix = modelViewMatrix * scaleMatrix;
        modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

        //Send necessary Matrix to Shader in respective Uniform
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        //Bind with vao_triangle
        glBindVertexArray(vao_triangle);

        //simillar Bind with Texture if any


        //Draw necessary Scence
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //Unbind with vao_triangle
        glBindVertexArray(0);

        //////SQUARE//////

        //Initialize above changed Matrices to identity
        modelViewMatrix = mat4::identity();
        modelViewProjectionMatrix = mat4::identity();
        translateMatrix = mat4::identity();
        scaleMatrix = mat4::identity();

        //Do necessary Trasformation
        translateMatrix = translate(0.5f, 0.5f, -3.0f);
        scaleMatrix = scale(0.5f, 0.5f, 0.0f);

        //Do necessary Matrix Multiplication
        modelViewMatrix = modelViewMatrix * translateMatrix;
        modelViewMatrix = modelViewMatrix * scaleMatrix;
        modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

        //Send necessary Matrix to Shader in respective Uniform
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        //Bind with vao_square
        glBindVertexArray(vao_square);

        //simillar Bind with Texture if any


        //Draw necessary Scence
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //Unbind with vao_square
        glBindVertexArray(0);

        //////CIRCLE//////

        //Initialize above changed Matrices to identity
        modelViewMatrix = mat4::identity();
        modelViewProjectionMatrix = mat4::identity();
        translateMatrix = mat4::identity();
        scaleMatrix = mat4::identity();

        //Do necessary Trasformation
        translateMatrix = translate(0.0f, -0.5f, -3.0f);
        scaleMatrix = scale(0.5f, 0.5f, 0.5f);

        //Do necessary Matrix Multiplication
        modelViewMatrix = modelViewMatrix * translateMatrix;
        modelViewMatrix = modelViewMatrix * scaleMatrix;
        modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

        //Send necessary Matrix to Shader in respective Uniform
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        //Bind with vao_square
        glBindVertexArray(vao_circle);

        //simillar Bind with Texture if any


        //Draw necessary Scence
        glDrawArrays(GL_LINES, 0, numberOfCircleLines);

        //Unbind with vao_square
        glBindVertexArray(0);

    glUseProgram(0);

    glXSwapBuffers(gpDisplay, gWindow);
}

void ToggleFullscreen(void)
{
	//variable declarations
    Atom wm_state;
    Atom fullscreen;
    XEvent xev = {0};

	//code
    wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    memset(&xev, 0, sizeof(xev));
	
    xev.type = ClientMessage;
    xev.xclient.window = gWindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = bFullscreen ? 0 : 1;
	
	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[1] = fullscreen;

    XSendEvent(gpDisplay, XRootWindow(gpDisplay, gpXVisualInfo->screen), False, StructureNotifyMask, &xev);
}

void uninitialize(void)
{
    if (vbo_circle_color)
    {
        glDeleteBuffers(1, &vbo_circle_color);
    }
    if (vbo_circle_position)
    {
        glDeleteBuffers(1, &vbo_circle_position);
    }
    if (vao_circle)
    {
        glDeleteBuffers(1, &vao_circle);
    }
    if (vbo_color_square)
    {
        glDeleteBuffers(1, &vbo_color_square);
    }
    if (vbo_position_square)
    {
        glDeleteBuffers(1, &vbo_position_square);
    }
    if (vao_square)
    {
        glDeleteBuffers(1, &vao_square);
    }
    if (vbo_color_triangle)
    {
        glDeleteBuffers(1, &vbo_color_triangle);
    }
    if (vbo_position_triangle)
    {
        glDeleteBuffers(1, &vbo_position_triangle);
    }
    if (vao_triangle)
    {
        glDeleteBuffers(1, &vao_triangle);
    }
    if (vbo_graph_color)
    {
        glDeleteBuffers(1, &vbo_graph_color);
    }
    if (vbo_graph_position)
    {
        glDeleteBuffers(1, &vbo_graph_position);
    }
    if (vao_graph)
    {
        glDeleteBuffers(1, &vao_graph);
    }
    if (gShaderProgramObject)
    {
        GLsizei shaderCount;
        GLsizei shaderNumber;
        glUseProgram(gShaderProgramObject);

        //Ask Program: How many Programs are attched to you?
        glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);
        GLuint* pShaders = (GLuint*)malloc(2 * sizeof(shaderCount));
        if (pShaders)
        {
            glGetAttachedShaders(gShaderProgramObject, shaderCount, &shaderCount, pShaders);
            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                //Detach everyone
                glDetachShader(gShaderProgramObject, pShaders[shaderNumber]);

                //Delete The Detached Shaders
                pShaders[shaderNumber] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gShaderProgramObject);
        gShaderProgramObject = 0;
        glUseProgram(0);
    }
	GLXContext currentGLXContext = glXGetCurrentContext();
	if(currentGLXContext != NULL && currentGLXContext == gGLXContext)
	{
		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if(gGLXFBConfig)
	{
		XFree(gGLXFBConfig);
	}

	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay, gGLXContext);
	}
    if(gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
    }

    if(gColormap)
    {
        XFreeColormap(gpDisplay, gColormap);
    }

    if(gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }

    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }
}
