#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include <ctime>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[6];
GLint gIntensityLoc;
float gIntensity = 1000;
int gWidth=1000 , gHeight=800;
int textY = 0.95 * gHeight;
//Custom global variables
int bunny_size,cube_size,quad_size;
float initSpeed = 0.04f;
float speed = initSpeed;
float initObstacleSpeed = 0.16f;
float obstacleSpeed = initObstacleSpeed;
float ypos = -3.5f;
float initJumpSpeed = 0.04f;
float jumpspeed = initJumpSpeed;
bool movement = false;
bool restart = true;
float initialObstacleLocation = -80.0f;
float currentObstacleLocation = initialObstacleLocation;
float bunny_position = 0.0f;
int selectedIndex = 0;  
int score = 0;
bool hit = false;
bool gameOver = false;
bool happy = false;
float angle = 0;
float initRotateSpeed = 4;
float rotateSpeed = initRotateSpeed;
float angleRad = 0;
int skip = -1;
struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

void Restart(){
    std::srand(std::time(0));

    // Generate a random number between 0 and 2
    int randomNumber = std::rand() % 3;
    selectedIndex = randomNumber;
    std::cout << "Random number between 0 and 2: " << randomNumber << std::endl;
    restart = false;
    hit = false;
    skip = -1;
    currentObstacleLocation = initialObstacleLocation;
    speed = initSpeed;
    obstacleSpeed = initObstacleSpeed;
    rotateSpeed = initRotateSpeed;
    jumpspeed = initJumpSpeed;
    ypos = -3.5;
    score = 0;
    bunny_position = 0.0f;

}

bool ParseObj(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);
    int size = 0;
    if(gVertices.size()!= 0){
        size = gVertices.size();
    }
    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] += size -1;
						nIndex[c] += size -1;
						tIndex[c] += size -1;
					}

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x, 
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x, 
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x, 
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices.size() == gNormals.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();
    gProgram[3] = glCreateProgram();
    gProgram[4] = glCreateProgram();
    gProgram[5] = glCreateProgram();

    createVS(gProgram[0], "vert0.glsl");
    createFS(gProgram[0], "frag0.glsl");

    createVS(gProgram[1], "vert1.glsl");
    createFS(gProgram[1], "frag1.glsl");

    createVS(gProgram[2], "vert_text.glsl");
    createFS(gProgram[2], "frag_text.glsl");

    createVS(gProgram[3], "bunny.glsl");
    createFS(gProgram[3], "bunnyFrag.glsl");

    createVS(gProgram[4], "cubeYellow.glsl");
    createFS(gProgram[4], "cubeYellowShader.glsl");

    createVS(gProgram[5], "cubeRed.glsl");
    createFS(gProgram[5], "cubeRedShader.glsl");

    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "inVertex");
    glBindAttribLocation(gProgram[1], 1, "inNormal");
    glBindAttribLocation(gProgram[2], 2, "vertex");
    glBindAttribLocation(gProgram[3], 0, "inVertex");
    glBindAttribLocation(gProgram[3], 1, "inNormal");
    glBindAttribLocation(gProgram[4], 0, "inVertex");
    glBindAttribLocation(gProgram[4], 1, "inNormal");
    glBindAttribLocation(gProgram[5], 0, "inVertex");
    glBindAttribLocation(gProgram[5], 1, "inNormal");

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glLinkProgram(gProgram[3]);
    glLinkProgram(gProgram[4]);
    glLinkProgram(gProgram[5]);
    glUseProgram(gProgram[3]);

    gIntensityLoc = glGetUniformLocation(gProgram[3], "intensity");
    cout << "gIntensityLoc = " << gIntensityLoc << endl;
    glUniform1f(gIntensityLoc, gIntensity);

    glUseProgram(gProgram[0]);

    gIntensityLoc = glGetUniformLocation(gProgram[0], "intensity");
    cout << "gIntensityLoc = " << gIntensityLoc << endl;
    glUniform1f(gIntensityLoc, gIntensity);
}

void initVBO()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [gVertices.size() * 3];
    GLfloat* normalData = new GLfloat [gNormals.size() * 3];
    GLuint* indexData = new GLuint [gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3*i] = gVertices[i].x;
        vertexData[3*i+1] = gVertices[i].y;
        vertexData[3*i+2] = gVertices[i].z;

        minX = std::min(minX, gVertices[i].x);
        maxX = std::max(maxX, gVertices[i].x);
        minY = std::min(minY, gVertices[i].y);
        maxY = std::max(maxY, gVertices[i].y);
        minZ = std::min(minZ, gVertices[i].z);
        maxZ = std::max(maxZ, gVertices[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3*i] = gNormals[i].x;
        normalData[3*i+1] = gNormals[i].y;
        normalData[3*i+2] = gNormals[i].z;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3*i] = gFaces[i].vIndex[0];
        indexData[3*i+1] = gFaces[i].vIndex[1];
        indexData[3*i+2] = gFaces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init() 
{
	//ParseObj("armadillo.obj");
	ParseObj("bunny.obj");
    bunny_size = gFaces.size();
    printf("bunny size %d\n", gFaces.size());
    ParseObj("cube.obj");
    printf("gfaces size after cube %d %d %d \n",gFaces.size(),bunny_size,gFaces.size()-bunny_size);
    cube_size = gFaces.size() - bunny_size;
    ParseObj("quad.obj");
    quad_size = gFaces.size() - bunny_size - cube_size;
	//ParseObj("bunny.obj");
    printf("bunny size %d cube size %d quad size %d face total size %d \n",bunny_size,cube_size,quad_size,gFaces.size());


    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(gWidth, gHeight);
    initVBO();
}

