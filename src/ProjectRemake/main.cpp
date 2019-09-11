//점수판 구현
//아이템 구현 공여러개,화면회전,
//무적

//바운더리,바,벽돌,아이템   무적공,그냥공     isvelochange life

// 0 무한                
//한줄 지우기

//땅 닿으면 공 없애기, 게임오버
//속도 빠르면 충돌처리 이상해지는거, 그 후 의문의 조건으로 공 정지하는 것

//배경음악 재생, 시작할 때 버튼
//High score 데이터 저장

#define gridrow 20
#define gridcol 10
#define boundaryleft -0.9
#define boundaryright 0.9
#define boundaryup 0.93
#define boundarydown -0.93
#define interval 0.015
#define PI 3.141592
#define TIME_CONSTANT 100
#define FPS 60
#define WindowWidth 640
#define WindowHeight 680

#pragma warning(disable:4996)
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string>
#include "vec.h"
#include "gamedata.h"
#include <GL/glut.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib") 

using namespace std;
int score = 0;
int level = 1;
int initialcount = 0;
bool rotation = false;
bool unbeatablestate = false;
unsigned int itemdeleteline = 0;
unsigned int itemunbeatable = 0;
unsigned int itemmultiple = 0;
unsigned int deletedbricknum = 0;
bool gamestart = false;
Vec3f newballpos;

void funcitemdelete();
void funcunitembeatable();
void funcitemmultiple();
void score_update();
void* glutFonts[7] = {
	GLUT_BITMAP_9_BY_15,
	GLUT_BITMAP_8_BY_13,
	GLUT_BITMAP_TIMES_ROMAN_10,
	GLUT_BITMAP_TIMES_ROMAN_24,
	GLUT_BITMAP_HELVETICA_10,
	GLUT_BITMAP_HELVETICA_12,
	GLUT_BITMAP_HELVETICA_18
};

void glutPrint(float x, float y, void* font, char* text, float r, float g, float b, float a)
{
	if (!text || !strlen(text)) return;
	bool blending = false;
	if (glIsEnabled(GL_BLEND)) blending = true;
	glEnable(GL_BLEND);
	glColor4f(r, g, b, a);
	glRasterPos2f(x, y);

	while (*text) {
		glutBitmapCharacter(font, *text);
		text++;
	}
	if (!blending) glDisable(GL_BLEND);
}

template<typename T, typename S>
int find_index(T begin, T end, S val) {
	//return array index
	int index = 0;

	for (T it = begin; it != end; it++) {
		if (*it == val)
			return index;
		index++;
	}

}

template<typename T, typename F>
std::size_t count_if(T begin, T end, F& func) {
	//count if function using functor
	std::size_t num = 0;
	for (T it = begin; it != end; it++) {
		if (func(*it) == true)
			++num;
	}
	return num;
}

class Shape {
public:
	Shape() {
	}
	Shape(Shape& sha) {
		velocity = sha.velocity;
	}
	void setvelocity(float x,float y) {
		velocity[0] = x;
		velocity[1] = y;
		velocity[2] = 0;

	}
	virtual ~Shape() {

	}
	virtual void draw() {}
	
	virtual void move() {}

	Vec3f velocity;
};
vector<Vec3f> color_array;
vector<Vec3f> color_brick;
class Color {
public:
	Color() {
		rgb = color_array[0];
	}
	Color(Color& c) {
		this->rgb = c.rgb;
	}
	virtual ~Color() {

	}
	void setRGB(float r, float g, float b) {
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
	}
	void colorUpdate(int newindex) {
		rgb = color_array[newindex];
	}
	Vec3f rgb;
};


class Circle : public Shape, public Color {
public:
	Circle() {
		radius = 0.05;
		//velocity[0] = float(rand() % 10 - 5) / (FPS * 3);
		//velocity[1] = float(rand() % 10 - 5) / (FPS * 3);
		velocity[0] =0;
		velocity[1] =0;
	}
	Circle(Circle& cir) :Shape(cir), Color(cir) {
		this->radius = cir.radius;
		this->centerPos = cir.centerPos;
	}
	~Circle() {}
	void draw();
	void move(){
		centerPos += velocity;
	}
	void setPosition(float x, float y) {
		centerPos[0] = x;
		centerPos[1] = y;
	}
	Vec3f centerPos;
	float radius;
};


void Circle::draw() {
	glColor3f(rgb[0], rgb[1], rgb[2]);
	glBegin(GL_TRIANGLE_FAN);
	float step = 100;
	glVertex3f(centerPos[0], centerPos[1], 0);
	for (unsigned int i = 0; i<step + 1; i++) {
		Vec3f displacement;
		displacement.val[0] = radius*cos(2.0*PI / step*i);
		displacement.val[1] = radius*sin(2.0*PI / step*i);

		Vec3f point = centerPos + displacement;
		glVertex3f(point[0], point[1], 0);
	}
	glEnd();
}

