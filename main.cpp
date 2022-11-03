#include "Unifiedheader.h"  
#include "vectormath.h"
#include "gameobject.h"

constexpr int screenwidth = 1920;
constexpr int screenheight = 1080;

float timePassed;
int score = 0;
constexpr int droneCost = 500;
// 4 different ways to look at the ship
// default is front          back,  left,  right
bool cameraChangeLook[3] = { false, false, false };
C_Model ship_obj, bullet_obj;

// texture variables
GLuint ship_texture;
GLuint bullet_texture;
GLuint missile_texture;
GLuint explosion_texture;
GLuint propeller_texture;
GLuint sky_front_texture;
GLuint sky_back_texture;
GLuint sky_left_texture;
GLuint sky_right_texture;
GLuint sky_up_texture;
GLuint sky_down_texture;
GLuint asteroid1_texture;
GLuint asteroid2_texture;
GLuint asteroid3_texture;
GLuint asteroid4_texture;

void init() noexcept {
	// load all the textures once and put them in the global variables for minimising the loading process
	ship_texture = load_texture("ship/ship.png");
	bullet_texture = load_texture("bullets/bullet.jpg");
	missile_texture = load_texture("bullets/missile.jpg");
	explosion_texture = load_texture("bullets/explosion.png");
	propeller_texture = load_texture("drone/propeller.png");
	sky_front_texture = load_texture("skybox/front.png");
	sky_back_texture = load_texture("skybox/back.png");
	sky_left_texture = load_texture("skybox/left.png");
	sky_right_texture = load_texture("skybox/right.png");
	sky_up_texture = load_texture("skybox/up.png");
	sky_down_texture = load_texture("skybox/down.png");
	asteroid1_texture = load_texture("asteroids/asteroid1.jpg");
	asteroid2_texture = load_texture("asteroids/asteroid2.jpg");
	asteroid3_texture = load_texture("asteroids/asteroid3.jpg");
	asteroid4_texture = load_texture("asteroids/asteroid4.jpg");

	// create skybox
	skybox* sb = new skybox();
	g_gameObjectPool.m_GlobalSkybox = sb;

	// create spaceship
	spaceship* s = new spaceship();
	g_gameObjectPool.m_GlobalSpaceship = s;
	g_gameObjectPool.m_gameObjectList.push_back(s);

	// load ship obj
	string filepath = "ship/ship.obj";
	ifstream ship_file(filepath);
	ship_obj.loadOjb(ship_file);
	ship_file.close();
	g_gameObjectPool.m_GlobalSpaceship_obj = ship_obj;

	/* load bullet obj
	filepath = "bullets/bullet.obj";
	ifstream bullet_file(filepath);
	bullet_obj.loadOjb(bullet_file);
	bullet_file.close();
	g_gameObjectPool.m_GlobalBullet_obj = bullet_obj;
	*/
}

void gameProcess() {
	g_TimeManger.GetTimefromClock();
	g_AsteroidLauncher.shootAsteroid(g_gameObjectPool);
}

constexpr int font = (int)GLUT_BITMAP_9_BY_15;

void renderBitmapString(float x, float y, float z, void* font, const char* string) noexcept {
	const char* c;
	c = nullptr;
	glPushMatrix();
	glColor4f(1, 1, 1, 1);
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++)
		glutBitmapCharacter(font, *c);
	glPopMatrix();
}

