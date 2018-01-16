// TestECSWorld.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\ECSWorld.h>
#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include <iostream>
#include <TestEntityManager\TestComponentTypes.h>
#include <TestEntityManager\TestEntityManagerTool.h>

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
		ECS::ECSWorld::newComponentManager<IntComponent>(g_MaxIntComponent);
		ECS::ECSWorld::newComponentManager<FloatComponent>(g_MaxFloatComponent);
		ECS::ECSWorld::newComponentManager<CharComponent>(g_MaxCharComponent);
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

#pragma region get the component Manager exist, and get none exist manager
		TEST_UNIT_START("get the component Manager exist, and get none exist manager")
			auto * pIntComponentManager		= ECS::ECSWorld::getComponentManager<IntComponent>();
			auto * pFloatComponentManager	= ECS::ECSWorld::getComponentManager<FloatComponent>();
			auto * pCharComponentManager	= ECS::ECSWorld::getComponentManager<CharComponent>();

			auto * pUnexistComponetManager	= ECS::ECSWorld::getComponentManager<DoubleComponent>();

			errorLogger.LogIfEq(nullptr, pIntComponentManager);
			errorLogger.LogIfEq(nullptr, pFloatComponentManager);
			errorLogger.LogIfEq(nullptr, pCharComponentManager);


			errorLogger.LogIfNotEq(nullptr, pUnexistComponetManager);

			errorLogger.LogIfNotEq(g_MaxIntComponent,	pIntComponentManager	->getMaxSize());
			errorLogger.LogIfNotEq(g_MaxFloatComponent, pFloatComponentManager	->getMaxSize());
			errorLogger.LogIfNotEq(g_MaxCharComponent,	pCharComponentManager	->getMaxSize());
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

