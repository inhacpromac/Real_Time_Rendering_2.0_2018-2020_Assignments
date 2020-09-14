package com.inhacpromac.shapes_on_graph;

//added by inhacpromac
//for context
import android.content.Context;


//for such a view which will give OpenGL supported surface
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;


//for touchevent
import android.view.MotionEvent;
//for gesture event
import android.view.GestureDetector;
//for gesture event's OnGestureListener
import android.view.GestureDetector.OnGestureListener;
//for gesture event's OnDoubleTapListener
import android.view.GestureDetector.OnDoubleTapListener;


//for basic feature of OpenGLES
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;


//for OpenGL Buffers
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;


//for Matrix math
import android.opengl.Matrix;

//for Math
import java.lang.Math;

//implements OnGestureListener, OnDoubleTapListener :means
//all events in this class are manditory to use even though it is empty
//implementing inner class of GLSurfaceView, Renderer
public class GlesView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener {

	private final Context context;
    private GestureDetector gestureDetector;
	
	//java does not have Uint nor GLuint
	private int vertexShaderObject;
    private int fragmentShaderObject;
    private int shaderProgramObject;
	
	//java does not have address oprator hence we give array of 1 and pass it's name as address
    private int[] vao_graph = new int[1];
    private int[] vbo_graph_position = new int[1];
    private int[] vbo_graph_color = new int[1];
	private int[] vao_triangle = new int[1];
    private int[] vbo_position_triangle = new int[1];
    private int[] vbo_color_triangle = new int[1];
	private int[] vao_square = new int[1];
    private int[] vbo_position_square = new int[1];
    private int[] vbo_color_square = new int[1];
	private int[] vao_circle = new int[1];
    private int[] vbo_circle_position = new int[1];
    private int[] vbo_circle_color = new int[1];
	private int mvpUniform;
	
	private int numberOfCircleLines = 6000;
	private int numberOfCircleVertices = numberOfCircleLines * 2 * 3;
	private int numberOfTriangleVertices = 3 * 3;
	private int numberOfSquareVertices = 4 * 3;
	private int numberOfGraphVertices = 1024; //(21lines*2ways*3(x,y,z))+2 = 1024
	private int giNumberOfGraphLines = 0;
	private int giTotalNumberOfLines = 0;

    private float[] perspectiveProjectionMatrix = new float[16];//4X4 matrix

