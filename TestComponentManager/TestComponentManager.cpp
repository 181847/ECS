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
	static const int initDataA;
	static const float initDataB;
public:
	int dataA;
	float dataB;
	TestStructA() :dataA(initDataA), dataB(initDataB) {}
	TestStructA(int initDataA, float initDataB)
		:dataA(initDataA), dataB(initDataB){}
	~TestStructA()
	{
		// add one to the deconstructCount
		++deconstructCount;
	}
};

int TestStructA::deconstructCount = 0;
const int TestStructA::initDataA = 2;
const float TestStructA::initDataB = 2.222;

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


// the main process is to allocate entityID
// and get a new component with the id, 
// then random delete components
// finally delete th entityIDs
// it will allocate all the entityID 
template<typename COMPONENT_TYPE>
int randomTestCompnets(
	ECS::EntityManager* eManager,
	ECS::ComponentManager<COMPONENT_TYPE>& cManager,
	size_t entityCount,
	size_t loopTime = 20, 
	bool randomPerBatch = false, 
	int randSeed = 1)
{
	TestUnit::ErrorLogger			errorLogger;
	std::vector<ECS::EntityID>		idList;			// store the entityIDs
	std::vector<COMPONENT_TYPE*>		cmpList;		// store the components pointer
	const size_t maxCmpSize = cManager.getMaxSize();

	// get the entityIDs
	newEntitis(eManager, &idList, maxCmpSize);
	RandomTool::RandomSet<size_t> randomIndices;
	RandomTool::RandomSet<size_t> deleteIndices;

	// set seed
	randomIndices.setSeed(randSeed);
	// make the delete indices different
	deleteIndices.setSeed(randSeed + 1);

	// init random indices
	randomIndices.randomSequence(maxCmpSize);
	deleteIndices.randomSequence(maxCmpSize);

	// loop allocat components, ensure ensure
	// to use the components out.
	for (size_t loopCount = 0; loopCount < loopTime; ++loopCount)
	{
		// each start, ensure the mananger don't allocate any component
		errorLogger += NOT_EQ(0, cManager.getUsedCount());
		// the random entities to get the components.
		if (randomPerBatch)
		{
			randomIndices.tickSeed();
			randomIndices.randomSequence(maxCmpSize);
		}
		
		// new components
		for (auto idIndex : randomIndices)
		{
			ECS::EntityID randID = idList[idIndex];
			auto * newCmp = cManager.newComponnet(randID);
			if (EQ(newCmp, nullptr))
			{
				break;
			}
			cmpList.push_back(newCmp);
		}

		// check each component is correct
		for (auto cmp : cmpList)
		{
			errorLogger += NOT_EQ(TestStructA::initDataA, cmp->dataA);
			errorLogger += NOT_EQ(TestStructA::initDataB, cmp->dataB);
		}

		// all the components should be used out
		errorLogger += NOT_EQ(cManager.getUsedCount(), cManager.getMaxSize());

		if (randomPerBatch)
		{
			deleteIndices.tickSeed();
			deleteIndices.randomSequence(maxCmpSize);
		}
		// delete all the components
		for (auto idIndex : deleteIndices)
		{
			ECS::EntityID randID = idList[idIndex];
			errorLogger += NOT_EQ(true, cManager.removeComponent(randID));
		}

		cmpList.clear();
	}

	destoryEntities(eManager, &idList);

	return errorLogger.getErrorCount();
	
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
				errorLogger += NOT_EQ(TestStructA::initDataA, cmpList[cmpIndex]->dataA);
				errorLogger += NOT_EQ(TestStructA::initDataB, cmpList[cmpIndex]->dataB);
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

		// can the entityID be set into the component?
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

			const size_t entityCount = 100;				// available entities
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

		// random test on componentManager
		{
		TEST_UNIT_START("random test on componentManager")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			errorLogger += randomTestCompnets(etManager, tacManager, gComponentMaxSize, 20, true);
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

