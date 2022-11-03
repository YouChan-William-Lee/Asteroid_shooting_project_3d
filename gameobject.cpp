#ifndef _GAMEOBJECT_
#define _GAMEOBJECT_

#include "gameobject.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

timeManager g_TimeManger;
arenaManager g_ArenaManager;
asteroidLauncher g_AsteroidLauncher;
objectPool g_gameObjectPool;
bulletManager g_bulletManager;
explosionManager g_explosionManager;
droneManager g_droneManager;
missileManager g_missileManager;
skyboxManager g_skyboxManager;
GAMESTATE g_gameState = FIRST_START;

// Arena's leftTop and rightBottom position
vec3f g_arenaPosition[2];

bool gameObject::checkcollision(const gameObject& other) {
	float distance = pos.distance(other.pos);
	if (distance <= radius + other.radius)
		return true; // collided 
	else
		return false;
}

asteroid::~asteroid() {}

void asteroid::render() noexcept {
	// Select the texture object
	glBindTexture(GL_TEXTURE_2D, obj_texture);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0);

	float randNum = GetRandom(0.1f);
	float rotation;

	// decide clockwise or anticlockwise
	if (rotate < 0.5)
		rotation = (g_TimeManger.Getsec() * -1.0f) - (createdtime + randNum);
	else
		rotation = g_TimeManger.Getsec() - (createdtime + randNum);

	glRotatef(rotation * 5.0f, 0, 0, -1);

	GLUquadric* quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricTexture(quadric, GL_TRUE);
	gluSphere(quadric, radius, tessellation1, tessellation2);
	gluDeleteQuadric(quadric);	

	// show cracks which means still have energy
	if (objectHasCrack) {
		glPushMatrix();
		glColor4f(rColour, gColour, bColour, 1.0f);
		glutWireSphere(radius + 0.05f, tessellation1, tessellation2);
		glPopMatrix();
	}
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool objectPool::asteroidInArena(asteroid* asteroid) noexcept {
	float halfArenaX = xwidthofspace * ArenaXPercentage * 0.5f;
	float halfArenaY = yheightofspace * ArenaYPercentage * 0.5f;
	float distanceOffset;

	if (asteroid->objectInArena == false)
		distanceOffset = 1.0f;
	else
		distanceOffset = -1.0f;

	if ((asteroid->pos.x > (-halfArenaX + asteroid->radius + distanceOffset))
		&& (asteroid->pos.x < (halfArenaX - asteroid->radius - distanceOffset))
		&& (asteroid->pos.y > (-halfArenaY + asteroid->radius + distanceOffset))
		&& (asteroid->pos.y < (halfArenaY - asteroid->radius - distanceOffset)))
		return true;
	return false;
}

spaceship::~spaceship() {}

void spaceship::process() {
	// if gameover, move spaceship to out of Arena to prevent more collisions
	if (g_gameState != GAME_OVER) {
		dir.x = cosf(rotate + PI / 2.0f);
		dir.y = sinf(rotate + PI / 2.0f);
		dir.z = 0;
		pos = (pos + (dir * speed));

		speed -= spaceshipNaturalDeAccel * 0.03f;
		if (speed < 0)
			speed = 0;
	}
	else
		g_gameObjectPool.m_GlobalSpaceship->pos = vec3f(RAND_MAX, RAND_MAX, 0);
}

