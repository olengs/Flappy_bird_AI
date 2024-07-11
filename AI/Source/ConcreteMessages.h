#ifndef CONCRETE_MESSAGE_H
#define CONCRETE_MESSAGE_H

#include "Message.h"
#include "GameObject.h"

struct MessageWRU : public Message
{
	enum SEARCH_TYPE
	{
		SEARCH_NONE = 0,
		NEAREST_SHARK,
		NEAREST_FISHFOOD,
		NEAREST_FULLFISH,
		HIGHEST_ENERGYFISH,
	};
	MessageWRU(GameObject *goValue, SEARCH_TYPE typeValue, float thresholdValue) : go(goValue), type(typeValue), threshold(thresholdValue) {}
	virtual ~MessageWRU() {}

	GameObject *go;
	SEARCH_TYPE type;
	float threshold;
};

struct MessageCheckActive : public Message
{
	MessageCheckActive() {}
	virtual ~MessageCheckActive() {}
};

struct MessageCheckFish : public Message
{
	MessageCheckFish() {}
	virtual ~MessageCheckFish() {}
};

struct MessageSpawn : public Message
{
	MessageSpawn(GameObject* goVal, int typeVal, int countVal, int* range) : go(goVal), type(typeVal), count(countVal) 
	{
		distRange[0] = range[0];
		distRange[1] = range[1];
	}
	virtual ~MessageSpawn() {}

	int distRange[2];
	int type;
	int count;
	GameObject* go;
};

struct MessageStop : public Message
{
	MessageStop() {}
	virtual ~MessageStop() {}
};

struct MessageEvolve : public Message
{
	MessageEvolve(GameObject* goVal) : go(goVal) {}
	virtual ~MessageEvolve() {}

	GameObject* go;
};

#endif
