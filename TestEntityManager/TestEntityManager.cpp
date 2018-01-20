// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include "TestComponentTypes.h"
#include <iostream>
#include "TestEntityManagerTool.h"

DECLARE_TEST_UNITS;


using TestEntityManager = ECS::EntityManager<TestEntityTraits>;
using IDList			= std::vector<ECS::EntityID>;

namespace
{
	const int TestEntityCount = 100;
}

#define DECLARE_ENTITY_MANAGER(managerName)\
	auto * managerName = TestEntityManager::getInstance();\
	assert(managerName->getUsedIDCount() == 0 && "the test must ensure that the entityManager is empty.")


namespace TestUnit
{

void GetReady(){}

void AfterTest(){}

void AddTestUnit()
{
#pragma region test Unit always success test unit
	TEST_UNIT_START("always success test unit")

		return true;
	TEST_UNIT_END;
#pragma endregion

#pragma region check IDGenerator::IDOf()
	TEST_UNIT_START("check IDGenerator's newID() and getID()")
		// a temp struct to create a series id.
		struct TestIDGenStruct {};
		using TestIDGen = TypeTool::IDGenerator<TestIDGenStruct>;

		// first part, try first newID(), then getID().
		errorLogger.LogIfNotEq(1, TestIDGen::IDOf<int>());
		errorLogger.LogIfNotEq(2, TestIDGen::IDOf<float>());
		errorLogger.LogIfNotEq(1, TestIDGen::IDOf<int>());
		errorLogger.LogIfNotEq(2, TestIDGen::IDOf<float>());
	TEST_UNIT_END;
#pragma endregion

#pragma region creat EntityManager
	TEST_UNIT_START("creat EntityManager")
		DECLARE_ENTITY_MANAGER(eManager);
		
		ECS::EntityID firstID = eManager->newEntity();

		errorLogger.LogIfNotEq(firstID, 1);

		errorLogger.LogIfNotEq(1, eManager->getUsedIDCount());

		eManager->destoryEntity(firstID);

		errorLogger.LogIfNotEq(0, eManager->getUsedIDCount());
	TEST_UNIT_END;
#pragma endregion

#pragma region generate ID, and random destroy
	TEST_UNIT_START("generate ID, and random destroy")

		DECLARE_ENTITY_MANAGER(eManager);

		IDList idList;
		const int TestEntityCount = eManager->getSize();

		for (int i = 0; i < 20; ++i)
		{
			// Ensure that the eManager is empty.
			errorLogger.LogIfNotEq(0, eManager->getUsedIDCount());

			EntityManagerTool::	NewEntities(eManager, idList, TestEntityCount);

			RandomTool::Func::	Shuffle(idList, i);


			// Ensure that the eManager no more id left.
			errorLogger.LogIfNotEq(eManager->getSize(), eManager->getUsedIDCount());

			EntityManagerTool::	DeleteEntities(eManager, idList);
		}

	TEST_UNIT_END;
#pragma endregion

#pragma region test checkInvalid EntityID
	TEST_UNIT_START("test checkInvalid EntityID")

		DECLARE_ENTITY_MANAGER(eManager);
		
		const int TestEntityCount = eManager->getSize();
		const int halfSplit = TestEntityCount / 2;

		IDList idList		(TestEntityCount);
		IDList validList	(halfSplit);
		IDList inValidList	(TestEntityCount - halfSplit);

		auto DivideIDsFunc = [&]() 
		{
			validList.clear();
			inValidList.clear();

			for (int i = idList.size() - 1; i >= 0; --i)
			{
				if (i < halfSplit)
				{
					validList.push_back(idList[i]);
				}
				else
				{
					inValidList.push_back(idList[i]);
				}
			}
		};

		auto CheckIDValidationFunc = [&](const IDList& list, bool shouldBeValid)
		{
			for (auto id : list)
			{
				errorLogger.LogIfNotEq(shouldBeValid, eManager->isValid(id));
			}
		};

		for (int i = 0; i < 20; ++i)
		{
			EntityManagerTool::NewEntities(eManager, idList, TestEntityCount);

			RandomTool::Func::Shuffle(idList, i);

			DivideIDsFunc();

			EntityManagerTool::DeleteEntities(eManager, inValidList);

			CheckIDValidationFunc(validList, true);
			CheckIDValidationFunc(inValidList, false);

			errorLogger.LogIfNotEq(validList.size(), eManager->getUsedIDCount());

			EntityManagerTool::DeleteEntities(eManager, validList);

			errorLogger.LogIfNotEq(0, eManager->getUsedIDCount());
		}
		
	TEST_UNIT_END;
#pragma endregion

#pragma region test componentMask use EntityManager
	TEST_UNIT_START("test componentMask use EntityManager")
		DECLARE_ENTITY_MANAGER(eManager);

		IDList idList(78);
		IDList branch1(30);
		IDList branch2(48);
		std::vector<int> dummyList0, dummyList1, dummyList2;

		RandomTool::Func::Dispatch(dummyList0, dummyList1, dummyList2);

		/*
		using MaskResultFlag = ECS::MaskResultFlag;
		const size_t idCount = 5;
		const size_t partOneCount = 2;
		const size_t partTwoCount = idCount - partOneCount;

		std::vector<ECS::EntityID> idList;
		errorLogger += newEntitis(eManager, &idList, idCount);

		RandomTool::RandomSet<size_t> randomIndices;
		randomIndices.setSeed(1);
		randomIndices.RandomSequence(idCount);

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
	*/
	TEST_UNIT_END;
#pragma endregion

		/*
#pragma region get mask of components
		TEST_UNIT_START("get mask of components")
			auto mask0010 = TestEntityManager::getComponentMask<IntComponent>();
			auto mask1010 = TestEntityManager::getComponentMask<IntComponent, CharComponent>();
			auto mask1100 = TestEntityManager::getComponentMask<CharComponent, FloatComponent>();
			auto mask1110 = TestEntityManager::getComponentMask<IntComponent, FloatComponent, CharComponent>();

			errorLogger += NOT_EQ(mask0010, 2);
			errorLogger += NOT_EQ(mask1010, 10);
			errorLogger += NOT_EQ(mask1100, 12);
			errorLogger += NOT_EQ(mask1110, 14);
		TEST_UNIT_END;
#pragma endregion

#pragma region check componentType of the entity
		TEST_UNIT_START("check componentType of the entity")
			DECLARE_ENTITY_MANAGER(eManager);

			using MaskResultFlag = ECS::MaskResultFlag;
			const size_t idCount = 5;
			const size_t partOneCount = 2;
			const size_t partTwoCount = idCount - partOneCount;

			std::vector<ECS::EntityID> idList;
			errorLogger += newEntitis(eManager, &idList, idCount);

			RandomTool::RandomSet<size_t> randomIndices;
			randomIndices.setSeed(1);
			randomIndices.RandomSequence(idCount);

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
#pragma endregion

#pragma region get entityIter
		TEST_UNIT_START("get entityIter")

			DECLARE_ENTITY_MANAGER(eManager);
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


			errorLogger += destoryEntities(eManager, &idList2);
			errorLogger += destoryEntities(eManager, &idList4);

			return errorLogger.conclusion();
		TEST_UNIT_END;
#pragma endregion

#pragma region test traverse the entity.
		TEST_UNIT_START("test traverse the entity.")
			DECLARE_ENTITY_MANAGER(eManager);

			std::vector<ECS::EntityID> totalIDList;
			std::vector<std::vector<ECS::EntityID>> idListArray;

			errorLogger += randomIdListArray(eManager, &totalIDList, &idListArray);

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
#pragma endregion
*/
}// function void AddTestUnit()


}// namespace TestUnit


int main()
{

	TestUnit::testMain();

    return 0;
}

