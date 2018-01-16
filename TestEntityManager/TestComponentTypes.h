#pragma once
#include <ECS\ECS.h>
#include <ECS\ComponentManager.h>
#include <ECS\EntityManager.h>

class IntComponent
	:public ECS::BaseComponent
{
public:
	IntComponent(int initD) :data(initD) {}
	int data;
	bool operator == (const IntComponent & other)
	{
		return this->data == other.data;
	}
};

class FloatComponent
	:public ECS::BaseComponent
{
public:
	FloatComponent(float initD) :data(initD) {}
	float data;
	bool operator == (const FloatComponent & other)
	{
		return this->data == other.data;
	}
};

class CharComponent
	:public ECS::BaseComponent
{
public:
	CharComponent(char initD) :data(initD) {}
	char data;
	bool operator == (const CharComponent & other)
	{
		return this->data == other.data;
	}
};

class DoubleComponent
	:public ECS::BaseComponent
{
public:
	DoubleComponent(double initD) :_data(initD) {}
	double _data;
	bool operator == (const DoubleComponent & other)
	{
		return this->_data == other._data;
	}
};

