// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>

DECLARE_TEST_UNITS;

#define DeclareEntityManager(managerName)\
	ECS::EntityManager* managerName = ECS::EntityManager::getInstance()

namespace TestUnit
{

inline int testEntityManager(
	ECS::EntityManager* pManager, 
	const size_t batchSize = 100,
	const bool randomPerBatch = false,
	const size_t loopTime = 20,
	const size_t randSeed = 1)
{
	int error = 0;
	const size_t maxSize = pManager->getSize();
	std::vector<ECS::EntityID> idList;
	std::vector<size_t> randomIndex(maxSize);
	ECS::EntityID newID(0);

	for (size_t i = 0; i < loopTime; ++i)
	{
		// allocate
		for (size_t batchIndex = 0; batchIndex < std::min(batchSize, maxSize); ++batchIndex)
		{
			newID = pManager->newEntity();
			if (!newID)
			{// return zero mean the ids have been used out.
				break;
			}
			idList.push_back(newID);
		}

		// destory the entityID
		if (randomPerBatch)
		{
			RandomTool::RandomSequence(idList.size(), &randomIndex, i + randSeed);
		}
		else
		{
			RandomTool::RandomSequence(idList.size(), &randomIndex, randSeed);
		}
		
		for (auto randi : randomIndex)
		{
			pManager->destoryEntity(idList[randi]);
		}

		idList.clear();

		error += NOT_EQ(0, pManager->getUsedIDCount());
	}

	return error;
}

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

	TEST_UNIT_START("creat EntityManager")
		int error = 0;
		/*ECS::EntityManager* eManager = 
			ECS::EntityManager::getInstance();*/

		DeclareEntityManager(eManager);
		
		ECS::EntityID firstID = eManager->newEntity();

		error += firstID != 1;
		error += NOT_EQ(1, eManager->getUsedIDCount());

		eManager->destoryEntity(firstID);

		error += NOT_EQ(0, eManager->getUsedIDCount());

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("generat ID, and random destoried")
		int error = 0;
		DeclareEntityManager(eManager);
		error += testEntityManager(eManager);
		error += testEntityManager(eManager, 300, true, 30);
		return error == 0;
	TEST_UNIT_END;
}
}


int main()
{

	TestUnit::testMain();

    return 0;
}