class Rectan : public Shape, public Color {
public:
	Rectan();
	Rectan(Rectan& rect) :Shape(rect), Color(rect) {
		this->cornerPoint = rect.cornerPoint;
		this->length = rect.length;
	}
	~Rectan() {}
	void draw();
	void move() {
		cornerPoint += velocity;
	}
	void move(Vec3f m) {
		cornerPoint += m;
	}
	void setPosition(float x, float y) {
		cornerPoint[0] = x;
		cornerPoint[1] = y;
	}
	void setLength(float w, float h) {
		length[0] = w;
		length[1] = h;
	}
	Vec3f cornerPoint;
	Vec3f length;
	int isitem;
};

Rectan::Rectan() {
	//length[0] = 0.4;
	//length[1] = 0.02;

	length[0] =0.4;
	length[1] = 0.25;
	isitem = 0;
	rgb.val[0] = rand() % 10 / double(10);
	rgb.val[1] = rand() % 10 / double(10);
	rgb.val[2] = rand() % 10 / double(10);
}


void Rectan::draw() {

	if (isitem == 0) {
		glColor3f(rgb[0], rgb[1], rgb[2]);
		glBegin(GL_QUADS);

		glVertex3f(cornerPoint[0], cornerPoint[1], 0);
		glVertex3f(cornerPoint[0] + length[0], cornerPoint[1], 0);
		glVertex3f(cornerPoint[0] + length[0], cornerPoint[1] + length[1], 0);
		glVertex3f(cornerPoint[0], cornerPoint[1] + length[1], 0);

		glEnd();

		glColor3f(rgb[0] * 0.7, rgb[1] * 0.7, rgb[2] * 0.7);
		glBegin(GL_QUADS);

		glVertex3f(cornerPoint[0] + length[0] * 0.25, cornerPoint[1] + length[1] * 0.15, 0);
		glVertex3f(cornerPoint[0] + length[0] * 0.75, cornerPoint[1] + length[1] * 0.15, 0);
		glVertex3f(cornerPoint[0] + length[0] * 0.75, cornerPoint[1] + length[1] * 0.85, 0);
		glVertex3f(cornerPoint[0] + length[0] * 0.25, cornerPoint[1] + length[1] * 0.85, 0);
		glEnd();
	}
	else if (isitem == 1) {

		glBegin(GL_QUADS);

		glColor3f(0, 0.7, 0);
		glVertex3f(cornerPoint[0], cornerPoint[1], 0);

		glColor3f(0, 0.7, 0.6);
		glVertex3f(cornerPoint[0] + length[0], cornerPoint[1], 0);

		glColor3f(0.6, 0.7, 0);
		glVertex3f(cornerPoint[0] + length[0], cornerPoint[1] + length[1], 0);

		glColor3f(0.6, 0.7, 0);
		glVertex3f(cornerPoint[0], cornerPoint[1] + length[1], 0);
		glEnd();
	}
	else if (isitem == 2) {

		glBegin(GL_QUADS);

		glColor3f(1, 0, 0);
		glVertex3f(cornerPoint[0], cornerPoint[1], 0);

		glColor3f(1, 0.6, 0);
		glVertex3f(cornerPoint[0] + length[0], cornerPoint[1], 0);

		glColor3f(1, 0, 0.6);
		glVertex3f(cornerPoint[0] + length[0], cornerPoint[1] + length[1], 0);

		glColor3f(1, 0, 0.6);
		glVertex3f(cornerPoint[0], cornerPoint[1] + length[1], 0);
		glEnd();
	}
}
class compare_color {
public:
	compare_color(Vec3f b) : temp(b) {}
	bool operator()(Circle* x) {
		//implement compare_color functor
		return temp == (*x).rgb;
	}

private:
	Vec3f temp;
};

vector<Circle*> cir;
Rectan* rectofbar;
Circle* leftcirofbar;
Circle* rightcirofbar;
vector<Rectan*> bricks;
vector<Rectan*> boundaries;
vector<Rectan*> items;


