#include "AutoCamera.h"
#include "Window.h"
#include <string>



AutoCamera::AutoCamera(void)
{
	timer = 0;
	travelTime = 5;
	yaw = 0.0f;
	pitch = 0.0f;
	Shot shot = { 0,0,Vector3(0,0,0), 0 };
	shots.emplace_back(shot);
	currentShot = shots.begin();
	paused = true;
}

AutoCamera::AutoCamera(float pitch, float yaw, Vector3 position, float time)
{
	this->pitch = pitch;
	this->yaw = yaw;
	this->position = position;
	timer = 0;
	travelTime = 5;
	Shot shot = { yaw,pitch,position, time };
	shots.emplace_back(shot);
	currentShot = shots.begin();
	paused = true;
}


float Lerp(float a, float b, float t)
{
	t = t > 1 ? 1 : t;
	return a + (t * (b - a));
}

Vector3 Lerp(Vector3 a, Vector3 b, float t)
{
	a.x = Lerp(a.x, b.x, t);
	a.y = Lerp(a.y, b.y, t);
	a.z = Lerp(a.z, b.z, t);
	return a;
}


void AutoCamera::UpdateCamera(float dt)
{
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		paused = !paused;

	}
	if (!paused)
	{
		timer += dt;
		if (position == currentShot->position)
		{
			if (timer > (*currentShot).time)
			{
				currentShot++;
				if (currentShot == shots.end())
				{
					currentShot = shots.begin();
				}
				timer = 0;
			}
		}
		else
		{
			float t = timer / travelTime;
			pitch = Lerp(pitch, currentShot->pitch, t);
			yaw = Lerp(yaw, currentShot->yaw, t);
			position = Lerp(position, currentShot->position, t);
			if (position == currentShot->position)
			{
				timer = 0;
			}
		}
	}
}

void AutoCamera::AddShot(Shot shot)
{
	shots.emplace_back(shot);
	std::cout << std::to_string(shots.size());
}
