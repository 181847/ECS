// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include "TestComponentTypes.h"
#include <iostream>

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

template<typename ...COMPONENT_TYPES>
inline int maskIdListWithComponentTypes(
	ECS::EntityManager* pManager,
	const std::vector<ECS::EntityID>& idList)
{
	int error = 0;
	for (auto & id : idList)
	{
		error += NOT_EQ(ECS::MaskResultFlag::Success, 
			pManager->maskComponentType<COMPONENT_TYPES...>(id));
	}
	return error;
}

struct CheckComponentsResult
{
public:
	// how many components have been iterated.
	size_t	count = 0;
	// how many error happends.
	int		error = 0;

	CheckComponentsResult& operator += (const CheckComponentsResult& another)
	{
		this->count += another.count;
		this->error += another.error;
		return *this;
	}
};

template<typename ...CHECK_COMPONENT_TYPES>
inline CheckComponentsResult HelpTraverseEntities(ECS::EntityManager* pManager)
{
	CheckComponentsResult resultSum;
	auto range = pManager->RangeEntities<CHECK_COMPONENT_TYPES...>();

	for (auto id : range)
	{
		bool result = pManager->haveComponent<CHECK_COMPONENT_TYPES...>(id);
		++resultSum.count;
		resultSum.error += NOT_EQ(true, result);
	}

	return resultSum;
}

// a struct used in the function 'maskIdListWithComponentContainer()'
// to allow multiple component used in one function call.
template<typename ...COMPONENT_TYPES>
struct ComponentTypeContainer {
public:
	static inline int maskIdList(
		ECS::EntityManager* pManager,
		const std::vector<ECS::EntityID>& idList)
	{
		return maskIdListWithComponentTypes<COMPONENT_TYPES...>(pManager, idList);
	}

	static CheckComponentsResult HelpCheck(
		ECS::EntityManager* pManager)
	{
		return HelpTraverseEntities<COMPONENT_TYPES...>(pManager);
	}
};
// make the name of ComponentTypeContainer shorter.
template<typename...CMP_TYPES> using CMPS =
ComponentTypeContainer<CMP_TYPES...>;

template<typename ...CMP_CONTAINER>
inline int maskIdListWithComponentContainer(
	ECS::EntityManager* pManager,
	const std::vector<std::vector<ECS::EntityID>>& idListArray)
{
	int error = 0;
	
	if (sizeof...(CMP_CONTAINER) != idListArray.size())
		return 1;

	size_t idListNumber = 0;
	
	bool arg[] = {(error += CMP_CONTAINER::maskIdList(pManager, idListArray[idListNumber++]), false)...};
	return error;
}