void showGameScoreAndTime() {
	const int gametime = g_TimeManger.GetGameTime();
	score = g_AsteroidLauncher.m_DestroyedAsteroid * 100 - (droneCost * g_droneManager.howManyDronesSpawned);

	char emptystring[128];
	char total[256];

	spaceship* ship = g_gameObjectPool.m_GlobalSpaceship;
	vec3f vecDir = ship->dir;
	vecDir = vecDir.normalize();

	// calculate up vector and right vector of ship's dir vector
	vec3f vecUp = vec3f(0, 0, 1);
	vec3f vecRight;

	vecRight = vecDir.cross(vecUp);
	vecRight = vecRight.normalize();

	sprintf_s(emptystring, "                            ");

	if (g_gameState != GAME_OVER) {
		sprintf_s(total, "TIME PASSED : %.2f sec %s Asteroid Game %s Score : %d < 1 DRONE COST : 500 >", static_cast<float>(gametime) / 1000.0f, emptystring, emptystring, score);
		timePassed = static_cast<float>(gametime) / 1000;
		renderBitmapString(ship->pos.x - (vecRight.x * 7.4f), ship->pos.y - (vecRight.y * 7.4f), 8.0f, (void*)font, total);
	}
	else {
		sprintf_s(total, "TIME PASSED : %.2f sec %s Asteroid Game %s Score : %d < 1 DRONE COST : 500 >", timePassed, emptystring, emptystring, score);
		renderBitmapString(-5.0f, -5.0f, 7.81f, (void*)font, total);
	}
}

// Little bit of reddish light for realistic view
const GLfloat ambientLight0[] = { 0.6f, 0.5f, 0.5f, 1.0f };
const GLfloat diffuseLight0[] = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat specular0[] = { 0.5f, 0.5f, 0.5f, 1.0f };

void cameraSetting() {
	// camera setup using ship pos and dir
	spaceship* ship;
	if (g_gameState != GAME_OVER)
		ship = g_gameObjectPool.m_GlobalSpaceship;
	else 
		ship = new spaceship();
	
	vec3f vecDir = ship->dir;
	vecDir = vecDir.normalize();

	// calculate up vector and right vector of ship's dir vector
	vec3f vecUp = vec3f(0, 0, 1);
	vec3f vecRight;

	vecRight = vecDir.cross(vecUp);
	vecRight = vecRight.normalize();

	float fDirDistance = 15.0f;
	float fUpDistance = 7.5f;

	// camera pos is behind and up of ship
	vec3f cameraEye;

	// select which point the camera looks at
	if (cameraChangeLook[0]) // back
		cameraEye = ship->pos - (ship->dir * (-1) * fDirDistance) + (vecUp * fUpDistance);
	else if (cameraChangeLook[1]) // left
		cameraEye = ship->pos - (vecRight * (-1) * fDirDistance) + (vecUp * fUpDistance);
	else if (cameraChangeLook[2]) // right
		cameraEye = ship->pos - (vecRight * fDirDistance) + (vecUp * fUpDistance);
	else // front
		cameraEye = ship->pos - (ship->dir * fDirDistance) + (vecUp * fUpDistance);

	// Light for ship
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

	// Light pos is same as camera pos
	GLfloat lightPos[] = { cameraEye.x, cameraEye.y, cameraEye.z, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHT0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular0);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Look ship
	gluLookAt(cameraEye.x, cameraEye.y, cameraEye.z, ship->pos.x, ship->pos.y, ship->pos.z, 0, 0, 1);
}

float last_time = 0;

void display(void) {
	// Process   
	if (g_gameState == GAME_PLAYING)
		gameProcess();

	// Render
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Skybox
	g_skyboxManager.Render();

	// if game over
	if (g_gameState == GAME_OVER) {
		gameProcess();
		// Draw objects first to use glBlendFunc and GL_BLEND in Arena
		g_gameObjectPool.process();
		g_ArenaManager.Process();
		g_gameObjectPool.render();
		g_ArenaManager.Render();

		showGameScoreAndTime();
		renderBitmapString(-0.5f, 0, 1.5f, (void*)font, "Game Over. Press any key to play again...");
	}
	// if game start
	else if (g_gameState == FIRST_START) {
		cameraSetting();
		renderBitmapString(-0.5f, 0, 1.5f, (void*)font, "Press any key to start...");
	}
	// if game is starting
	else if (g_gameState == GAME_PLAYING) {
		g_gameObjectPool.process();
		g_ArenaManager.Process();

		float cur_time = glutGet(GLUT_ELAPSED_TIME);
		float dt = cur_time - last_time;
		last_time = cur_time;
		if (dt > 0.5f) {
			// Draw objects first to use glBlendFunc and GL_BLEND in Arena
			cameraSetting();
			g_gameObjectPool.render();
			g_ArenaManager.Render();

			showGameScoreAndTime();
		}
	}
	glutSwapBuffers();
}