void spaceship::render() noexcept {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, obj_texture);
	glPushMatrix();

	glTranslatef(pos.x, pos.y, pos.z);
	// obj file is heading to - z axis, so this makes it to head + y axis
	glRotatef(180.0f, 0, 0, 1);
	glRotatef(rotate * 180.0f / PI, 0, 0, 1);
	// obj file is slightly big, so make it bit smaller
	glScalef(0.85f, 0.85f, 0.85f);

	GLfloat x, y, z, nx, ny, nz, tx, ty;
	int v_id, vt_id, vn_id;
	int nFaces = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].f.size();

	for (int k = 0; k < nFaces; k++) {
		int nPoints = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].f[k].v_pairs.size();
		glBegin(GL_POLYGON);
		for (int i = 0; i < nPoints; i++) {
			v_id = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].f[k].v_pairs[i].d[0];
			vt_id = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].f[k].v_pairs[i].d[1];
			vn_id = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].f[k].v_pairs[i].d[2];

			x = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].v[v_id - 1].d[0];
			y = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].v[v_id - 1].d[1];
			z = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].v[v_id - 1].d[2];

			nx = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].vn[vn_id - 1].d[0];
			ny = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].vn[vn_id - 1].d[1];
			nz = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].vn[vn_id - 1].d[2];

			tx = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].vt[vt_id - 1].d[0];
			ty = g_gameObjectPool.m_GlobalSpaceship_obj.objs[0].vt[vt_id - 1].d[1];

			glNormal3f(nx, ny, nz);
			glTexCoord2f(tx, ty);
			glVertex3f(x, y, z);
		}
		glEnd();
	}
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void objectPool::process() {
	for (std::list<gameObject*>::iterator it = m_gameObjectList.begin(); it != m_gameObjectList.end(); it++) {
		(*it)->process();
		float halfArenaX = xwidthofspace * ArenaXPercentage * 0.5f;
		float halfArenaY = yheightofspace * ArenaYPercentage * 0.5f;

		for (std::list<gameObject*>::iterator it2 = m_gameObjectList.begin(); it2 != m_gameObjectList.end(); it2++) {
			// collision between Asteroid and Arena
			if ((*it2)->type == ASTEROID) {
				// check if Asteroid get into the Arena
				if ((*it2)->objectInArena == false) {
					if (asteroidInArena((asteroid*)(*it2)))
						(*it2)->objectInArena = true;
				}
				// collision with one of the walls after Asteroid get into the Arena
				else {
					// once asteroid get into Arena, then it cannot go out of Arena, so consider it as a bug 
					if (asteroidInArena((asteroid*)(*it2)) == false)
						(*it2)->duration = -1;

					float distanceOffset = 1.3f;
					vec3f posCurrent = (*it2)->pos;
					float radius = (*it2)->radius;

					// left line of Arena
					if (abs(posCurrent.x - g_arenaPosition[0].x) <= radius) {
						vec3f reverseVec = vec3f((*it2)->dir.x * -1.0f, (*it2)->dir.y, 0);
						reverseVec = reverseVec.normalize();
						(*it2)->pos = posCurrent + reverseVec * distanceOffset;
						(*it2)->dir = reverseVec;
					}

					// top line of Arena
					if (abs(posCurrent.y - g_arenaPosition[0].y) <= radius) {
						vec3f reverseVec = vec3f((*it2)->dir.x, (*it2)->dir.y * -1.0f, 0);
						reverseVec = reverseVec.normalize();
						(*it2)->pos = posCurrent + reverseVec * distanceOffset;
						(*it2)->dir = reverseVec;
					}

					// right line of Arena
					if (abs(posCurrent.x - g_arenaPosition[1].x) <= radius) {
						vec3f reverseVec = vec3f((*it2)->dir.x * -1.0f, (*it2)->dir.y, 0);
						reverseVec = reverseVec.normalize();
						(*it2)->pos = posCurrent + reverseVec * distanceOffset;
						(*it2)->dir = reverseVec;
					}

					// bottom line of Arena
					if (abs(posCurrent.y - g_arenaPosition[1].y) <= radius) {
						vec3f reverseVec = vec3f((*it2)->dir.x, (*it2)->dir.y * -1.0, 0);
						reverseVec = reverseVec.normalize();
						(*it2)->pos = posCurrent + reverseVec * distanceOffset;
						(*it2)->dir = reverseVec;
					}
				}
			}
			// collision between Bullet and Arena
			if ((*it2)->type == BULLET || (*it2)->type == MISSILE) {
				// collision with wall after Bullet is launched
				if ((abs(-halfArenaX - ((*it2)->pos.x + (*it2)->radius)) <= ArenaGameOverdistance)
					|| (abs(halfArenaX - ((*it2)->pos.x + (*it2)->radius)) <= ArenaGameOverdistance))
					(*it2)->duration = -1;
				else if (((abs(-halfArenaY - ((*it2)->pos.y + (*it2)->radius)) <= ArenaGameOverdistance)
					|| (abs(halfArenaY - ((*it2)->pos.y + (*it2)->radius)) <= ArenaGameOverdistance)))
					(*it2)->duration = -1;
			}

			if ((*it)->checkcollision(**it2)) {
				// collision between Asteroid and Spaceship 
				if (((*it)->type == ASTEROID && (*it2)->type == SPACESHIP) || ((*it)->type == SPACESHIP && (*it2)->type == ASTEROID)) {
					killAfterCollision(*it, *it2);

					// SHIP COLLIDE WITH ASTEROID . WE CAN MAKE HEALTH POINT HERE OR JUST GO TO GAME OVER
					g_gameState = GAME_OVER;
				}
				// collision between Asteroid and Bullet or Missile
				else if (((*it)->type == ASTEROID && (*it2)->type == BULLET)
					|| ((*it)->type == BULLET && (*it2)->type == ASTEROID)
					|| ((*it)->type == ASTEROID && (*it2)->type == MISSILE)
					|| ((*it)->type == MISSILE && (*it2)->type == ASTEROID)) {
					killAfterCollision(*it, *it2);

					g_AsteroidLauncher.m_DestroyedAsteroid++;
				}
				// collision between 2 Asteroids in Arena
				else if ((*it)->type == ASTEROID && ((*it)->type == (*it2)->type)
					&& ((*it)->objectInArena == true) && ((*it2)->objectInArena == true)) {
					vec3f firstAsteroidDir = (*it)->dir;
					vec3f firstAsteroidPos = (*it)->pos;
					vec3f secondAsteroidDir = (*it2)->dir;
					vec3f secondAsteroidPos = (*it2)->pos;

					// calculate first Asteroid's dir
					(*it)->dir = ((firstAsteroidPos - secondAsteroidPos).normalize() + firstAsteroidDir).normalize();
					// Calculate second Asteroid's dir
					(*it2)->dir = ((secondAsteroidPos - firstAsteroidPos).normalize() + secondAsteroidDir).normalize();
				}
				// collision between Asteroid and Drone in Arena
				else if (((*it)->type == ASTEROID && (*it2)->type == DRONE)
					|| ((*it)->type == DRONE && (*it2)->type == ASTEROID))
					killAfterCollision(*it, *it2);
			}
		}

		// kill objects which have 0 or less than 0 energy, or 0 or less than 0 duration except spaceship
		if ((*it)->NeedtoKillthis(g_droneManager)) {
			if ((*it)->type == DRONE) {
				// launch one more set of bullets before get killed
				g_droneManager.droneFireBullet(g_gameObjectPool, (*it)->pos, g_gameObjectPool.m_GlobalSpaceship->radius);
				g_droneManager.droneInArena -= 1;
			}
			delete* it;
			it = m_gameObjectList.erase(it);
			--it;
		}
	}
}

