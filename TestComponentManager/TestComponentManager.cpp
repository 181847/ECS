// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <ECS\ComponentManager.h>
#include <ECS\BaseComponent.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include <vector>

DECLARE_TEST_UNITS;

const int gComponentMaxSize = 1024;
#define DeclareComponentManager(managerName, ComponentType, MaxSize)\
	ECS::ComponentManager<ComponentType> managerName(MaxSize)

#define DeclareEntityManager(managerName)\
	ECS::EntityManager* managerName = ECS::EntityManager::getInstance()

struct TestStructA
	:public ECS::BaseComponent
{

	static int deconstructCount;
public:
	int dataA;
	float dataB;
	TestStructA() :dataA(1), dataB(2.222f) {}
	TestStructA(int initDataA, float initDataB)
		:dataA(initDataA), dataB(initDataB){}
	~TestStructA()
	{
		// add one to the deconstructCount
		++deconstructCount;
	}
};

int TestStructA::deconstructCount = 0;

// get batch Entities,
// return errors count, (if no error, return 0)
int newEntitis(
	ECS::EntityManager* eManager, 
	std::vector<ECS::EntityID> * idList, 
	const size_t count)
{
	int error = 0;

	idList->clear();
	for (size_t i = 0; i < count; ++i)
	{
		ECS::EntityID newID = eManager->newEntity();
		if (newID == 0)
		{
			++error;
			break;
		}
		idList->push_back(newID);
	}
	
	return error;
}

// destory the batch Entitys， and clear the idList.
// return errors count, (if no error, return 0)
int destoryEntities(ECS::EntityManager* eManager, std::vector<ECS::EntityID> * idList)
{
	int error = 0;
	for (ECS::EntityID id : *idList)
	{
		if (false == eManager->destoryEntity(id))
		{
			++error;
		}
	}
	idList->clear();
	return error;
}