// mouse button
void mouse(int button, int state, int x, int y) {
	if (g_gameState != GAME_PLAYING) {
		g_gameState = GAME_PLAYING;
		g_TimeManger.SetStartTime();
		RestartGame();
	}

	if (button == GLUT_LEFT_BUTTON) {
		if (g_missileManager.missileInUse == false)
			g_bulletManager.spaceshipFireBullet(g_gameObjectPool, *g_gameObjectPool.m_GlobalSpaceship);
		else
			g_missileManager.spaceshipFireMissile(g_gameObjectPool, *g_gameObjectPool.m_GlobalSpaceship);
	}
}

// keyboard button
void keyboard(unsigned char key, int x, int y) {
	if (g_gameState != GAME_PLAYING) {
		g_gameState = GAME_PLAYING;
		g_TimeManger.SetStartTime();
		RestartGame();
	}
	else {
		switch (key) {
		case 'w':
		case 'W':
			g_gameObjectPool.m_GlobalSpaceship->speed += spaceshipAccel;
			if (g_gameObjectPool.m_GlobalSpaceship->speed >= spaceshipMaxSpeed)
				g_gameObjectPool.m_GlobalSpaceship->speed = spaceshipMaxSpeed;
			break;
		case 'a':
		case 'A':
			g_gameObjectPool.m_GlobalSpaceship->rotate += spaceshipRotateSpeed;
			break;
		case 'd':
		case 'D':
			g_gameObjectPool.m_GlobalSpaceship->rotate -= spaceshipRotateSpeed;
			break;
		case 'x':
		case 'X':
			if ((score >= 500) && (g_droneManager.droneInArena < maxDroneNumberAtOnce))
				g_droneManager.spawnDrone(g_gameObjectPool, *g_gameObjectPool.m_GlobalSpaceship);
			break;
		case 'z':
		case 'Z':
			g_missileManager.toggleMissile(g_gameObjectPool, *g_gameObjectPool.m_GlobalSpaceship);
			break;
		case 'c':
		case 'C':
			if ((cameraChangeLook[0] == false && cameraChangeLook[1] == false && cameraChangeLook[2] == false)
				|| (cameraChangeLook[0] == true && cameraChangeLook[1] == false && cameraChangeLook[2] == false))
				cameraChangeLook[0] = !cameraChangeLook[0];
			break;
		case 'q':
		case 'Q':
			if ((cameraChangeLook[0] == false && cameraChangeLook[1] == false && cameraChangeLook[2] == false)
				|| (cameraChangeLook[0] == false && cameraChangeLook[1] == true && cameraChangeLook[2] == false))
				cameraChangeLook[1] = !cameraChangeLook[1];
			break;
		case 'e':
		case 'E':
			if ((cameraChangeLook[0] == false && cameraChangeLook[1] == false && cameraChangeLook[2] == false)
				|| (cameraChangeLook[0] == false && cameraChangeLook[1] == false && cameraChangeLook[2] == true))
				cameraChangeLook[2] = !cameraChangeLook[2];
			break;
		default:
			break;
		}
	}
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat ratio = (float)w / h;
	gluPerspective(60, ratio, 0.1, 1000);
}

float last_idle_time = 0;

void idle() {
	float cur_time = glutGet(GLUT_ELAPSED_TIME);
	float dt = cur_time - last_idle_time;
	last_idle_time = cur_time;
	if (dt > 0.5)
		glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(screenwidth, screenheight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("ASTEROID GAME");

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	// Please delete // if you want to play with full screen
	//glutFullScreen();
	init();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glutMainLoop();

	return 0;
}