void drawModel()
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, bunny_size*3, GL_UNSIGNED_INT, 0);
}

void drawCube()
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, cube_size*3, GL_UNSIGNED_INT, (GLvoid *)(sizeof(GLuint) * bunny_size * 3));
}

void drawQuad()
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, quad_size * 3, GL_UNSIGNED_INT, (GLvoid *)(sizeof(GLuint) * (bunny_size + cube_size) * 3));
}



void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(!gameOver){
        ypos += jumpspeed;
        if(ypos > -2.1){
            jumpspeed = -jumpspeed;
        }
        if(ypos < -3.5){
            jumpspeed = -jumpspeed;
        }

        // Draw the bunny
        // Compute the modeling matrix 
        if(movement){
            bunny_position += speed;
        }
        if(bunny_position<-4){
            bunny_position= -4;
        }
        if(bunny_position>4){
            bunny_position = 4;
        }
        currentObstacleLocation += obstacleSpeed;

        if(currentObstacleLocation > -7.0){
            currentObstacleLocation = initialObstacleLocation;
            restart = true;
            
        }
        if(restart){
            std::srand(std::time(0));

            // Generate a random number between 0 and 2
            int randomNumber = std::rand() % 3;
            selectedIndex = randomNumber;
            std::cout << "Random number between 0 and 2: " << randomNumber << std::endl;
            restart = false;
            hit = false;
            skip = -1;
        
        }
        
        
    
        
        if(currentObstacleLocation > -12.0 && (bunny_position > -0.9 && bunny_position < 0.9)){
            //set game over in else
            
            if(!hit){
                if(selectedIndex == 1){
                    score += 1000;
                    printf("SCORED\n");
                    hit = true;
                    happy = true;
                    skip = 1;
                }
                else{
                    gameOver = true;
                    skip = 1;
                }
                    
            }
            
        }
        if(currentObstacleLocation > -12.0 && (bunny_position > -4.4 && bunny_position < -2.6)){
            //set game over in else
            printf("left obstacle hit\n");
            if(!hit){
                if(selectedIndex == 0){
                    score += 1000;
                    printf("SCORED\n");
                    hit = true;
                    happy = true;
                    skip = 0;
                }
                else{
                    gameOver = true;
                    skip = 0;
                }
                    
            }
        }
        if(currentObstacleLocation > -12.0 && (bunny_position > 2.6 && bunny_position < 4.4)){
            //set game over in else
            printf("right obstacle hit\n");
            if(!hit){
                if(selectedIndex == 2){
                    score += 1000;
                    printf("SCORED\n");
                    hit = true;
                    happy = true;
                    skip = 2;
                }
                else{
                    gameOver = true;
                    skip = 2;
                }
                    
            }
        }
        glUseProgram(gProgram[3]);
        //glLoadIdentity();
        //glTranslatef(-2, 0, -10);
        //glRotatef(angle, 0, 1, 0);
        if(happy){
            //rotate
            angle += rotateSpeed;
            if(angle >= 360){
                angle = 0;
                happy = false;

            }
            printf("angle right now is %f \n", angle);
            
        }
        angleRad = (float)(angle / 180.0) * M_PI;
        glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(bunny_position, ypos, -10.f));
        glm::mat4 R = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.3, 0.5, 0.3)); 
        glm::mat4 modelMat = T * matRz * R * matS;
        glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
        glm::mat4 perspMat = glm::perspective(glm::radians(45.0f), 1.f, 1.0f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
        drawModel();

        
        //glLoadIdentity();
        //glTranslatef(2, 0, -10);
        //glRotatef(-angle, 0, 1, 0);
        for(int x=0;x<3;x++){
            if(x == skip){
                continue;
            }
            glUseProgram(gProgram[5]);
            if(x == selectedIndex){
                glUseProgram(gProgram[4]);
            }
            
            T = translate(glm::mat4(1.0), glm::vec3(-3.5f + 3.5*x, -2.5f, currentObstacleLocation));
            R = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
            matS = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 1, 0.5)); 
            modelMat = T * R * matS;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            if(x == selectedIndex){
                glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
            }
            else{
                glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
        
            }
            
            drawCube();
        }
        

        glUseProgram(gProgram[1]);
        for(int j=0;j<15;j++){

            for (int i = 0; i < 4; i++) {
                // Calculate the translation for each quad
                T = glm::translate(glm::mat4(1.0), glm::vec3(-3.7f + i * 2.5f, -3.7f, -10.f + j * (-8.0f)));

                // Apply scaling if needed - adjust the values as necessary
                matS = glm::scale(glm::mat4(1.0), glm::vec3(1.25, 4.0, 1.0)); // Adjust X, Y, and Z scaling if needed

                // Apply rotation if needed - since we want the quads to face the camera, we don't rotate them
                R = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));

                // Combine transformations
                modelMat = T * R * matS;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                // Set the uniform variables for the shader
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                // Draw the quad
                glDrawElements(GL_TRIANGLES, quad_size * 3, GL_UNSIGNED_INT, (GLvoid *)(sizeof(GLuint) * (bunny_size + cube_size) * 3));
            }
        }
        jumpspeed += 0.0002;
        obstacleSpeed += 0.0003;
        if(speed < 0){
            speed -= 0.0002;
        }
        else{
            speed += 0.0002;
        }
        
        score += (int) (abs(speed) * 100);
        rotateSpeed += 0.0002;

        assert(glGetError() == GL_NO_ERROR);
    }
    else{
        glUseProgram(gProgram[3]);
        glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(bunny_position, ypos, -10.f));
        glm::mat4 R = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(-0.25, 1.0, 1.5));
        glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.3, 0.5, 0.3)); 
        glm::mat4 modelMat = T * matRz * R * matS;
        glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
        glm::mat4 perspMat = glm::perspective(glm::radians(45.0f), 1.f, 1.0f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
        drawModel();


        for(int x=0;x<3;x++){
            if(x == skip){
                continue;
            }
            glUseProgram(gProgram[5]);
            if(x == selectedIndex){
                glUseProgram(gProgram[4]);
            }
            
            T = translate(glm::mat4(1.0), glm::vec3(-3.5f + 3.5*x, -2.5f, currentObstacleLocation));
            R = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
            matS = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 1, 0.5)); 
            modelMat = T * R * matS;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            if(x == selectedIndex){
                glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
            }
            else{
                glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
        
            }
            
            drawCube();
        }
        
        glUseProgram(gProgram[1]);
        for(int j=0;j<15;j++){

            for (int i = 0; i < 4; i++) {
                // Calculate the translation for each quad
                T = glm::translate(glm::mat4(1.0), glm::vec3(-3.7f + i * 2.5f, -3.7f, -10.f + j * (-8.0f)));

                // Apply scaling if needed - adjust the values as necessary
                matS = glm::scale(glm::mat4(1.0), glm::vec3(1.25, 4.0, 1.0)); // Adjust X, Y, and Z scaling if needed

                // Apply rotation if needed - since we want the quads to face the camera, we don't rotate them
                R = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));

                // Combine transformations
                modelMat = T * R * matS;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                // Set the uniform variables for the shader
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                // Draw the quad
                glDrawElements(GL_TRIANGLES, quad_size * 3, GL_UNSIGNED_INT, (GLvoid *)(sizeof(GLuint) * (bunny_size + cube_size) * 3));
            }
        }
    }
    
    
    string text = "Score: ";
    text += to_string(score);
    if(!gameOver){
        renderText(text, 0, textY, 1, glm::vec3(0, 1, 1));
    }
    else{
        renderText(text, 0, textY, 1, glm::vec3(1, 0, 0));
    }
    

    assert(glGetError() == GL_NO_ERROR);

}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;
    textY = 0.95 * gHeight;
    glViewport(0, 0, w, h);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if(!gameOver){
            switch (key) {
                case GLFW_KEY_A:
                    if(speed>0){
                        speed = -speed; // Move left
                    }
                    movement = true;
                    break;
                case GLFW_KEY_D:
                    if(speed<0){
                            speed = -speed; // Move left
                        }
                    movement = true; // Move right
                    break;
            }
        }
    }
    else if(key == GLFW_KEY_R && (action == GLFW_RELEASE || action == GLFW_PRESS || action == GLFW_REPEAT)){
        printf("R pressed \n");
		gameOver = false;
        Restart();
	}
    else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        cout << "F pressed" << endl;
        glUseProgram(gProgram[1]);
    }
    else if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        cout << "V pressed" << endl;
        glUseProgram(gProgram[0]);
    }
    else if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        cout << "X pressed" << endl;
        gIntensity /= 1.5;
        cout << "gIntensity = " << gIntensity << endl;
        glUseProgram(gProgram[0]);
        glUniform1f(gIntensityLoc, gIntensity);
    }
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
    else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
    else if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        cout << "B pressed" << endl;
        gIntensity *= 1.5;
        cout << "gIntensity = " << gIntensity << endl;
        glUseProgram(gProgram[0]);
        glUniform1f(gIntensityLoc, gIntensity);
    }
    else if(key == GLFW_KEY_A && action == GLFW_RELEASE){
		movement = false;
	}
	else if(key == GLFW_KEY_D && action == GLFW_RELEASE){
		movement = false;
	}
    

}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

