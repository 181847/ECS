// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <ECS\ComponentManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>

DECLARE_TEST_UNITS;


struct TestStructA
{
public:
	int dataA;
	float dataB;
	TestStructA() :dataA(1), dataB(2.222f) {}
};

#define DECLARE_TESTSTRUCT_A_COMPONENT_MANAGER(managerName, maxSize)\
	ECS::ComponentManager<TestStructA>* managerName(maxSize);

namespace TestUnit
{
	void GetReady()
	{

	}

	void AfterTest()
	{

	}

	void AddTestUnit()
	{
		TEST_UNIT_START("always success test unit")

			return true;
		TEST_UNIT_END;
	}
}


int main()
{

	TestUnit::testMain();

	return 0;
}

