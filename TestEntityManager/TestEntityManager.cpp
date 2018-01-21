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

#pragma region test remove mask
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

		EntityManagerTool::MaskComponent<IntComponent>					(eManager, branch1);
		EntityManagerTool::MaskComponent<IntComponent, FloatComponent>	(eManager, branch2);

		for (auto id : branch1)
		{
			errorLogger.LogIfNotEq(true, eManager->haveComponent<IntComponent>(id));
			errorLogger.LogIfEq(true, eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfEq(true, eManager->haveComponent<FloatComponent, DoubleComponent>(id));
		}
		for (auto id : branch2)
		{
			errorLogger.LogIfNotEq(true, eManager->haveComponent<FloatComponent>(id));
			errorLogger.LogIfNotEq(true, eManager->haveComponent<IntComponent>(id));
			errorLogger.LogIfNotEq(true, eManager->haveComponent<IntComponent, FloatComponent>(id));

			errorLogger.LogIfEq(true, eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfEq(true, eManager->haveComponent<CharComponent, FloatComponent>(id));
			errorLogger.LogIfEq(true, eManager->haveComponent<IntComponent, CharComponent>(id));
		}

		// remove component mask
		EntityManagerTool::RemoveMask<IntComponent>		(eManager, branch1);
		EntityManagerTool::RemoveMask<FloatComponent>	(eManager, branch2);

		for (auto id : branch1)
		{
			// the branch1 should have no component.
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<FloatComponent, DoubleComponent>(id));
		}
		for (auto id : branch2)
		{
			// branch 2 should only have IntComponent.
			errorLogger.LogIfFalse(eManager->haveComponent<IntComponent>(id));

			errorLogger.LogIfTrue(eManager->haveComponent<FloatComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, CharComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<CharComponent, FloatComponent>(id));
		}

		// remove the IntComponent from branch2.
		EntityManagerTool::RemoveMask<IntComponent>(eManager, branch2);

		// check branch2's mask
		for (auto id : branch2)
		{
			// branch 2 should only have IntComponent.
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<FloatComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, CharComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<CharComponent, FloatComponent>(id));
		}

		// now all the masks have been removed, let's add them again.
		EntityManagerTool::MaskComponent<IntComponent, CharComponent>(eManager, branch1);
		EntityManagerTool::MaskComponent<IntComponent, FloatComponent>(eManager, branch2);

		// check the mask.
		for (auto id : branch1)
		{
			errorLogger.LogIfFalse(eManager->haveComponent<IntComponent, CharComponent>(id));
			errorLogger.LogIfFalse(eManager->haveComponent<IntComponent>(id));
			errorLogger.LogIfFalse(eManager->haveComponent<CharComponent>(id));

			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<FloatComponent, DoubleComponent>(id));
		}
		for (auto id : branch2)
		{
			errorLogger.LogIfFalse(eManager->haveComponent<IntComponent, FloatComponent>(id));
			errorLogger.LogIfFalse(eManager->haveComponent<IntComponent>(id));
			errorLogger.LogIfFalse(eManager->haveComponent<FloatComponent>(id));

			errorLogger.LogIfTrue(eManager->haveComponent<IntComponent, FloatComponent, DoubleComponent>(id));
			errorLogger.LogIfTrue(eManager->haveComponent<FloatComponent, DoubleComponent>(id));
		}

		EntityManagerTool::DeleteEntities(eManager, idList);
	}


	TEST_UNIT_END;
#pragma endregion

#pragma region test traverse the entity.
	TEST_UNIT_START("test traverse the entity.")
		DECLARE_ENTITY_MANAGER(eManager);

		const int sizeSum = eManager->getSize();

		IDList idList(sizeSum);
		IDList branch1;	// IntComponent
		IDList branch2;	// IntComponent - FloatComponent
		IDList branch3;	// IntComponent - FloatComponent - CharComponent

		enum {
			BRANCH_I = 1,
			BRANCH_I_F,
			BRANCH_I_F_C
		};

		// The the index is the entityID, the element is the tags,
		// 1 -> branch1, 2 -> branch2, 3 -> branch3.
		std::vector<int> branchTags(sizeSum + 1);	// Add aditional one for mapping id correctly.

		// Helper function to update the tags in the branchTags.
		auto UpdateBranchTags = [&branchTags, &branch1, &branch2, &branch3]()
		{
			for (auto id : branch1)
			{
				branchTags[id] = BRANCH_I;
			}
			for (auto id : branch2)
			{
				branchTags[id] = BRANCH_I_F;
			}
			for (auto id : branch3)
			{
				branchTags[id] = BRANCH_I_F_C;
			}
		};

		for (int i = 0; i < 20; ++i)
		{
			branch1.resize(sizeSum * (i + 1) / (20 * 2));
			branch2.resize(sizeSum * (i + 1) / (20 * 3));
			branch3.resize(sizeSum - branch1.size() - branch2.size());

			EntityManagerTool::NewEntities(eManager, idList, idList.size());

			RandomTool::Func::Shuffle(idList, i);

			RandomTool::Func::Dispatch(idList, branch1, branch2, branch3);

			EntityManagerTool::MaskComponent<IntComponent>									(eManager, branch1);
			EntityManagerTool::MaskComponent<IntComponent, FloatComponent>					(eManager, branch2);
			EntityManagerTool::MaskComponent<IntComponent, FloatComponent, CharComponent>	(eManager, branch3);

			UpdateBranchTags();

			// Next several for loop is to traverse the entityID by assigning the ComponentType,
			// then check whether the id have the ComponentType,
			// and whether the id is in the correct branch,
			// for example, if the entity have IntComponent, 
			// it must be in the BRANCH_I or BRANCH_I_F or BRANCH_I_F_C (the 'I' stantds for the IntComponent),

			for (auto id : eManager->RangeEntities<IntComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<IntComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I || branchTags[id] == BRANCH_I_F || branchTags[id] == BRANCH_I_F_C);
			}

			for (auto id : eManager->RangeEntities<FloatComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<FloatComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I_F || branchTags[id] == BRANCH_I_F_C);
			}

			for (auto id : eManager->RangeEntities<CharComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<CharComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I_F_C);
			}

			for (auto id : eManager->RangeEntities<IntComponent, FloatComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<IntComponent, FloatComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I_F || branchTags[id] == BRANCH_I_F_C);
			}

			for (auto id : eManager->RangeEntities<FloatComponent, CharComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<FloatComponent, CharComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I_F_C);
			}

			for (auto id : eManager->RangeEntities<IntComponent, CharComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<IntComponent, CharComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I_F_C);
			}

			for (auto id : eManager->RangeEntities<IntComponent, FloatComponent, CharComponent>())
			{
				errorLogger
					.LogIfFalse(eManager->haveComponent<IntComponent, FloatComponent, CharComponent>(id))
					.LogIfFalse(branchTags[id] == BRANCH_I_F_C);
			}

			// delete all ids.
			EntityManagerTool::DeleteEntities(eManager, idList);
		}
	TEST_UNIT_END;
#pragma endregion

}// function void AddTestUnit()


}// namespace TestUnit


int main()
{

	TestUnit::testMain();

    return 0;
}