void initialize() {
	
	{
	Vec3f color;
	color[0] = 0.7f; color[1] = 0.6f; color[2] = 0.4f;
	color_array.push_back(color);
	color[0] = 0.6f; color[1] = 0.4f; color[2] = 0.3f;
	color_array.push_back(color);
	color[0] = 0.3f; color[1] = 0.25f; color[2] = 0.25f;
	color_array.push_back(color);
	}
	
	{
		Vec3f color;
		color[0] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;
		color_brick.push_back(color);
		color[0] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;
		color_array.push_back(color);
		color[0] = 1.0f; color[1] = 0.5f; color[2] = 0.0f;
		color_array.push_back(color);
		color[0] = 1.0f; color[1] = 1.0f; color[2] = 0.0f;
		color_array.push_back(color);
		color[0] = 0.0f; color[1] = 0.0f; color[2] = 1.0f;
		color_array.push_back(color);
	}



}
void initShape() {
	rectofbar = new Rectan();
	rectofbar->setRGB(1.0f, 0.f, 0.f);
	rectofbar->setLength(0.45,0.05);
	rectofbar->setPosition(-0.2, -0.85);

	leftcirofbar = new Circle();
	leftcirofbar->setRGB(rectofbar->rgb[0], rectofbar->rgb[1], rectofbar->rgb[2]);
	leftcirofbar->radius = rectofbar->length[1]/2;
	leftcirofbar->setPosition(rectofbar->cornerPoint[0], rectofbar->cornerPoint[1]+rectofbar->length[1]/2);

	rightcirofbar = new Circle();
	rightcirofbar->setRGB(rectofbar->rgb[0], rectofbar->rgb[1], rectofbar->rgb[2]);
	rightcirofbar->radius = rectofbar->length[1] / 2;
	rightcirofbar->setPosition(rectofbar->cornerPoint[0]+rectofbar->length[0], rectofbar->cornerPoint[1] + rectofbar->length[1] / 2);

	int num = 1;
	for (int i = 0; i<num; i++) {
		Circle* shape = new Circle();
		shape->setPosition(rectofbar->cornerPoint[0] + rectofbar->length[0] / 2, rectofbar->cornerPoint[1] + rectofbar->length[1] + shape->radius);
		//shape->setPosition(leftcirofbar->centerPos[0] , rectofbar->cornerPoint[1] + rectofbar->length[1] + shape->radius);

		shape->setvelocity(0, 0);
		shape->setRGB(0, 0, 1);
		cir.push_back(shape);
		newballpos = shape->centerPos;
	}

	float gamewidth = boundaryright - boundaryleft;
	float gameheight = boundaryup - boundarydown;
	float depth = 0.005;
	
	Rectan *upboundary = new Rectan();
	upboundary->setLength(gamewidth, depth);
	upboundary->setPosition(boundaryleft,boundaryup);
	upboundary->setRGB(0,0,0);
	boundaries.push_back(upboundary);
	
	Rectan *downboundary = new Rectan();
	downboundary->setLength(gamewidth, depth);
	downboundary->setPosition(boundaryleft, boundarydown);
	downboundary->setRGB(0, 0, 0);
	boundaries.push_back(downboundary);
	
	Rectan *leftboundary = new Rectan();
	leftboundary->setLength(depth,gameheight);
	leftboundary->setPosition(boundaryleft,boundarydown);
	leftboundary->setRGB(0, 0, 0);
	boundaries.push_back(leftboundary);
	
	Rectan *rightboundary = new Rectan();
	rightboundary->setLength(depth, gameheight);
	rightboundary->setPosition(boundaryright, boundarydown);
	rightboundary->setRGB(0, 0, 0);
	boundaries.push_back(rightboundary);
	int degree;
	for (unsigned int i = 0; i < gridrow; i++) {
		for (int j = 0; j < gridcol; j++) {
			Rectan* a = new Rectan();
			float newwidth = (boundaryright - boundaryleft)/gridcol;
			float newheight = (boundaryup - boundarydown)/gridrow;

			a->setPosition(boundaryleft+newwidth*j,boundaryup-newheight*(i+1));
			a->setLength(newwidth - interval, newheight - interval);

			{
				switch (level)
				{
				case 1:
					degree = level1[i][j];
					break;
				case 2:
					degree = level2[i][j];
					break;
				case 3:
					degree = level3[i][j];
					break;
				case 4:
					degree = level4[i][j];
					break;
				case 5:
					degree = level5[i][j];
					break;
				case 6:
					degree = level6[i][j];
					break;
				case 7:
					degree = level7[i][j];
					break;
				case 8:
					degree = level8[i][j];
					break;
				case 9:
					degree = level9[i][j];
					break;
				case 10:
					degree = level10[i][j];
					break;
				default:
					degree = 11;
					break;
				}

				if (degree >= 1&&degree<=10) {
					a->setRGB(color_array[degree - 1][0], color_array[degree - 1][1], color_array[degree - 1][2]);
					bricks.push_back(a);
				}
				else if (degree == 0) {

				}
				else if (degree == 11)
					exit(0);
				
			}

		}
	}

	initialcount = bricks.size();

}

bool collisionDetection(Circle& c1, Circle& c2) {
	Vec3f differenceVector = c1.centerPos - c2.centerPos;

	float distance = sqrt(dotoperator(differenceVector, differenceVector));

	if (c1.radius + c2.radius > distance)
		return true;
	else
		return false;
}

