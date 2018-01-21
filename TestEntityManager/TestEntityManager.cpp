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

		const int sizeSum = eManager->getSize();

		IDList idList(sizeSum);
		IDList branch1;
		IDList branch2;

		for (int i = 0; i < 20; ++i)
		{
			idList.resize(sizeSum);
			branch1.resize(sizeSum * (1 + i) / 21);
			branch2.resize(sizeSum - branch1.size());

			EntityManagerTool::NewEntities(eManager, idList, idList.size());

			RandomTool::Func::Shuffle(idList, i);

			RandomTool::Func::Dispatch(idList, branch1, branch2);

			EntityManagerTool::MaskComponent<IntComponent>(eManager, branch1);

			EntityManagerTool::MaskComponent<IntComponent, FloatComponent>(eManager, branch2);

			for (auto id : branch1)
			{
				errorLogger.LogIfNotEq	(true, eManager->haveComponent<IntComponent>(id));
				errorLogger.LogIfEq		(true, eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
				errorLogger.LogIfEq		(true, eManager->haveComponent<FloatComponent, DoubleComponent>(id));
			}

			for (auto id : branch2)
			{
				errorLogger.LogIfNotEq	(true, eManager->haveComponent<FloatComponent>(id));
				errorLogger.LogIfNotEq	(true, eManager->haveComponent<IntComponent>(id));
				errorLogger.LogIfNotEq	(true, eManager->haveComponent<IntComponent, FloatComponent>(id));

				errorLogger.LogIfEq		(true, eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
				errorLogger.LogIfEq		(true, eManager->haveComponent<CharComponent, FloatComponent>(id));
				errorLogger.LogIfEq		(true, eManager->haveComponent<IntComponent, CharComponent>(id));
			}

			EntityManagerTool::DeleteEntities(eManager, idList);
		}
		

	TEST_UNIT_END;
#pragma endregion

	/*
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

