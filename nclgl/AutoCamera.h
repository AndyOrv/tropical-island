#pragma once
#include "Camera.h"
#include <list>


//never fully implemented
struct Shot
{
	float yaw;
	float pitch;
	Vector3 position;
	float time;
};

class AutoCamera :
	public Camera
{
public:

	AutoCamera(void);
	AutoCamera(float pitch, float yaw, Vector3 position, float time = 0);
	void UpdateCamera(float dt = 1.0f);
	void AddShot(Shot shot);
	void Pause(){ paused = true; }
	void Play(){ paused = false; }

protected:
	std::list<Shot> shots;
	std::list<Shot>::iterator currentShot;
	bool paused;
	float timer;
	float travelTime;
};