void SpecialKey(int key, int x, int y)
{
	
	static Vec3f right_velo(0.08, 0, 0);
	static Vec3f left_velo(0.08, 0, 0);

	switch (key)
	{
	case GLUT_KEY_RIGHT:
		left_velo.setposition(0.08, 0, 0);
		right_velo = right_velo+Vec3f(0.02,0,0);

		rectofbar->cornerPoint += right_velo;
		break;
	case GLUT_KEY_LEFT:
		right_velo.setposition(0.08, 0, 0);
		left_velo = left_velo + Vec3f(0.02, 0, 0);

		rectofbar->cornerPoint -=left_velo;
		break;

	default:
		break;
	}

	if (rectofbar->cornerPoint[0] < boundaryleft + leftcirofbar->radius)
		rectofbar->cornerPoint[0] = boundaryleft + leftcirofbar->radius;
	if (rectofbar->cornerPoint[0] + rectofbar->length[0] > boundaryright - rightcirofbar->radius)
		rectofbar->cornerPoint[0] = boundaryright - rectofbar->length[0] - rightcirofbar->radius;

	leftcirofbar->centerPos[0] = rectofbar->cornerPoint[0];
	rightcirofbar->centerPos[0] = rectofbar->cornerPoint[0] + rectofbar->length[0];


	glutPostRedisplay();
}


void MyKeyboardFunc(unsigned char Key, int x, int y)
{
	switch (Key) {
		case 'Q':
		case 'q':
		{
			if (itemdeleteline > 0) {
				funcitemdelete();
				itemdeleteline--;
			}
			break;
		}
		case 'W':
		case 'w':
		{
			if (itemunbeatable > 0) {
				funcunitembeatable();
				itemunbeatable--;
			}
			break;
		}
		case 'E':
		case'e':
		{
			if (itemmultiple > 0) {
				funcitemmultiple();
				itemmultiple--;
			}
			break;
		}
	}
}
void processMouse(int x, int y)
{
	// buttons : GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON
	// GLUT_RIGHT_BUTTON
	// states : GLUT_DOWN, GLUT_UP
	//std::cout << x<<std::endl;
	float Xlimit = WindowWidth / 2.f;
	float Xcoord = x;
	rectofbar->cornerPoint[0] = (Xcoord - Xlimit) / Xlimit / boundaryright-rectofbar->length[0]/2;

	if (rectofbar->cornerPoint[0] < boundaryleft+leftcirofbar->radius)
		rectofbar->cornerPoint[0] = boundaryleft + leftcirofbar->radius;
	if (rectofbar->cornerPoint[0] + rectofbar->length[0] > boundaryright - rightcirofbar->radius)
		rectofbar->cornerPoint[0] = boundaryright - rectofbar->length[0] - rightcirofbar->radius;

	leftcirofbar->centerPos[0] = rectofbar->cornerPoint[0] ;
	rightcirofbar->centerPos[0] = rectofbar->cornerPoint[0] + rectofbar->length[0];
	glutPostRedisplay();
}

clock_t startTime=0;
clock_t preTime;
clock_t current;