    public GlesView(Context drawingContext) {

        super(drawingContext);
		
		context = drawingContext;
		
		setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(drawingContext, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {

        //this is most important line but not in use under openGL programming.
        //this line is for keyboard gesture.
        int eventAction = event.getAction();

        if (!gestureDetector.onTouchEvent(event)) {
            super.onTouchEvent(event);
        }

        return true;
    }

    //OnDoubleTapListener
    @Override
    public boolean onDoubleTap(MotionEvent event) {
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent event) {
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent event) {
        return true;
    }

    //OnGestureListener
    @Override
    public boolean onDown(MotionEvent event) {
        return true;
    }

    @Override
    public boolean onFling(MotionEvent event1, MotionEvent event2, float velocityX, float velocityY) {
        return true;
    }

    @Override
    public void onLongPress(MotionEvent event) {
		
    }

    @Override
    public boolean onScroll(MotionEvent event1, MotionEvent event2, float distanceX, float distanceY) {
		uninitialize();
        System.exit(0);
        return true;
    }

    @Override
    public void onShowPress(MotionEvent event) {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent event) {
        return true;
    }
	
	//Implement GLSurfaceView.Renderer methods
	@Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        String version = gl.glGetString(GL10.GL_VERSION);
		System.out.println("INHACPROMAC_OpenGL_version: " + version);
		
		String glslVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);
        System.out.println("INHACPROMAC_GLSL_version: " + glslVersion);
		
		String vendor = gl.glGetString(GL10.GL_VENDOR);
		System.out.println("INHACPROMAC_OpenGL_Vendor: " + vendor);
		
		String renderer = gl.glGetString(GL10.GL_RENDERER);
		System.out.println("INHACPROMAC_OpenGL_Renderer: " + renderer);

        initialize();
    }
	//ususally don't use OpenGL fuctions is resemble by GL10 unused..
    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height) {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 unused) {
        display();
    }
	
	//Our custom methods
	private void initialize() {
		
		///////////////////////////////////////////////////////////////////////////////////////////////Vertex Shader
		//Define Vertex Shader Object
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
		//Write Vertex Shader Object
		final String vertexShaderSourceCode = String.format
		(	
			"#version 320 es" +
			"\n" +
			"in vec4 vPosition;" +
			"in vec4 vColor;" +
			"uniform mat4 u_mvp_matrix;" +
			"out vec4 out_color;" +
			"void main(void)" +
			"{" +
			"   gl_Position = u_mvp_matrix * vPosition;" +
			"	out_color = vColor;" +
			"}"
		);
		//Specifying Above code to the VertexShaderObject
		GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
		//Compile the Vertex Shader
		GLES32.glCompileShader(vertexShaderObject);
		//Error Checking for Vertex Shader Compiling
		int[]  iShaderCompileStatus = new int[1];
		int[]  iInfoLogLength = new int[1];
		String szInfoLog = null;
		GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus, 0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if (iInfoLogLength[0] > 0)
			{
				szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
				System.out.println("INHACPROMAC | Vertex Shader Compile Log: " + szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////Fragment Shader
		//Define Fragment Shader Object
		fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
		//Write Fragment Shader Object
		final String fragmentShaderSourceCode = String.format
		(
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"in vec4 out_color;" +
			"out vec4 FragColor;" +
			"void main(void)" +
			"{" +
			"	FragColor = out_color;" +
			"}"
		);
		//Specifying Above code to the FragmentShaderObject
		GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);
		//Compile the fragment Shader
		GLES32.glCompileShader(fragmentShaderObject);
		//Error Checking for Fragment Shader Compiling
		iShaderCompileStatus[0] = 0;
		iInfoLogLength[0] = 0;
		szInfoLog = null;
		GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus, 0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if (iInfoLogLength[0] > 0)
			{
				szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
				System.out.println("INHACPROMAC | Fragment Shader Compile Log: " + szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////Shader Program
		//Create Shader Program Object
		shaderProgramObject = GLES32.glCreateProgram();
		//Attach Vertex Shader to Shader Program
		GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
		//Attach Fragment Shader to Shader Program
		GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);
		//preLinking Binding to Vertex Attributes
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");
		//Link The Shader Program
		GLES32.glLinkProgram(shaderProgramObject);
		//Error Checking for Shader Program Linking
		int[] iProgramLinkStatus = new int[1];
		iInfoLogLength[0] = 0;
		szInfoLog = null;
		GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, iProgramLinkStatus, 0);
		if (iProgramLinkStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if (iInfoLogLength[0] > 0)
			{
				GLES32.glGetProgramInfoLog(shaderProgramObject);
				System.out.println("INHACPROMAC | Shader Program Link Log: " + szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		//postLinking retriving uniform location
		mvpUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
		///////////////////////////////////////////////////////////////////////////////////////////////		
		// Graph
		final float[] graphVertices = new float[numberOfGraphVertices];
		final float[] graphColor = new float[numberOfGraphVertices];
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
			graphVertices[iArrayIndex++] = -1.0f;
			graphVertices[iArrayIndex++] = 0.0f;

			graphColor[iArrayIndex] = 0;
			graphColor[iArrayIndex + 1] = 0;
			graphColor[iArrayIndex + 2] = 1;

			graphVertices[iArrayIndex++] = fIterator;
			graphVertices[iArrayIndex++] = 1.0f;
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
		final float[] triangleVertices = new float[]
		{
			0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f
		};

		final float[] triangleColor = new float[]
		{
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};

		// Square
		final float[] squareVertices = new float[]
		{
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};

		final float[] squareColor = new float[]
		{
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};
		
		GLES32.glLineWidth(2.0f);
		
		// Circle
		final float[] circleVertices = new float[numberOfCircleVertices];
		final float[] circleColor = new float[numberOfCircleVertices];

		iArrayIndex = 0;
		for (int iIterator = 0; iIterator < numberOfCircleLines; iIterator++)
		{
			circleColor[iArrayIndex] = 1.0f;
			circleColor[iArrayIndex + 1] = 1.0f;
			circleColor[iArrayIndex + 2] = 0.0f;
			circleVertices[iArrayIndex++] = (float)Math.cos((5.0f * 3.14f * iIterator) / numberOfCircleLines);
			circleVertices[iArrayIndex++] = (float)Math.sin((5.0f * 3.14f * iIterator) / numberOfCircleLines);
			circleVertices[iArrayIndex++] = 0.0f;

			iIterator++;

			circleColor[iArrayIndex] = 1.0f;
			circleColor[iArrayIndex + 1] = 1.0f;
			circleColor[iArrayIndex + 2] = 0.0f;
			circleVertices[iArrayIndex++] = (float)Math.cos((5.0f * 3.14f * iIterator) / numberOfCircleLines);
			circleVertices[iArrayIndex++] = (float)Math.sin((5.0f * 3.14f * iIterator) / numberOfCircleLines);
			circleVertices[iArrayIndex++] = 0.0f;
		}

		//Create vao_graph
		GLES32.glGenVertexArrays(1, vao_graph, 0);
		GLES32.glBindVertexArray(vao_graph[0]);

		GLES32.glGenBuffers(1, vbo_graph_position, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_graph_position[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(graphVertices.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        FloatBuffer positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(graphVertices);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, graphVertices.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glGenBuffers(1, vbo_graph_color, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_graph_color[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(graphColor.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(graphColor);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, graphColor.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glBindVertexArray(0);
		
		//Create vao_triangle
		GLES32.glGenVertexArrays(1, vao_triangle, 0);
		GLES32.glBindVertexArray(vao_triangle[0]);

		GLES32.glGenBuffers(1, vbo_position_triangle, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_triangle[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(triangleVertices.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(triangleVertices);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, triangleVertices.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glGenBuffers(1, vbo_color_triangle, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_color_triangle[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(triangleColor.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(triangleColor);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, triangleColor.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glBindVertexArray(0);
		
		//Create vao_square
		GLES32.glGenVertexArrays(1, vao_square, 0);
		GLES32.glBindVertexArray(vao_square[0]);

		GLES32.glGenBuffers(1, vbo_position_square, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_square[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(squareVertices.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(squareVertices);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, squareVertices.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glGenBuffers(1, vbo_color_square, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_color_square[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(squareColor.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(squareColor);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, squareColor.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glBindVertexArray(0);

		//Create vao_circle
		GLES32.glGenVertexArrays(1, vao_circle, 0);
		GLES32.glBindVertexArray(vao_circle[0]);

		GLES32.glGenBuffers(1, vbo_circle_position, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_circle_position[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(circleVertices.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(circleVertices);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, circleVertices.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glGenBuffers(1, vbo_circle_color, 0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_circle_color[0]);
		
		//Convert the array into compatible buffer such the we can pass it through to BufferData
		
		//1: Allocate the buffer directly from the native memory(Not from VM Memeory(Unmanaged memory)).
		byteBuffer = ByteBuffer.allocateDirect(circleColor.length * 4);//(ArrayLength*DatatypeSize)
		//2: Arrange the byte order of buffer in native byte order.
        byteBuffer.order(ByteOrder.nativeOrder());
		//3: Create the float type buffer and Convert our ByteBuffer in FloatBuffer.
        positionBuffer = byteBuffer.asFloatBuffer();
		//4: Now, put your array into "cooked buffer".
        positionBuffer.put(circleColor);
		//5: Set the array at "0TH" position of the buffer.
        positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, circleColor.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		
		GLES32.glBindVertexArray(0);
		
		GLES32.glEnable(GLES32.GL_DEPTH_TEST);
		GLES32.glDepthFunc(GLES32.GL_LEQUAL);

		GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		Matrix.setIdentityM(perspectiveProjectionMatrix,0);
    }
	
	private void resize(int width, int height) {
		if (height == 0)
		{
			height = 1;
		}
        GLES32.glViewport(0, 0, width, height);
		Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, (float)width / (float)height, 0.1f, 100.0f);
    }

    private void display() {
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);

		//Declaration of Matrices
        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];

		//Initialize above Matrices to identity
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		
		//Do necessary Trasformation
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.0f);

		//Do necessary Matrix Multiplication
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

		//Send necessary Matrix to Shader in respective Uniform
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        //Bind with vao
        GLES32.glBindVertexArray(vao_graph[0]);
		
		//simillar Bind with Texture if any

        //Draw necessary Scence
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, giNumberOfGraphLines);

        //Unbind with vao
        GLES32.glBindVertexArray(0);
		
		//Initialize above Matrices to identity
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		
		//Do necessary Trasformation
		Matrix.translateM(modelViewMatrix, 0, -0.5f, 0.5f, -3.0f);
		Matrix.scaleM(modelViewMatrix, 0, 0.5f, 0.5f, 0.0f);

		//Do necessary Matrix Multiplication
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

		//Send necessary Matrix to Shader in respective Uniform
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        //Bind with vao
        GLES32.glBindVertexArray(vao_triangle[0]);
		
		//simillar Bind with Texture if any

        //Draw necessary Scence
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 3);

        //Unbind with vao
        GLES32.glBindVertexArray(0);
		
		//Initialize above Matrices to identity
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		
		//Do necessary Trasformation
		Matrix.translateM(modelViewMatrix, 0, 0.5f, 0.5f, -3.0f);
		Matrix.scaleM(modelViewMatrix, 0, 0.5f, 0.5f, 0.0f);

		//Do necessary Matrix Multiplication
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

		//Send necessary Matrix to Shader in respective Uniform
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        //Bind with vao
        GLES32.glBindVertexArray(vao_square[0]);
		
		//simillar Bind with Texture if any

        //Draw necessary Scence
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);

        //Unbind with vao
        GLES32.glBindVertexArray(0);
		
		//Initialize above Matrices to identity
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		
		//Do necessary Trasformation
		Matrix.translateM(modelViewMatrix, 0, 0.0f, -0.5f, -3.0f);
		Matrix.scaleM(modelViewMatrix, 0, 0.5f, 0.5f, 0.5f);

		//Do necessary Matrix Multiplication
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

		//Send necessary Matrix to Shader in respective Uniform
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        //Bind with vao
        GLES32.glBindVertexArray(vao_circle[0]);
		
		//simillar Bind with Texture if any

        //Draw necessary Scence
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, numberOfCircleLines);

        //Unbind with vao
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);
		
        requestRender();
    }
	
	private void uninitialize() {
		
		if(vbo_circle_color[0] != 0) 
		{
            GLES32.glDeleteVertexArrays(1, vbo_circle_color, 0);
            vbo_circle_color[0] = 0;
        }
		
		if(vbo_circle_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_circle_position, 0);
            vbo_circle_position[0] = 0;
        }
		
		if(vao_circle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vao_circle, 0);
            vao_circle[0] = 0;
        }
		
		if(vbo_color_square[0] != 0) 
		{
            GLES32.glDeleteVertexArrays(1, vbo_color_square, 0);
            vbo_color_square[0] = 0;
        }
		
		if(vbo_position_square[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_position_square, 0);
            vbo_position_square[0] = 0;
        }
		
		if(vao_square[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vao_square, 0);
            vao_square[0] = 0;
        }
		
		if(vbo_color_triangle[0] != 0) 
		{
            GLES32.glDeleteVertexArrays(1, vbo_color_triangle, 0);
            vbo_color_triangle[0] = 0;
        }
		
		if(vbo_position_triangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_position_triangle, 0);
            vbo_position_triangle[0] = 0;
        }
		
		if(vao_triangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vao_triangle, 0);
            vao_triangle[0] = 0;
        }
		
		if(vbo_graph_color[0] != 0) 
		{
            GLES32.glDeleteVertexArrays(1, vbo_graph_color, 0);
            vbo_graph_color[0] = 0;
        }
		
		if(vbo_graph_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_graph_position, 0);
            vbo_graph_position[0] = 0;
        }
		
		if(vao_graph[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vao_graph, 0);
            vao_graph[0] = 0;
        }
		
		if (shaderProgramObject != 0)
		{
			int[] shaderCount= new int[1];
			int shaderNumber;
			GLES32.glUseProgram(shaderProgramObject);

			//Ask Program: How many Programs are attched to you?
			GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
			int[] shaders = new int[shaderCount[0]];
			GLES32.glGetAttachedShaders(shaderProgramObject, shaderCount[0], shaderCount, 0, shaders, 0);
			for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
			{
				//Detach everyone
				GLES32.glDetachShader(shaderProgramObject, shaders[shaderNumber]);

				//Delete The Detached Shaders
				shaders[shaderNumber] = 0;
			}
			GLES32.glDeleteProgram(shaderProgramObject);
			shaderProgramObject = 0;
			GLES32.glUseProgram(0);
		}
    }
}