void objectPool::render() noexcept {
	for (std::list<gameObject*>::iterator it = m_gameObjectList.begin(); it != m_gameObjectList.end(); it++)
		(*it)->render();
}

void objectPool::killAfterCollision(gameObject* gameObject1, gameObject* gameObject2) noexcept {
	// ASTEROID with SPACESHIP -> just kill Asteroid
	if (gameObject1->type == ASTEROID && gameObject2->type == SPACESHIP) {
		g_explosionManager.showExplosionSpaceship(g_gameObjectPool, gameObject2->pos, gameObject2->radius);
		g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject1);
		gameObject1->duration = -1;
	}
	// ASTEROID with SPACESHIP -> just kill Asteroid
	else if (gameObject1->type == SPACESHIP && gameObject2->type == ASTEROID) {
		g_explosionManager.showExplosionSpaceship(g_gameObjectPool, gameObject1->pos, gameObject1->radius);
		g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject2);
		gameObject2->duration = -1;
	}
	// ASTEROID with BULLET -> kill both but still calculate energy of Asteroid
	else if (gameObject1->type == ASTEROID && gameObject2->type == BULLET) {
		gameObject1->energy -= 1;
		gameObject2->duration = -1;
		if (gameObject1->energy <= 0 && gameObject1->objectHasCrack == false)
			g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject1);
		// Asteroid has crack and energy is exhausted, then split it
		if (gameObject1->energy <= 0 && gameObject1->objectHasCrack == true) {
			SplitAsteroid(gameObject1);
			gameObject1->duration = -1;
		}
		if (gameObject1->energy > 0) {
			gameObject1->objectHasCrack = true;
		}
	}
	// ASTEROID with BULLET -> kill both but still calculate energy of Asteroid
	else if (gameObject1->type == BULLET && gameObject2->type == ASTEROID) {
		gameObject2->energy -= 1;
		gameObject1->duration = -1;
		if (gameObject2->energy <= 0 && gameObject2->objectHasCrack == false)
			g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject2);
		// Asteroid has crack and energy is exhausted, then split it
		if (gameObject2->energy <= 0 && gameObject2->objectHasCrack == true) {
			SplitAsteroid(gameObject2);
			gameObject2->duration = -1;
		}
		if (gameObject2->energy > 0) {
			gameObject2->objectHasCrack = true;
		}
	}
	// ASTEROID with MISSILE -> kill both
	else if (gameObject1->type == ASTEROID && gameObject2->type == MISSILE) {
		g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject1);
		// missile do not split the Asteroid but just kill
		gameObject1->energy -= 2;
		gameObject2->duration = -1;
	}
	// ASTEROID with MISSILE -> kill both
	else if (gameObject1->type == MISSILE && gameObject2->type == ASTEROID) {
		g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject2);
		gameObject2->energy -= 2;
		gameObject1->duration = -1;
	}
	// ASTEROID with DRONE -> kill Asteroid
	else if (gameObject1->type == ASTEROID && gameObject2->type == DRONE) {
		g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject1);
		gameObject1->duration -= 1;
	}
	// ASTEROID with DRONE -> kill Asteroid
	else if (gameObject1->type == DRONE && gameObject2->type == ASTEROID) {
		g_explosionManager.showExplosionAsteroid(g_gameObjectPool, gameObject2);
		gameObject2->duration = -1;
	}
}