void renderScene()
{
	static clock_t rotationtime;
	static bool isrotated = false;
	if (rotation) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glRotatef(180, 0, 1 , 0);
		if (isrotated == false) {
			rotationtime = clock();
			isrotated = true;
		}
		if(clock()>rotationtime+5000){
			rotation = false;
			isrotated = false;
		}
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	}

	static clock_t unbeatabletime;
	static bool isunbeatabled = false;

	if (unbeatablestate) {
		if (isunbeatabled == false) {
			unbeatabletime = clock();
			for (unsigned int i = 0; i<cir.size(); i++)
				cir[i]->rgb = color_array[3];
			isunbeatabled = true;
		}
		if (clock()>unbeatabletime + 5000) {
			unbeatablestate = false;
			for (unsigned int i = 0; i<cir.size(); i++)
				cir[i]->rgb = color_array[6];
			isunbeatabled = false;
		}
	}





	// Clear Color and Depth Buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (unsigned int i = 0; i < cir.size(); i++)
		cir[i]->draw();
	for (unsigned int i = 0; i < bricks.size(); i++)
		bricks[i]->draw();
	for (unsigned int i = 0; i < boundaries.size(); i++)
		boundaries[i]->draw();
	for (unsigned int i = 0; i < items.size(); i++)
		items[i]->draw();
	rectofbar->draw();
	leftcirofbar->draw();
	rightcirofbar->draw();
	
	if (rotation == false) {
		glutPrint(-0.9, 0.95, glutFonts[6], strdup(("Score: " + to_string(score)).c_str()), 1, 0, 1, 1);
		glutPrint(-0.1, 0.95, glutFonts[6], strdup(("Level: " + to_string(level)).c_str()), 0.0, 0, 1, 1);
		glutPrint(-0.9, -0.98, glutFonts[6], strdup(("DelteL(Q): " + to_string(itemdeleteline)).c_str()), 1, 0, 0, 1);
		glutPrint(-0.22, -0.98, glutFonts[6], strdup(("Unbeatable(W): " + to_string(itemunbeatable)).c_str()), 1, 0, 1, 1);
		glutPrint(0.6, -0.98, glutFonts[6], strdup(("Ball++(E): " + to_string(itemmultiple)).c_str()), 0, 0, 1, 1);
		if (gamestart) {
			float timestring = (current - startTime) / 1000;
			int sec = (current - startTime) / 1000;
			int microsec = ((current - startTime) % 1000) / 100;
			char *secstring = strdup(("Time Left: " + to_string(499-sec) + "." + to_string(9-microsec)).c_str());
			glutPrint(0.5, 0.955, glutFonts[6], secstring, 1, 0, 0, 1);
		}
	}
	else if (rotation == true) {
		glutPrint(0.9, 0.95, glutFonts[6], strdup(("Score: " + to_string(score)).c_str()), 1, 0, 1, 1);
		glutPrint(0.1, 0.95, glutFonts[6], strdup(("Level: " + to_string(level)).c_str()), 0.0, 0, 1, 1);
		glutPrint(0.9, -0.98, glutFonts[6], strdup(("DelteL(Q): " + to_string(itemdeleteline)).c_str()), 1, 0, 0, 1);
		glutPrint(0.22, -0.98, glutFonts[6], strdup(("Unbeatable(W): " + to_string(itemunbeatable)).c_str()), 1, 0, 1, 1);
		glutPrint(-0.6, -0.98, glutFonts[6], strdup(("Ball++(E): " + to_string(itemmultiple)).c_str()), 0, 0, 1, 1);
		if (gamestart) {
			float timestring = (current - startTime) / 1000;
			int sec = (current - startTime) / 1000;
			int microsec = ((current - startTime) % 1000) / 100;
			char *secstring = strdup(("Time Left: " + to_string(499 - sec) + "." + to_string(9 - microsec)).c_str());
			glutPrint(-0.5, 0.955, glutFonts[6], secstring, 1, 0, 0, 1);
		}
	}
	
	glutSwapBuffers();

}