template<typename...CMP_CONTAINER>
inline CheckComponentsResult checkMaskResultWithComponentContainer(
	ECS::EntityManager* pManager)
{
	CheckComponentsResult retResult;
	bool arg[] = { (retResult += CMP_CONTAINER::HelpCheck(pManager), false)... };
	return retResult;
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

			using MaskResultFlag = ECS::MaskResultFlag;
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

			using MaskResultFlag = ECS::MaskResultFlag;
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
			//auto entIter = eManager->RangeEntities<IntComponent, FloatComponent>();
			//auto mask = entIter._desirMask;
			std::vector<ECS::EntityID> idList1;
			const size_t idCount1 = 60;
			std::vector<ECS::EntityID> idList2;
			const size_t idCount2 = 50;
			std::vector<ECS::EntityID> idList3;
			const size_t idCount3 = 60;
			std::vector<ECS::EntityID> idList4;
			const size_t idCount4 = 60;
			newEntitis(eManager, &idList1, idCount1);
			newEntitis(eManager, &idList2, idCount2);
			newEntitis(eManager, &idList3, idCount3);
			newEntitis(eManager, &idList4, idCount4);

			errorLogger += destoryEntities(eManager, &idList1);
			errorLogger += destoryEntities(eManager, &idList3);

			using MaskFlag = ECS::MaskResultFlag;

			errorLogger.LogIfNotEq(MaskFlag::Success,
				eManager->maskComponentType<IntComponent, FloatComponent>(idList2[4]));

			errorLogger.LogIfNotEq(MaskFlag::Success,
				eManager->maskComponentType<IntComponent, FloatComponent>(idList2[8]));

			errorLogger.LogIfNotEq(MaskFlag::Success,
				eManager->maskComponentType<IntComponent, CharComponent>(idList4[15]));

			auto range1 = eManager->RangeEntities<IntComponent, FloatComponent>();
			auto range2 = eManager->RangeEntities<IntComponent, CharComponent>();

			auto be1 = range1.begin();
			auto be2 = range2.begin();

			errorLogger += NOT_EQ(*be1, idList2[4]);
			errorLogger += NOT_EQ(*be2, idList4[15]);

			/*for (auto id : range1)
			{
				std::cout << "range 1 id: " << id << std::endl;
			}*/

			errorLogger += destoryEntities(eManager, &idList2);
			errorLogger += destoryEntities(eManager, &idList4);

			return errorLogger.conclusion();
		TEST_UNIT_END;
	}

	// test Unit test traverse the entity.
	{
		TEST_UNIT_START("test traverse the entity.")
			DeclareEntityManager(eManager);
			
			const size_t idListSizeSeed = 1;
			const size_t dispatchIDSeed = 2;
			
			// multiply idLists
			const size_t idListArrayCount	= 8;
			// for each idList set a random size for it.
			RandomTool::RandomSet<size_t> randomIdListSize;
			randomIdListSize.setSeed(idListSizeSeed);
			randomIdListSize.randomNumbers(idListArrayCount, 64, 128);

			// allocate all the IDs for later use.
			size_t totalIDCount = 0;
			for (auto size : randomIdListSize)
			{
				totalIDCount += size;
			}
			std::vector<ECS::EntityID> totalIDList;
			newEntitis(eManager, &totalIDList, totalIDCount);

			// ready to randomly dispatch all the id to different idList
			RandomTool::RandomSet<size_t> randomIDDisaptchIndices;
			randomIDDisaptchIndices.setSeed(dispatchIDSeed);
			randomIDDisaptchIndices.randomSequence(totalIDCount);
			// init several idList
			std::vector<std::vector<ECS::EntityID>> idListArray(idListArrayCount);
			
			// dispatch the ids
			size_t randomDispatchIndexLocation = 0;
			for (size_t idListNumber = 0; idListNumber < idListArrayCount; ++ idListNumber)
			{
				for (size_t dispatchCount = 0; dispatchCount < randomIdListSize[idListNumber]; ++dispatchCount)
				{
					// get the id.
					ECS::EntityID id = totalIDList[randomIDDisaptchIndices[randomDispatchIndexLocation++]];
					// dispatch it.
					idListArray[idListNumber].push_back(id);
				}
			}

			errorLogger +=
				maskIdListWithComponentContainer
				<
				CMPS<>,
				CMPS< IntComponent>,
				CMPS< FloatComponent>,
				CMPS< CharComponent>,
				CMPS< IntComponent,		FloatComponent>,
				CMPS< IntComponent,		CharComponent>,
				CMPS< FloatComponent,	CharComponent>,
				CMPS< IntComponent,		FloatComponent,	CharComponent>
				>
				(eManager, idListArray);

			enum ComponentNumber
			{
				None = 0,
				IntC,
				FloatC,
				CharC,
				Int_FloatC,
				Int_CharC,
				Float_CharC,
				Int_Float_CharC = 7
			};

			auto autoCheckComponentMask =
				[&errorLogger, &idListArray](std::vector<size_t> numberList, CheckComponentsResult result)
			{
				size_t desiredCount = 0;
				for (auto number : numberList)
				{
					desiredCount += idListArray[number].size();
				}
				errorLogger += result.error;
				errorLogger.LogIfNotEq(result.count, desiredCount);
			};

			// check all the entity that have IntComponent.
			autoCheckComponentMask(
				{
				ComponentNumber::IntC,
				ComponentNumber::Int_FloatC,
				ComponentNumber::Int_CharC,
				ComponentNumber::Int_Float_CharC 
				},
				checkMaskResultWithComponentContainer
				<CMPS<IntComponent>>(eManager));

			// check all the entity that have FloatComponent.
			autoCheckComponentMask(
				{
				ComponentNumber::FloatC,
				ComponentNumber::Int_FloatC,
				ComponentNumber::Float_CharC,
				ComponentNumber::Int_Float_CharC
				},
				checkMaskResultWithComponentContainer
				<CMPS<FloatComponent>>(eManager));

			// check all the entity that have CharComponent.
			autoCheckComponentMask(
				{
				ComponentNumber::CharC,
				ComponentNumber::Int_CharC,
				ComponentNumber::Float_CharC,
				ComponentNumber::Int_Float_CharC
				},
				checkMaskResultWithComponentContainer
				<CMPS<CharComponent>>(eManager));

			// check all the entity that have Int and Float Component.
			autoCheckComponentMask(
				{
				ComponentNumber::Int_FloatC,
				ComponentNumber::Int_Float_CharC
				},
				checkMaskResultWithComponentContainer
					<CMPS<IntComponent, FloatComponent>>(eManager));
			

			// check all the entity that have Int and Char Component.
			autoCheckComponentMask(
				{
				ComponentNumber::Int_CharC,
				ComponentNumber::Int_Float_CharC
				},
				checkMaskResultWithComponentContainer
					<CMPS<IntComponent, CharComponent>>(eManager));

			// check all the entity that have Float and Char Component.
			autoCheckComponentMask(
				{
				ComponentNumber::Float_CharC,
				ComponentNumber::Int_Float_CharC
				},
				checkMaskResultWithComponentContainer
					<CMPS<FloatComponent, CharComponent>>(eManager));

			// check all the entity that have Float and Char Component.
			autoCheckComponentMask(
				{
				ComponentNumber::Int_Float_CharC
				},
				checkMaskResultWithComponentContainer
				<CMPS<IntComponent, FloatComponent, CharComponent>>(eManager));

			destoryEntities(eManager, &totalIDList);
			
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