void objectPool::SplitAsteroid(gameObject* gameObject) {
	asteroid* aNewHalfAsteroid1 = new asteroid;
	asteroid* aNewHalfAsteroid2 = new asteroid;

	// --- calculate first new Asteroid's pos and dir ---
	// position is original position (sin(@), -cos(@))
	vec3f originalAsteroidDir = gameObject->dir;
	vec3f newAsteroid1Pos = vec3f(gameObject->pos.y, gameObject->pos.x * -1.0f, 0);
	// vector of 45 degree
	vec3f newAsteroid1Dir = (originalAsteroidDir.normalize() + newAsteroid1Pos.normalize()).normalize();

	aNewHalfAsteroid1->speed = gameObject->speed;
	aNewHalfAsteroid1->pos = gameObject->pos + (newAsteroid1Pos.normalize() * gameObject->radius * 2.0f);
	aNewHalfAsteroid1->dir = newAsteroid1Dir;
	aNewHalfAsteroid1->energy = 1;
	aNewHalfAsteroid1->objectInArena = true;
	// follow the original asteroid's texture
	aNewHalfAsteroid1->obj_texture = gameObject->obj_texture;
	aNewHalfAsteroid1->radius = gameObject->radius / 2.0f;
	m_gameObjectList.push_back(aNewHalfAsteroid1);

	// --- calculate second new Asteroid's pos and dir --- 
	// position is original position + (-sin(@), cos(@))
	vec3f newAsteroid2Pos = vec3f(gameObject->pos.y * -1.0f, gameObject->pos.x, 0);
	// vector of 45 degree
	vec3f newAsteroid2Dir = (originalAsteroidDir.normalize() + newAsteroid2Pos.normalize()).normalize();

	aNewHalfAsteroid2->speed = gameObject->speed;
	aNewHalfAsteroid2->pos = gameObject->pos + (newAsteroid2Pos.normalize() * gameObject->radius * 2.0f);
	aNewHalfAsteroid2->dir = newAsteroid2Dir;
	aNewHalfAsteroid2->energy = 1;
	// follow the original asteroid's texture
	aNewHalfAsteroid2->obj_texture = gameObject->obj_texture;
	aNewHalfAsteroid2->objectInArena = true;
	aNewHalfAsteroid2->radius = gameObject->radius / 2.0f;
	m_gameObjectList.push_back(aNewHalfAsteroid2);
}

bullet::~bullet() {}

