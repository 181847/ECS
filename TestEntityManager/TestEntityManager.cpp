// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>

DECLARE_TEST_UNITS;

#define DeclareEntityManager(managerName)\
	ECS::EntityManager* managerName = ECS::EntityManager::getInstance()


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

	TEST_UNIT_START("test checkInvalid EntityID")
		DeclareEntityManager(eManager);

		// the test must ensure that the entityManager is empty.
		if (NOT_EQ(0, eManager->getUsedIDCount()))
		{
			return false;
		}
		std::vector<ECS::EntityID> idList;
		const size_t entityCount = 500;
		// new entityID
		errorLogger += newEntitis(eManager, &idList, entityCount);

		RandomTool::RandomSet<size_t> randomIndices;
		randomIndices.setSeed(1);
		// we generate a random indices for the idList from 0 to entityCount - 1
		randomIndices.randomSequence(entityCount);

		// use an array to indicate the corresponding id in the idList is valid.
		std::vector<bool> validFlagList(entityCount);
		for (auto & vFlag : validFlagList)
		{
			vFlag = true;
		}

		// seperate the idList to two part, 
		// the first part of the idList 
		// whose index started from 0 to deletePartCount will be delete.
		const size_t deletePartCount = 200;
		std::vector<ECS::EntityID> deletePartIDList;

		for (int dIndex = 0; dIndex < deletePartCount; ++dIndex)
		{
			deletePartIDList.push_back(idList[ randomIndices[dIndex] ]);
			// toggle the flag
			validFlagList[randomIndices[dIndex]] = false;
		}

		// delete the seperate entityID
		errorLogger += destoryEntities(eManager, &deletePartIDList);

		// now here is the main part
		// iterate all the idList,
		// check the EntityManager::isValid() is same as deleteFlagList.
		for (int idIndex = 0; idIndex < idList.size(); ++idIndex)
		{
			errorLogger += NOT_EQ(
				validFlagList[idIndex], 
				eManager->isValid(idList[idIndex]));
		}

		// for clean the entityManager
		// destory the rest entityID
		for (int idIndex = 0; idIndex < idList.size(); ++idIndex)
		{
			if (validFlagList[idIndex])
			{
				errorLogger += NOT_EQ(true, eManager->destoryEntity(idList[idIndex]));
			}
		}

		return errorLogger.conclusion();
	TEST_UNIT_END;
}// function void AddTestUnit()


}// namespace TestUnit


int main()
{

	TestUnit::testMain();

    return 0;
}

