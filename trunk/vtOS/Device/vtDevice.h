#pragma once

class vtDeviceBase
{
public:
	vtDeviceBase(void);
	virtual ~vtDeviceBase(void);
};

class vtDevice : public vtDeviceBase
{
public:
	vtDevice(void);
	virtual ~vtDevice(void);
};
