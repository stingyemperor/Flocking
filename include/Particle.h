#pragma once
#include "cinder/Channel.h"
#include "cinder/Vector.h"

#include <vector>

class Particle
{
public:
	Particle();
	Particle(ci::vec2);

	void update(const ci::Channel32f &channel, const ci::vec2 &mouseLoc);
	void draw();

	ci::vec2 mLoc;
	ci::vec2 mLocPer;
	ci::vec2 mDir;
	ci::vec2 mDirToCursor;
	float mVel;

	float mRadius;
	float mScale;
	float mColor;
};