template<typename COMPONENT_TYPE>
int newComponents(
	ECS::ComponentManager<COMPONENT_TYPE>& cManager,
	std::vector<ECS::EntityID>& idList,
	std::vector<COMPONENT_TYPE*> * compList)
{
	int error = 0;
	compList->clear();
	for (auto id : idList)
	{
		auto * newComp = cManager.newComponnet(id);
		if (nullptr == newComp)
		{
			error += 1;
			break;
		}
		compList->push_back(newComp);
	}
	return error;
}

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
		// test errorLogger, this is not the test about ComponentManager.
		{
		TEST_UNIT_START("test errorLogger, this is not the test about ComponentManager.")
			int error = 0;

			error += NOT_EQ(0, errorLogger.getErrorCount());

			errorLogger++;
			error += NOT_EQ(1, errorLogger.getErrorCount());

			++errorLogger;
			error += NOT_EQ(2, errorLogger.getErrorCount());

			errorLogger.tick();
			error += NOT_EQ(3, errorLogger.getErrorCount());

			errorLogger += 1;
			error += NOT_EQ(4, errorLogger.getErrorCount());

			errorLogger += 2;
			error += NOT_EQ(6, errorLogger.getErrorCount());

			errorLogger.addErrorCount(1);
			error += NOT_EQ(7, errorLogger.getErrorCount());

			errorLogger.addErrorCount(2);
			error += NOT_EQ(9, errorLogger.getErrorCount());
			
			return EQ(0, error);
		TEST_UNIT_END;
		}

		// create a ComponentManager with TestStructA
		{
		TEST_UNIT_START("create a ComponentManager with TestStructA")
			int error = 0;
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			const size_t entityCount = 6;
			// allocate the ids 
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> componentList;

			error += newComponents(tacManager, idList, &componentList);

			error += NOT_EQ(idList.size(), componentList.size());

			// deallcote the ids, because the etManager is global。
			errorLogger += destoryEntities(etManager, &idList);

			return error == 0;
		TEST_UNIT_END;
		}

		// get component from ComponentManager 
		{
		TEST_UNIT_START("get component from ComponentManager")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			const size_t entityCount = 6;
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> componentList;


			errorLogger += newComponents(tacManager, idList, &componentList);

			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				auto id = idList[idIndex];
				errorLogger += NOT_EQ(tacManager.getComponent(id), componentList[idIndex]);
			}

			errorLogger += destoryEntities(etManager, &idList);

			return errorLogger.conclusion();
		TEST_UNIT_END;
		}
		
		// get component from ComponentManager using the operator []
		{
		TEST_UNIT_START("get component from ComponentManager using the operator []")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			const size_t entityCount = 6;
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> componentList;


			errorLogger += newComponents(tacManager, idList, &componentList);

			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				auto id = idList[idIndex];
				errorLogger += NOT_EQ(tacManager[id], componentList[idIndex]);
			}

			errorLogger += destoryEntities(etManager, &idList);

			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// can pass the argument to the component constructor?
		{
		TEST_UNIT_START("can pass the argument to the component constructor?")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			size_t idIndex = 0;
			const size_t entityCount				= 5;				// avaliable entities
			const size_t defaultConstructorCount	= 3;				// the component use the default construct
			const size_t nonDefaultConstructorCount	= entityCount - 3;	// the component use the none default constructor
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> cmpList;
			int nonDefaultDataA = 3;
			float nonDefaultDataB = 4.2;
			
			// generate the components
			for (size_t defaultIndex = 0; defaultIndex < defaultConstructorCount; ++defaultIndex)
			{
				cmpList.push_back(tacManager.newComponnet(idList[idIndex++]));
			}
			for (size_t nonDefaultIndex = 0; nonDefaultIndex < nonDefaultConstructorCount; ++nonDefaultIndex)
			{
				cmpList.push_back(tacManager.newComponnet(idList[idIndex++], nonDefaultDataA, nonDefaultDataB));
			}

			// check components
			size_t cmpIndex = 0;
			// default constructor
			for (size_t defaultIndex = 0; defaultIndex < defaultConstructorCount; ++defaultIndex)
			{
				errorLogger += NOT_EQ(1, cmpList[cmpIndex]->dataA);
				errorLogger += NOT_EQ(2.222f, cmpList[cmpIndex]->dataB);
				++cmpIndex;
			}
			// none default constructor
			for (size_t nonDefaultIndex = 0; nonDefaultIndex < nonDefaultConstructorCount; ++nonDefaultIndex)
			{
				errorLogger += NOT_EQ(nonDefaultDataA, cmpList[cmpIndex]->dataA);
				errorLogger += NOT_EQ(nonDefaultDataB, cmpList[cmpIndex]->dataB);
				++cmpIndex;
			}
			
			errorLogger += destoryEntities(etManager, &idList);

			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// can the entityID be set into the componet?
		{
		TEST_UNIT_START("can the entityID be set into the componet?")

			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);
			RandomTool::RandomSet<size_t> randomIndices;
			std::vector<TestStructA*> cmpList;
			std::vector<ECS::EntityID> idList;

			const size_t entityCount = 100;				// avaliable entities
			// allocate the ids
			errorLogger += newEntitis(etManager, &idList, entityCount);
			// allocate the components
			errorLogger += newComponents(tacManager, idList, &cmpList);

			// for each id check that the id is same as the component
			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				errorLogger +=
					NOT_EQ(idList[idIndex], cmpList[idIndex]->getHostID());
			}
			// free all the id.
			errorLogger += destoryEntities(etManager, &idList);
			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// ensure that when we remove the entityID from the componentManager, the deconstructor is called
		{
		TEST_UNIT_START("ensure that when we remove the entityID from the componentManager, the deconstructor is called")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);
			int beforeDeconstructCount = TestStructA::deconstructCount;
			RandomTool::RandomSet<size_t> randomIndices;
			std::vector<TestStructA*> cmpList;
			std::vector<ECS::EntityID> idList;

			const size_t entityCount = 100;				// avaliable entities
			// allocate the ids
			errorLogger += newEntitis(etManager, &idList, entityCount);
			// allocate the components
			errorLogger += newComponents(tacManager, idList, &cmpList);

			// for each id check that the id is same as the component
			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				errorLogger += 
					NOT_EQ(idList[idIndex], cmpList[idIndex]->getHostID());
			}

			// free all the components
			for (auto id : idList)
			{
				errorLogger += NOT_EQ(true, tacManager.removeComponent(id));
			}

			// check the deallocat count
			int aftherDeconstructCount = TestStructA::deconstructCount;
			errorLogger += 
				NOT_EQ(entityCount, aftherDeconstructCount - beforeDeconstructCount);

			// free all the id.
			errorLogger += destoryEntities(etManager, &idList);
			return errorLogger.conclusion();
		TEST_UNIT_END;
		}
	}
}


int main()
{

	TestUnit::testMain();

	return 0;
}

