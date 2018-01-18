// TestECSWorld.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\ECSWorld.h>
#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include <iostream>
#include <TestEntityManager\TestComponentTypes.h>
#include <TestEntityManager\TestEntityManagerTool.h>
#include <vector>

DECLARE_TEST_UNITS;

#define DeclareEntityManager(managerName)\
	ECS::EntityManager* managerName = ECS::EntityManager::getInstance()

const size_t g_MaxIntComponent		= 1024;
const size_t g_MaxFloatComponent	= 512;
const size_t g_MaxCharComponent		= 2048;

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

#pragma region check testModules work
	TEST_UNIT_START("check testModules work")
		return errorLogger.conclusion();
	TEST_UNIT_END;
#pragma endregion

#pragma region declare ecs world
	TEST_UNIT_START("declare ecs world")
		ECS::ECSWorld<ECS::DefaultEntityManagerTraits, IntComponent, DoubleComponent, FloatComponent, CharComponent> testECSWorld;
		
	testECSWorld.Foreach<IntComponent, CharComponent>(
		[](ECS::EntityID id, auto*, auto*)->void {});

		return errorLogger.conclusion();
	TEST_UNIT_END;
#pragma endregion

}// function void AddTestUnit()


}// namespace TestUnit


int main()
{

	TestUnit::testMain();

	return 0;
}