void idle() {
	current = clock();
	clock_t diff = current - preTime;
	if (diff > 1000.0 / FPS) {

		for (unsigned int i = 0; i < cir.size(); i++) {
			if (cir[i]->velocity == Vec3f(0, 0, 0)) {
				if (rectofbar->cornerPoint[0] + rectofbar->length[0] / 2 != newballpos[0]) {
					gamestart = true;
					if (startTime == 0)
						startTime = clock();
					float angle = PI / 4 + PI / 10 * (rand() % 5);
					//float angle = PI / 2;

					cir[i]->setvelocity(0.025*cos(angle), 0.025*sin(angle));
				}
			}
		}


		for (unsigned int i = 0; i < cir.size(); i++) {
			cir[i]->move();
		}

		for (unsigned int i = 0; i < items.size(); i++) {
			items[i]->move();
		}


		//공과 공사이의 충돌
		for (unsigned int i = 0; i < cir.size(); i++) {
			for (unsigned int j = i + 1; j < cir.size(); j++) {
				if (collisionDetection(*cir[i], *cir[j])) {

					Vec3f vel_difference = cir[i]->velocity - cir[j]->velocity;
					Vec3f pos_difference = cir[i]->centerPos - cir[j]->centerPos;

					float cal = dotoperator(vel_difference, pos_difference) / dotoperator(pos_difference, pos_difference);

					cir[i]->velocity -= cal*pos_difference;
					cir[j]->velocity += cal*pos_difference;

					float distance = sqrt(dotoperator(pos_difference, pos_difference));
					float overlap = cir[i]->radius + cir[j]->radius - distance;
					Vec3f normalize = pos_difference / distance;
					cir[i]->centerPos += normalize*overlap / 2.0;
					cir[j]->centerPos -= normalize*overlap / 2.0;
					PlaySound(TEXT("bounce.wav"), NULL, SND_ASYNC);
				}
			}
		}
		//공과 벽돌사이의 충돌
		if (unbeatablestate == false) {
			for (unsigned int i = 0; i < cir.size(); i++) {
				for (unsigned int j = 0; j < bricks.size(); j++) {
					Rectan* rectobject = bricks[j];
					bool quickcheck = cir[i]->centerPos[1] < rectobject->cornerPoint[1] + rectobject->length[1] + cir[i]->radius && cir[i]->centerPos[1] > rectobject->cornerPoint[1] - cir[i]->radius;
					if (quickcheck) {

						Vec3f rectofbarcenter = rectobject->cornerPoint + Vec3f(rectobject->length[0] / 2, rectobject->length[1] / 2, 0);

						float deltax = (rectofbarcenter - cir[i]->centerPos)[0] / rectobject->length[0];
						float deltay = (rectofbarcenter - cir[i]->centerPos)[1] / rectobject->length[1];

						float slope = deltay / deltax;
						bool contacthorizontally = -1 < slope &&slope < 1;


						Vec3f distanceCorner1 = cir[i]->centerPos - rectobject->cornerPoint;
						Vec3f distanceCorner2 = distanceCorner1;
						distanceCorner2[0] -= rectobject->length[0];

						Vec3f distanceCorner3 = cir[i]->centerPos - (rectobject->cornerPoint + Vec3f(0, rectobject->length[1], 0));
						Vec3f distanceCorner4 = distanceCorner3;
						distanceCorner4[0] -= rectobject->length[0];
						float distance1 = sqrt(dotoperator(distanceCorner1, distanceCorner1));
						float distance2 = sqrt(dotoperator(distanceCorner2, distanceCorner2));

						float distance3 = sqrt(dotoperator(distanceCorner3, distanceCorner4));
						float distance4 = sqrt(dotoperator(distanceCorner3, distanceCorner4));

						bool mostly = cir[i]->centerPos[0] > rectobject->cornerPoint[0] && cir[i]->centerPos[0] < rectobject->cornerPoint[0] + rectobject->length[0];
						bool rarely = distance1 < cir[i]->radius || distance2 < cir[i]->radius || distance3 < cir[i]->radius || distance4 < cir[i]->radius;

						if (mostly || rarely) {
							if (contacthorizontally) {
								cir[i]->velocity[0] *= -1;
								if (cir[i]->centerPos[0] < rectofbarcenter[0])
									cir[i]->centerPos[0] = rectobject->cornerPoint[0] - cir[i]->radius;
								else
									cir[i]->centerPos[0] = rectobject->cornerPoint[0] + rectobject->length[0] + cir[i]->radius;
							}

							else {
								cir[i]->velocity[1] *= -1;
								if (cir[i]->centerPos[1] < rectofbarcenter[1])
									cir[i]->centerPos[1] = rectobject->cornerPoint[1] - cir[i]->radius;
								else
									cir[i]->centerPos[1] = rectobject->cornerPoint[1] + rectobject->length[1] + cir[i]->radius;
							}

							int index = find_index(color_array.begin(), color_array.end(), rectobject->rgb);
							if (0 < index)
								rectobject->colorUpdate(index - 1);
							else if (index == 0) {
								if (rand() % 100 > 66) {
									Rectan* a = new Rectan();

									a->setLength(0.1, 0.1);
									a->setPosition(rectobject->cornerPoint[0] + rectobject->length[0] / 2 - a->length[0] / 2, rectobject->cornerPoint[1]);
									a->setvelocity(0, -0.01);
									if (rand() % 100 > 50) { a->isitem = 1; }
									else a->isitem = 2;

									items.push_back(a);
								};
								Rectan* temppointer = *(bricks.begin() + j);
								bricks.erase(bricks.begin() + j);
								deletedbricknum++;
								delete temppointer;
								if (rotation == false)
									PlaySound(TEXT("bomb.wav"), NULL, SND_ASYNC);

							}
						}
					}
				}
			}
		}

		if (unbeatablestate == true) {
			for (unsigned int i = 0; i < cir.size(); i++) {
				for (unsigned int j = 0; j < bricks.size(); j++) {
					Rectan* rectobject = bricks[j];
					bool quickcheck = cir[i]->centerPos[1] < rectobject->cornerPoint[1] + rectobject->length[1] + cir[i]->radius && cir[i]->centerPos[1] > rectobject->cornerPoint[1] - cir[i]->radius;
					if (quickcheck) {

						Vec3f rectofbarcenter = rectobject->cornerPoint + Vec3f(rectobject->length[0] / 2, rectobject->length[1] / 2, 0);

						float deltax = (rectofbarcenter - cir[i]->centerPos)[0] / rectobject->length[0];
						float deltay = (rectofbarcenter - cir[i]->centerPos)[1] / rectobject->length[1];

						float slope = deltay / deltax;
						bool contacthorizontally = -1 < slope &&slope < 1;


						Vec3f distanceCorner1 = cir[i]->centerPos - rectobject->cornerPoint;
						Vec3f distanceCorner2 = distanceCorner1;
						distanceCorner2[0] -= rectobject->length[0];

						Vec3f distanceCorner3 = cir[i]->centerPos - (rectobject->cornerPoint + Vec3f(0, rectobject->length[1], 0));
						Vec3f distanceCorner4 = distanceCorner3;
						distanceCorner4[0] -= rectobject->length[0];
						float distance1 = sqrt(dotoperator(distanceCorner1, distanceCorner1));
						float distance2 = sqrt(dotoperator(distanceCorner2, distanceCorner2));

						float distance3 = sqrt(dotoperator(distanceCorner3, distanceCorner4));
						float distance4 = sqrt(dotoperator(distanceCorner3, distanceCorner4));

						bool mostly = cir[i]->centerPos[0] > rectobject->cornerPoint[0] && cir[i]->centerPos[0] < rectobject->cornerPoint[0] + rectobject->length[0];
						bool rarely = distance1 < cir[i]->radius || distance2 < cir[i]->radius || distance3 < cir[i]->radius || distance4 < cir[i]->radius;

						if (mostly || rarely) {
							Rectan* temppointer = *(bricks.begin() + j);
							bricks.erase(bricks.begin() + j);
							deletedbricknum++;
							delete temppointer;
							if (rotation == false)
								PlaySound(TEXT("bomb.wav"), NULL, SND_ASYNC);

						}
					}
				}
			}
		}


		//공과 아이템사이의 충돌

		for (unsigned int i = 0; i < items.size(); i++) {

			bool xcheck=rectofbar->cornerPoint[0] < items[i]->cornerPoint[0] && items[i]->cornerPoint[0] + items[i]->length[0] < rectofbar->cornerPoint[0] + rectofbar->length[0];
			bool ycheck = items[i]->cornerPoint[1] < rectofbar->cornerPoint[1] + rectofbar->length[1] && items[i]->cornerPoint[1] + items[i]->length[1] > rectofbar->cornerPoint[1] + rectofbar->length[1];;
			if (xcheck&&ycheck) {

				if (items[i]->isitem == 1) {
					int switchrand = rand() % 3;
					if ( switchrand== 0) {
						itemmultiple++;
					}
					else if (switchrand == 1) {
						itemunbeatable++;
					}
					else if (switchrand == 2) {
						itemdeleteline++;

					}
					if(rotation==false)
					PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
				}
				if (items[i]->isitem == 2) {
					rotation = true;
					PlaySound(TEXT("warning.wav"), NULL, SND_ASYNC);
					
				}
				delete *(items.begin()+i);
				items.erase(items.begin() + i);
			}
		}
			

		//공과 바운더리의 충돌
		for (unsigned int i = 0; i < cir.size(); i++) {
			if (cir[i]->centerPos[0] > boundaryright - cir[i]->radius) {
				cir[i]->velocity[0] *= -1;
				cir[i]->centerPos[0] = boundaryright - cir[i]->radius;
			}
			else if (cir[i]->centerPos[0] < boundaryleft + cir[i]->radius) {
				cir[i]->velocity[0] *= -1;
				cir[i]->centerPos[0] = boundaryleft + cir[i]->radius;
			}
			if (cir[i]->centerPos[1] > boundaryup - cir[i]->radius) {
				cir[i]->velocity[1] *= -1;
				cir[i]->centerPos[1] = boundaryup - cir[i]->radius;
			}

			else if (cir[i]->centerPos[1] < boundarydown - cir[i]->radius) {
				Circle* temppointer = *(cir.begin() + i);
				cir.erase(cir.begin()+i);
				delete temppointer;
			}
		}

		//공과 바의 충돌
		{
			for (unsigned int i = 0; i < cir.size(); i++) {
				Rectan* rectofbarobject = rectofbar;
				{
					bool quickcheck = cir[i]->centerPos[1] < rectofbarobject->cornerPoint[1] + rectofbarobject->length[1] + cir[i]->radius && cir[i]->centerPos[1] > rectofbarobject->cornerPoint[1] - cir[i]->radius;
					if (quickcheck) {

						Vec3f rectofbarcenter = rectofbarobject->cornerPoint + Vec3f(rectofbarobject->length[0] / 2, rectofbarobject->length[1] / 2, 0);

						float deltax = (rectofbarcenter - cir[i]->centerPos)[0] / rectofbarobject->length[0];
						float deltay = (rectofbarcenter - cir[i]->centerPos)[1] / rectofbarobject->length[1];

						float slope = deltay / deltax;
						bool contacthorizontally = -1 < slope &&slope < 1;


						Vec3f distanceCorner1 = cir[i]->centerPos - rectofbarobject->cornerPoint;
						Vec3f distanceCorner2 = distanceCorner1;
						distanceCorner2[0] -= rectofbarobject->length[0];

						Vec3f distanceCorner3 = cir[i]->centerPos - (rectofbarobject->cornerPoint + Vec3f(0, rectofbarobject->length[1], 0));
						Vec3f distanceCorner4 = distanceCorner3;
						distanceCorner4[0] -= rectofbarobject->length[0];
						float distance1 = sqrt(dotoperator(distanceCorner1, distanceCorner1));
						float distance2 = sqrt(dotoperator(distanceCorner2, distanceCorner2));

						float distance3 = sqrt(dotoperator(distanceCorner3, distanceCorner4));
						float distance4 = sqrt(dotoperator(distanceCorner3, distanceCorner4));

						bool mostly = cir[i]->centerPos[0] > rectofbarobject->cornerPoint[0] && cir[i]->centerPos[0] < rectofbarobject->cornerPoint[0] + rectofbarobject->length[0];
						bool rarely = distance1 < cir[i]->radius || distance2 < cir[i]->radius || distance3 < cir[i]->radius || distance4 < cir[i]->radius;

						if (mostly || rarely) {
							if (contacthorizontally) {
								cir[i]->velocity[0] *= -1;
								if (cir[i]->centerPos[0] < rectofbarcenter[0])
									cir[i]->centerPos[0] = rectofbarobject->cornerPoint[0] - cir[i]->radius;
								else
									cir[i]->centerPos[0] = rectofbarobject->cornerPoint[0] + rectofbarobject->length[0] + cir[i]->radius;
							}
							else {
								cir[i]->velocity[1] *= -1;
								if (cir[i]->centerPos[1] < rectofbarcenter[1])
									cir[i]->centerPos[1] = rectofbarobject->cornerPoint[1] - cir[i]->radius;
								else
									cir[i]->centerPos[1] = rectofbarobject->cornerPoint[1] + rectofbarobject->length[1] + cir[i]->radius;
							}
						}
					}
				}
			}
			for (unsigned int i = 0; i < cir.size(); i++) {
				for (unsigned int j = 0; j < 2; j++) {
					static Circle *cirofbar;
					if (j == 0)
						cirofbar = leftcirofbar;
					else if (j == 1)
						cirofbar = rightcirofbar;
					if (collisionDetection(*cir[i], *cirofbar)) {
						Vec3f vel_difference = cir[i]->velocity - cirofbar->velocity;
						Vec3f pos_difference = cir[i]->centerPos - cirofbar->centerPos;
						float cal = dotoperator(vel_difference, pos_difference) / dotoperator(pos_difference, pos_difference);
						cir[i]->velocity -= 2 * cal*pos_difference;
						float distance = sqrt(dotoperator(pos_difference, pos_difference));
						float overlap = cir[i]->radius + cirofbar->radius - distance;
						Vec3f normalize = pos_difference / distance;
						cir[i]->centerPos += normalize*overlap;

					}
				}
			}
		}

		if (cir.size() == 0) {
			exit(0);
		}
		score_update();
		preTime = current;
	}

	if (bricks.size() == 0) {
	
		level++;
		for (unsigned int i = 0; i < cir.size();i++) {
			Circle* temppointer = *(cir.begin() + i);
			cir.erase(cir.begin() + i);
			delete temppointer;
		}
		for (unsigned int i = 0; i < items.size(); i++) {
			Rectan* temppointer = *(items.begin() + i);
			items.erase(items.begin() + i);
			delete temppointer;
		}
		initShape();
		if (level>10)
			exit(0);
	}




	glutPostRedisplay();
}
void funcitemdelete() {
	float mostdown=100;
	unsigned int bricknum=bricks.size();
	for (unsigned int i = 0; i < bricknum; i++) {
		if (bricks[i]->cornerPoint[1]<mostdown) {
			mostdown = bricks[i]->cornerPoint[1];
		}
	}
	for (unsigned int j = 0; j < 10;j++) {
		for (unsigned int i = 0; i < bricks.size(); i++) {
			if (bricks[i]->cornerPoint[1] == mostdown) {
				Rectan* temppointer = *(bricks.begin() + i);
				bricks.erase(bricks.begin() + i);
				deletedbricknum++;
				delete temppointer;
			}
		}
	}



}
void funcunitembeatable() {
	unbeatablestate = true;
}
void funcitemmultiple() {
	Circle* shape = new Circle();
	newballpos = Vec3f(rectofbar->cornerPoint[0] + rectofbar->length[0] / 2, rectofbar->cornerPoint[1] + rectofbar->length[1] + shape->radius, 0);
	shape->setPosition(rectofbar->cornerPoint[0] + rectofbar->length[0] / 2, rectofbar->cornerPoint[1] + rectofbar->length[1] + shape->radius);
	shape->setvelocity(0, 0);
	shape->setRGB(0, 0, 1);
	cir.push_back(shape);
}
void score_update() {
	//score_update
	//red = 0, orange = 1, yellow = 2, green = 3, blue = 4, Indigo = 5, Violet = 6
	score = (500-clock()+startTime)/3000 + 5*deletedbricknum;
	if (score < 0)
		score = 0;
}

void main(int argc, char **argv)
{
	srand(time(NULL));
	initialize();
	initShape();

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow("Arkanoid");

	glutDisplayFunc(renderScene);
	preTime = clock();
	glutIdleFunc(idle);
	glutSpecialFunc(SpecialKey);
	glutKeyboardFunc(MyKeyboardFunc);
	glutMotionFunc(processMouse);
	glutMainLoop();

	for (unsigned int i = 0; i<cir.size(); i++) {
		delete cir[i];
	}
	delete rectofbar;
}