void bullet::render() noexcept {	
	glBindTexture(GL_TEXTURE_2D, obj_texture);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	
	GLUquadric* quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricTexture(quadric, GL_TRUE);
	gluSphere(quadric, radius, 30, 30);
	gluDeleteQuadric(quadric);

	/* I've tried to use free bullet 3d obj file(bullets/bullet.obj), but because of its size, it looks like sphere so 
	* there is no point to use a lot of vertices. And it also makes some buffer as well.
	* I haven't deleted to show how I wanted to draw.
	
	glScalef(0.05f, 0.05f, 0.05f); 
	GLfloat x, y, z, nx, ny, nz, tx, ty;
	int v_id, vt_id, vn_id;
	int nFaces = g_gameObjectPool.m_GlobalBullet_obj.objs[0].f.size();

	for (int i = 0; i < nFaces; i++) {
		int nPoints = g_gameObjectPool.m_GlobalBullet_obj.objs[0].f[i].v_pairs.size();
		glBegin(GL_POLYGON);
		glColor4f(1, 1, 1, 1);
		if (g_gameState == GAME_OVER)
			glColor4f(1.0f, 0, 0, 1.0f);
		else
			glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
		for (int j = 0; j < nPoints; j++) {
			v_id = g_gameObjectPool.m_GlobalBullet_obj.objs[0].f[i].v_pairs[j].d[0];
			vt_id = g_gameObjectPool.m_GlobalBullet_obj.objs[0].f[i].v_pairs[j].d[1];
			vn_id = g_gameObjectPool.m_GlobalBullet_obj.objs[0].f[i].v_pairs[j].d[2];

			x = g_gameObjectPool.m_GlobalBullet_obj.objs[0].v[v_id - 1].d[0];
			y = g_gameObjectPool.m_GlobalBullet_obj.objs[0].v[v_id - 1].d[1];
			z = g_gameObjectPool.m_GlobalBullet_obj.objs[0].v[v_id - 1].d[2];

			nx = g_gameObjectPool.m_GlobalBullet_obj.objs[0].vn[vn_id - 1].d[0];
			ny = g_gameObjectPool.m_GlobalBullet_obj.objs[0].vn[vn_id - 1].d[1];
			nz = g_gameObjectPool.m_GlobalBullet_obj.objs[0].vn[vn_id - 1].d[2];

			tx = g_gameObjectPool.m_GlobalBullet_obj.objs[0].vt[vt_id - 1].d[0];
			ty = g_gameObjectPool.m_GlobalBullet_obj.objs[0].vt[vt_id - 1].d[1];

			glNormal3f(nx, ny, nz);
			glTexCoord2f(tx, ty);
			glVertex3f(x, y, z);
		}
		glEnd();
	}
	*/

	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

explosion::~explosion() {}

void explosion::render() noexcept {
	glBindTexture(GL_TEXTURE_2D, obj_texture);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glRotatef(180.0f * GetRandom(1.0f), 0, 0, 1);

	GLUquadric* quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricTexture(quadric, GL_TRUE);
	gluSphere(quadric, radius, 10, 10);
	gluDeleteQuadric(quadric);

	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

drone::~drone() {}

void drone::render() noexcept {
	glPushMatrix();

	constexpr float count = 6.0f;
	constexpr float propellerRadius = 1.2f;
	constexpr float propellerAngle = 0.001f;

	glPushMatrix();
	glLineWidth(10.0f);

	// draw drone body
	glBegin(GL_LINE_LOOP);
	glColor4f(1.0f, 0.5f, 1.0f, 0.5f);
	for (float i = 0; i < count; i++) {
		constexpr float apie = PI * 2.0f / count;
		const float x = cosf(apie * i) * radius;
		const float y = sinf(apie * i) * radius;
		glVertex3f(pos.x + x, pos.y + y, 1.0f);
		glVertex3f(pos.x + x, pos.y + y, -1.0f);
		glVertex3f(pos.x + x, pos.y + y, 1.0f);
	}
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_LINE_LOOP);
	glColor4f(1.0f, 0.5f, 1.0f, 0.5f);
	for (float i = 0; i < count; i++) {
		constexpr float apie = PI * 2.0f / count;
		const float x = cosf(apie * i) * radius;
		const float y = sinf(apie * i) * radius;
		glVertex3f(pos.x + x, pos.y + y, -1.0f);
	}
	glEnd();
	glPopMatrix();
	
	glBindTexture(GL_TEXTURE_2D, propeller_texture);

	glPushMatrix();
	// draw drone propellers
	float randomPropellerAngle = GetRandom(propellerAngle);
	for (float i = 0; i < count; i++) {
		constexpr float aBodyPie = PI * 2.0f / count;
		const float x = cosf(aBodyPie * i) * radius;
		const float y = sinf(aBodyPie * i) * radius;

		float aPropellerPie = PI * 2.0f / randomPropellerAngle;
		const float wingX = cosf(aPropellerPie);
		const float wingY = sinf(aPropellerPie);

		const float uv_start_x = 0.0f;
		const float uv_start_y = 0.0f;
		const float uv_width = 1.0f;
		const float uv_height = 1.0f;

		glBegin(GL_POLYGON);
		glTexCoord2f(uv_start_x, uv_start_y);
		glVertex2f(pos.x + x * 1.1f + (propellerRadius * wingX), pos.y + y * 1.1f + (propellerRadius * wingY));

		glTexCoord2f(uv_start_x + uv_width, uv_start_y);
		glVertex2f(pos.x + x * 1.1f - (propellerRadius * wingX), pos.y + y * 1.1f + (propellerRadius * wingY));

		glTexCoord2f(uv_start_x + uv_width, uv_start_y + uv_height);
		glVertex2f(pos.x + x * 1.1f - (propellerRadius * wingX), pos.y + y * 1.1f - (propellerRadius * wingY));

		glTexCoord2f(uv_start_x, uv_start_y + uv_height);
		glVertex2f(pos.x + x * 1.1f + (propellerRadius * wingX), pos.y + y * 1.1f - (propellerRadius * wingY));		
		glEnd();
	}
	glPopMatrix();
	glLineWidth(1.0f);
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void missile::refreshMissileDirection() noexcept {
	for (std::list<gameObject*>::iterator it = g_gameObjectPool.m_gameObjectList.begin(); it != g_gameObjectPool.m_gameObjectList.end(); it++) {
		if (((*it)->type == MISSILE)) {
			vec3f nearestPos;
			float minDistance = FLT_MAX;
			int asteroidInArena = 0;
			vec3f missilePos = (*it)->pos;

			for (std::list<gameObject*>::iterator it2 = g_gameObjectPool.m_gameObjectList.begin(); it2 != g_gameObjectPool.m_gameObjectList.end(); it2++) {
				// check if there exists any Asteroid in Arena
				if (((*it2)->type == ASTEROID) && ((*it2)->objectInArena == true)) {
					vec3f asteroidPos = (*it2)->pos;
					float distance = missilePos.distance(asteroidPos);

					if (minDistance > distance) {
						minDistance = distance;
						nearestPos = asteroidPos;
						asteroidInArena++;
					}
				}
			}
			// find new vector from nearest asteroid's position
			vec3f missileNewDir = nearestPos - missilePos;
			// change the direction if only there is at least one Asteroid in Arena
			if (asteroidInArena > 0)
				(*it)->dir = missileNewDir.normalize();
			else
				(*it)->dir = g_gameObjectPool.m_GlobalSpaceship->dir;
		}
	}
}

void missile::render() noexcept {
	// refresh direction first
	refreshMissileDirection();

	glBindTexture(GL_TEXTURE_2D, obj_texture);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);

	GLUquadric* quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricTexture(quadric, GL_TRUE);
	gluSphere(quadric, radius, 10,10);
	gluDeleteQuadric(quadric);

	glColor4f(1, 1, 1, 1);
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
}

// There are 4 wave levels
int wave = 0;

void asteroidLauncher::shootAsteroid(objectPool& basepool) {
	int asteroidCount = 0;
	for (std::list<gameObject*>::iterator it = basepool.m_gameObjectList.begin(); it != basepool.m_gameObjectList.end(); it++) {
		if ((*it)->type == ASTEROID)
			if ((*it)->radius > Asteroidminradius)
				asteroidCount++;
	}

	const float timeAfterShoot = g_TimeManger.Getsec() - m_lastShootTime;
	// first wave launches 1 Asteroid, second wave launches 2 Asteroids, etc.
	const int waves[] = { 1, 2, 3, 4 };

	if (timeAfterShoot > asteroidInterval) {
		for (int i = 0; i < waves[wave]; i++) {
			if (asteroidCount < ateroidMaxCount) {
				// pick Random direction and shoot
				float Angle = GetRandom(2.0f * PI);
				float x = cosf(Angle);
				float y = sinf(Angle);
				float z = 0;

				// GetAsteroid Distance by Arena MAx size 
				float halfArenaX = xwidthofspace * ArenaXPercentage * 0.5f;
				float halfArenaY = yheightofspace * ArenaYPercentage * 0.5f;
				float RadiusofArena = sqrtf(halfArenaX * halfArenaX + halfArenaY * halfArenaY);

				RadiusofArena *= 1.1;

				vec3f startPos(x * RadiusofArena, y * RadiusofArena, z);
				vec3f dirToShip(g_gameObjectPool.m_GlobalSpaceship->pos.x - startPos.x, g_gameObjectPool.m_GlobalSpaceship->pos.y - startPos.y, z);
				dirToShip = dirToShip.normalize();

				asteroid* anewasteroid = new asteroid;

				anewasteroid->speed = asteroidMinSpeed + GetRandom(asteroidMaxSpeed - asteroidMinSpeed);
				anewasteroid->pos = startPos;
				anewasteroid->dir = dirToShip;
				anewasteroid->radius = GetRandom(AsteroidRandomRadius) + AsteroidBaseRadius;
				anewasteroid->rColour = GetRandom(1);
				anewasteroid->gColour = GetRandom(1);
				anewasteroid->bColour = GetRandom(1);
				// not to make too dark
				if (anewasteroid->rColour < 0.3)
					anewasteroid->rColour = 1 - anewasteroid->rColour;
				if (anewasteroid->gColour < 0.3)
					anewasteroid->gColour = 1 - anewasteroid->gColour;
				if (anewasteroid->bColour < 0.3)
					anewasteroid->bColour = 1 - anewasteroid->bColour;
				// if asteroid's radius is larger or eqault to 2.5f, then make its energy 2
				if (anewasteroid->radius > 2.5f)
					anewasteroid->energy = 2;
				basepool.m_gameObjectList.push_back(anewasteroid);
			}
		}
		m_lastShootTime = g_TimeManger.Getsec();
		wave += 1;
		if (wave == 4)
			wave = 3;
	}
}

void arenaManager::Process() {
	const float halfX = xwidthofspace * ArenaXPercentage * 0.5f;
	const float halfY = yheightofspace * ArenaYPercentage * 0.5f;
	//check distance with Left 
	spaceship* ship = g_gameObjectPool.m_GlobalSpaceship;
	const float shipradius = g_gameObjectPool.m_GlobalSpaceship->radius;
	const float distanceOffset = 8.0f;
		
	// left wall
	if (abs(-halfX - (ship->pos.x + shipradius)) <= ArenaGameOverdistance) {
		vec3f reverseVec = vec3f(ship->dir.x * -1.0f, ship->dir.y, ship->dir.z);
		reverseVec = reverseVec.normalize();
		g_gameObjectPool.m_GlobalSpaceship->pos = ship->pos + reverseVec * distanceOffset;
		g_gameObjectPool.m_GlobalSpaceship->dir = reverseVec;
	}

	// right wall
	if (abs(halfX - (ship->pos.x + shipradius)) <= ArenaGameOverdistance) {
		vec3f reverseVec = vec3f(ship->dir.x * -1.0f, ship->dir.y, ship->dir.z);
		reverseVec = reverseVec.normalize();
		g_gameObjectPool.m_GlobalSpaceship->pos = ship->pos + reverseVec * distanceOffset;
		g_gameObjectPool.m_GlobalSpaceship->dir = reverseVec;
	}

	// back wall
	if (abs(-halfY - (ship->pos.y + shipradius)) <= ArenaGameOverdistance) {
		vec3f reverseVec = vec3f(ship->dir.x, ship->dir.y * -1.0f, ship->dir.z);
		reverseVec = reverseVec.normalize();
		g_gameObjectPool.m_GlobalSpaceship->pos = ship->pos + reverseVec * distanceOffset;
		g_gameObjectPool.m_GlobalSpaceship->dir = reverseVec;
	}

	// front wall
	if (abs(halfY - (ship->pos.y + shipradius)) <= ArenaGameOverdistance) {
		vec3f reverseVec = vec3f(ship->dir.x, ship->dir.y * -1.0f, ship->dir.z);
		reverseVec = reverseVec.normalize();
		g_gameObjectPool.m_GlobalSpaceship->pos = ship->pos + reverseVec * distanceOffset;
		g_gameObjectPool.m_GlobalSpaceship->dir = reverseVec;
	}
}

// natural light
const GLfloat ambientLight1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat diffuseLight1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat specular1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat light1Pos[] = { 0.0f, 0.0f, 6.0f, 1.0f };

void arenaManager::Render() {
	const float screenX = xwidthofspace * 0.5f;
	const float screenY = yheightofspace * 0.5f;
	const float halfX = xwidthofspace * ArenaXPercentage * 0.5f;
	const float halfY = yheightofspace * ArenaYPercentage * 0.5f;
	const float height = 6.0f;

	// draw Arena body
	const vec3f& shipos = g_gameObjectPool.m_GlobalSpaceship->pos;
	const float shipradius = g_gameObjectPool.m_GlobalSpaceship->radius;

	// Light for the entire objects in arena
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);

	// Light pos is the middle of the arena's ceiling
	
	glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
	glEnable(GL_LIGHT1);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight1);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight1);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular1);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);

	glPushMatrix();
	// left wall
	glBegin(GL_POLYGON);
	glColor4f(0.5f, 0.5f, 0.5f, 0.15f);
	if (abs(-halfX - (shipos.x + shipradius)) <= ArenaTooclosedistance)
		glColor4f(1, 0, 0, 0.7f);
		
	glVertex3f(-halfX, halfY, height);
	glVertex3f(-halfX, halfY, -height);
	glVertex3f(-halfX, -halfY, -height);
	glVertex3f(-halfX, -halfY, height);
	glEnd();
	glPopMatrix();

	g_arenaPosition[0].x = -halfX;

	glPushMatrix();
	// right wall
	glBegin(GL_POLYGON);
	glColor4f(0.5f, 0.5f, 0.5f, 0.15f);
	if (abs(halfX - (shipos.x + shipradius)) <= ArenaTooclosedistance)
		glColor4f(1, 0, 0, 0.7f);
		
	glVertex3f(halfX, -halfY, height);
	glVertex3f(halfX, -halfY, -height);
	glVertex3f(halfX, halfY, -height);
	glVertex3f(halfX, halfY, height);
	glEnd();
	glPopMatrix();

	g_arenaPosition[1].x = halfX;

	glPushMatrix();
	// back wall 
	glBegin(GL_POLYGON);
	glColor4f(0.5f, 0.5f, 0.5f, 0.15f);
	if (abs(-halfY - (shipos.y + shipradius)) <= ArenaTooclosedistance)
		glColor4f(1, 0, 0, 0.7f);
		
	glVertex3f(-halfX, -halfY, height);
	glVertex3f(-halfX, -halfY, -height);
	glVertex3f(halfX, -halfY, -height);
	glVertex3f(halfX, -halfY, height);
	glEnd();
	glPopMatrix();

	g_arenaPosition[1].y = -halfY;

	glPushMatrix();
	// front wall
	glBegin(GL_POLYGON);
	glColor4f(0.5f, 0.5f, 0.5f, 0.15f);
	if (abs(halfY - (shipos.y + shipradius)) <= ArenaTooclosedistance)
		glColor4f(1, 0, 0, 0.7f);
		
	glVertex3f(halfX, halfY, height);
	glVertex3f(halfX, halfY, -height);
	glVertex3f(-halfX, halfY, -height);
	glVertex3f(-halfX, halfY, height);
	glEnd();

	g_arenaPosition[0].y = halfY;

	glPopMatrix();
}

