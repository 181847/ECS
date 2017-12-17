// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include "TestComponentTypes.h"

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
int destoryEntities(
	ECS::EntityManager* eManager, 
	std::vector<ECS::EntityID> * idList)
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

// random allocate entityID, and random delete them.
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
	ECS::ComponentIDGenerator::newID<IntComponent>();
	ECS::ComponentIDGenerator::newID<FloatComponent>();
	ECS::ComponentIDGenerator::newID<CharComponent>();
}

void AfterTest()
{

}

void AddTestUnit()
{
	// test Unit always success test unit
	{
	TEST_UNIT_START("always success test unit")

		return true;
	TEST_UNIT_END;
	}// test Unit always success test unit

	// test Unit check IDGenerator's newID() and getID()
	{
		TEST_UNIT_START("check IDGenerator's newID() and getID()")
			// a temp struct to create a series id.
			struct TestIDGenStruct {};
			using TestIDGen = TypeTool::IDGenerator<TestIDGenStruct>;

			// first part, try first newID(), then getID().
			errorLogger += NOT_EQ(1, TestIDGen::newID<int>());
			errorLogger += NOT_EQ(2, TestIDGen::newID<float>());
			errorLogger += NOT_EQ(1, TestIDGen::getID<int>());
			errorLogger += NOT_EQ(2, TestIDGen::getID<float>());

			// second part, first getID(), which should return zero.
			// and newID(), which also return zero.
			errorLogger += NOT_EQ(0, TestIDGen::getID<unsigned int>());
			errorLogger += NOT_EQ(0, TestIDGen::getID<unsigned char>());
			errorLogger += NOT_EQ(0, TestIDGen::newID<unsigned int>());
			errorLogger += NOT_EQ(0, TestIDGen::newID<unsigned char>());
			
			return errorLogger.conclusion();
		TEST_UNIT_END;
	}

	// test Unit create EntityManager
	{
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
	}// test Unit create EntityManager

	// test Unit generat ID, and random destoried
	{
	TEST_UNIT_START("generat ID, and random destoried")
		int error = 0;
		DeclareEntityManager(eManager);
		error += testEntityManager(eManager);
		error += testEntityManager(eManager, 300, true, 30);
		return error == 0;
	TEST_UNIT_END;
	}// test Unit generat ID, and random destoried

	// test Unit test checkInvalid EntityID
	{
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
		for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
		{
			errorLogger += NOT_EQ(
				validFlagList[idIndex], 
				eManager->isValid(idList[idIndex]));
		}

		// for clean the entityManager
		// destory the rest entityID
		for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
		{
			if (validFlagList[idIndex])
			{
				errorLogger += NOT_EQ(true, eManager->destoryEntity(idList[idIndex]));
			}
		}

		return errorLogger.conclusion();
	TEST_UNIT_END;
	}// test Unit test checkInvalid EntityID

	// test Unit test componentMask use EntityManager
	{
		TEST_UNIT_START("test componentMask use EntityManager")
			DeclareEntityManager(eManager);

			using MaskResultFlag = ECS::EntityManager::MaskResultFlag;
			const size_t idCount = 5;
			const size_t partOneCount = 2;
			const size_t partTwoCount = idCount - partOneCount;

			std::vector<ECS::EntityID> idList;
			errorLogger += newEntitis(eManager, &idList, idCount);

			RandomTool::RandomSet<size_t> randomIndices;
			randomIndices.setSeed(1);
			randomIndices.randomSequence(idCount);

			// part one will mask two type of component[IntComponet, FloatComponent].
			// part two will only mask with IntComponent.
			for (size_t randomIndexPosition = 0;
				randomIndexPosition < randomIndices.size();
				++randomIndexPosition)
			{
				ECS::MaskResult result;
				if (randomIndexPosition < partOneCount)
				{
					result = eManager->maskComponentType<IntComponent, FloatComponent>(idList[
							randomIndices[randomIndexPosition]]);
					errorLogger += NOT_EQ(result, MaskResultFlag::Success);
				}
				else
				{
					result = eManager->maskComponentType<IntComponent>(idList[
						randomIndices[randomIndexPosition]]);
					errorLogger += NOT_EQ(result, MaskResultFlag::Success);
				}
			}

			// part one will append [IntComponent, CharComponent], 
			//		which result should return Success | RedundancyComponent
			// part two will only mask with FloatComponent,
			//		which result should return Success.
			for (size_t randomIndexPosition = 0;
				randomIndexPosition < randomIndices.size();
				++randomIndexPosition)
			{
				ECS::MaskResult result;
				if (randomIndexPosition < partOneCount)
				{
					result = eManager->maskComponentType<IntComponent, CharComponent>(idList[
						randomIndices[randomIndexPosition]]);
					errorLogger += NOT_EQ(result, MaskResultFlag::Success | MaskResultFlag::RedundancyComponent);
				}
				else
				{
					result = eManager->maskComponentType<CharComponent>(idList[
						randomIndices[randomIndexPosition]]);
					errorLogger += NOT_EQ(result, MaskResultFlag::Success);
				}
			}

			errorLogger += destoryEntities(eManager, &idList);
			return errorLogger.conclusion();
		TEST_UNIT_END;
	}// test Unit test componentMask use EntityManager

	// test Unit get mask of components
	{
		TEST_UNIT_START("get mask of components")
			auto mask0010 = ECS::getComponentMask<IntComponent>();
			auto mask1010 = ECS::getComponentMask<IntComponent, CharComponent>();
			auto mask1100 = ECS::getComponentMask<CharComponent, FloatComponent>();
			auto mask1110 = ECS::getComponentMask<IntComponent, FloatComponent, CharComponent>();

			errorLogger += NOT_EQ(mask0010, 2);
			errorLogger += NOT_EQ(mask1010, 10);
			errorLogger += NOT_EQ(mask1100, 12);
			errorLogger += NOT_EQ(mask1110, 14);
			return errorLogger.conclusion();
		TEST_UNIT_END;
	}

	// test Unit check componentType of the entity
	{
		TEST_UNIT_START("check componentType of the entity")
			DeclareEntityManager(eManager);

			using MaskResultFlag = ECS::EntityManager::MaskResultFlag;
			const size_t idCount = 5;
			const size_t partOneCount = 2;
			const size_t partTwoCount = idCount - partOneCount;

			std::vector<ECS::EntityID> idList;
			errorLogger += newEntitis(eManager, &idList, idCount);

			RandomTool::RandomSet<size_t> randomIndices;
			randomIndices.setSeed(1);
			randomIndices.randomSequence(idCount);

			// part one will mask two type of component[IntComponet, FloatComponent].
			// part two will only mask with IntComponent.
			for (size_t randomIndexPosition = 0;
				randomIndexPosition < randomIndices.size();
				++randomIndexPosition)
			{
				ECS::MaskResult result;
				if (randomIndexPosition < partOneCount)
				{
					result = eManager->maskComponentType<IntComponent, FloatComponent>(idList[
							randomIndices[randomIndexPosition]]);
					errorLogger += NOT_EQ(result, MaskResultFlag::Success);
				}
				else
				{
					result = eManager->maskComponentType<IntComponent>(idList[
						randomIndices[randomIndexPosition]]);
					errorLogger += NOT_EQ(result, MaskResultFlag::Success);
				}
			}

			// part one will append [IntComponent, CharComponent], 
			//		which result should return Success | RedundancyComponent
			// part two will only mask with FloatComponent,
			//		which result should return Success.
			for (size_t randomIndexPosition = 0;
				randomIndexPosition < randomIndices.size();
				++randomIndexPosition)
			{
				//ECS::EntityID id = idList[randomIndices[randomIndexPosition]];
				ECS::EntityID id = idList[randomIndices[randomIndexPosition]];
				if (randomIndexPosition < partOneCount)
				{
					bool result = eManager->haveComponent<IntComponent, FloatComponent>(id);
					errorLogger += NOT_EQ(true, result);
					result = eManager->haveComponent<IntComponent, FloatComponent, CharComponent>(id);
					errorLogger += NOT_EQ(false, result);
				}
				else
				{
					bool result = eManager->haveComponent<IntComponent>(id);
					errorLogger += NOT_EQ(true, result);
					result = eManager->haveComponent<IntComponent, FloatComponent>(id);
					errorLogger += NOT_EQ(false, result);
				}
			}

			errorLogger += destoryEntities(eManager, &idList);
			return errorLogger.conclusion();
		TEST_UNIT_END;
	}

	// test Unit get entityIter.
	{
		TEST_UNIT_START("get entityIter")

			DeclareEntityManager(eManager);
			auto entIter = eManager->RangeEntities<IntComponent, FloatComponent>();
			//auto mask = entIter._desiredMask;

			return errorLogger.conclusion();
		TEST_UNIT_END;
	}
}// function void AddTestUnit()


}// namespace TestUnit


int main()
{

	TestUnit::testMain();

    return 0;
}

