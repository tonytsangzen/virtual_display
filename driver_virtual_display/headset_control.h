#pragma once
#include "openvr_driver.h"

class HeadsetControl
{
public:
	virtual bool connect(void) = 0;
	virtual bool setDisplayMode(int mode) = 0;
	virtual vr::HmdQuaternion_t getPose(void) = 0;

};

HeadsetControl* getNativeGlassInstance(void);