void draw_textured_quad() {
	// calculate UV coordinates for each vertex
	const float uv_start_x = 0.0f;
	const float uv_start_y = 0.0f;
	const float uv_width = 1.0f;
	const float uv_height = 1.0f;

	glBegin(GL_POLYGON);
	glTexCoord2f(uv_start_x, uv_start_y);
	glVertex2f(-1, -1);

	glTexCoord2f(uv_start_x + uv_width, uv_start_y);
	glVertex2f(1, -1);

	glTexCoord2f(uv_start_x + uv_width, uv_start_y + uv_height);
	glVertex2f(1, 1);

	glTexCoord2f(uv_start_x, uv_start_y + uv_height);
	glVertex2f(-1, 1);
	glEnd();
}

void skyboxManager::Render() {
	glPushMatrix();
	// enable textures, disable depth testing
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);

	const float scale = 300.0f;
	// front
	glBindTexture(GL_TEXTURE_2D, g_gameObjectPool.m_GlobalSkybox->skybox_front_texture);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -scale);
	glScalef(scale, scale, scale);
	draw_textured_quad();
	glPopMatrix();

	// back
	glBindTexture(GL_TEXTURE_2D, g_gameObjectPool.m_GlobalSkybox->skybox_back_texture);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, scale);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glScalef(scale, scale, scale);
	draw_textured_quad();
	glPopMatrix();

	// left
	glBindTexture(GL_TEXTURE_2D, g_gameObjectPool.m_GlobalSkybox->skybox_left_texture);
	glPushMatrix();
	glTranslatef(-scale, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(scale, scale, scale);
	draw_textured_quad();
	glPopMatrix();

	// right
	glBindTexture(GL_TEXTURE_2D, g_gameObjectPool.m_GlobalSkybox->skybox_right_texture);
	glPushMatrix();
	glTranslatef(scale, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, -1.0f, 0.0f);
	glScalef(scale, scale, scale);
	draw_textured_quad();
	glPopMatrix();

	// up
	glBindTexture(GL_TEXTURE_2D, g_gameObjectPool.m_GlobalSkybox->skybox_up_texture);
	glPushMatrix();
	glTranslatef(0.0f, scale, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glScalef(scale, scale, scale);
	draw_textured_quad();
	glPopMatrix();

	// down
	glBindTexture(GL_TEXTURE_2D, g_gameObjectPool.m_GlobalSkybox->skybox_down_texture);
	glPushMatrix();
	glTranslatef(0.0f, -scale, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glScalef(scale, scale, scale);
	draw_textured_quad();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);

	// restore stuff
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDepthMask(true);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColor4f(1, 1, 1, 1);
	glPopMatrix();
}

void RestartGame() {
	// delete all the objects
	for (std::list<gameObject*>::iterator it = g_gameObjectPool.m_gameObjectList.begin(); it != g_gameObjectPool.m_gameObjectList.end(); it++)
		delete* it;

	g_gameObjectPool.m_gameObjectList.clear();

	// make a new spaceship
	spaceship* s = new spaceship();
	g_gameObjectPool.m_GlobalSpaceship = s;
	g_gameObjectPool.m_gameObjectList.push_back(s);

	g_AsteroidLauncher.m_DestroyedAsteroid = 0;
	g_TimeManger.SetStartTime();

	// reset drones
	g_droneManager.howManyDronesSpawned = 0;
	g_droneManager.droneInArena = 0;

	// reset wave
	wave = 0;
}

#define GL_CLAMP_TO_EDGE 0x812F

// load image texture
unsigned int load_texture(const char* filename) {
	int width, height, components;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename, &width, &height, &components, STBI_rgb_alpha);

	if (!data) {
		fprintf(stderr, "failed to load image %s\nreason - %s\n", filename, stbi_failure_reason());
		exit(0);
	}

	unsigned int id;
	glPushAttrib(GL_TEXTURE_BIT);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glPopAttrib();
	glBindTexture(GL_TEXTURE_2D, 0);

	// deallocate CPU memory - stuff is in GPU memory now
	stbi_image_free(data);

	return id;
}

#endif