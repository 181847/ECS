#pragma once
#include <ECS\ECS.h>
#include <ECS\ComponentManager.h>
#include <ECS\EntityManager.h>

class IntComponent
	:public ECS::BaseComponent
{
public:
	int data;
};

class FloatComponet
	:public ECS::BaseComponent
{
public:
	float data;
};

class CharComponent
	:public ECS::BaseComponent
{
public:
	char data;
};
