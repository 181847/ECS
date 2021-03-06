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

		using TestECSWorldType = ECS::ECSWorld<ECS::DefaultEntityManagerTraits, IntComponent, DoubleComponent, FloatComponent, CharComponent>;
		TestECSWorldType testECSWorld;

		using namespace ECS;
		
		EntityID theFirstID = testECSWorld.NewEntity();
		
		// Add an IntComponent to the FirstID.
		testECSWorld.AttachTo<IntComponent>(theFirstID, 4);

		testECSWorld.Foreach<IntComponent>([&](EntityID id, auto * pInt) 
		{
			errorLogger.LogIfNotEq(id, theFirstID);
			errorLogger.LogIfNotEq(pInt->data, 4);
		});

		testECSWorld.Foreach<IntComponent>([&](auto id, auto * pInt)
		{
			pInt->data = 8;
		});

		testECSWorld.Foreach<IntComponent>([&](auto id, auto * pInt)
		{
			errorLogger.LogIfNotEq(id, theFirstID);
			errorLogger.LogIfNotEq(pInt->data, 8);
		});

		testECSWorld.ForOne<IntComponent>(theFirstID, [&](auto * pInt)
		{
			errorLogger.LogIfNotEq(pInt->data, 8);
		});

		errorLogger.LogIfNotEq(true, testECSWorld.DoHas<IntComponent>(theFirstID));
		errorLogger.LogIfNotEq(false, testECSWorld.DoHas<FloatComponent>(theFirstID));

		// detach the IntComponent.
		errorLogger.LogIfFalse(testECSWorld.DetachFrom<IntComponent>(theFirstID));

		// now there is no entity that have IntComponent, 
		// if correctly, next lambda should never be called, 
		// and the errorLogger will not increase any error count.
		testECSWorld.Foreach<IntComponent>([&errorLogger](auto id, auto * pInt)
		{
			errorLogger++;
		});
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
