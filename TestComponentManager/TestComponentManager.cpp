// TestEntityManager.cpp : 定义控制台应用程序的入口点。
//

#include <ECS\EntityManager.h>
#include <ECS\ComponentManager.h>
#include <ECS\BaseComponent.h>
#include <MyTools\UnitTestModules.h>
#include <MyTools\RandomTool.h>
#include <vector>
#include <TestEntityManager\TestComponentTypes.h>
#include <TestEntityManager\TestEntityManagerTool.h>

// Necessary variable declarations for MyTools\UnitTestModules.h
DECLARE_TEST_UNITS;

using TestEntityManager = ECS::EntityManager<TestEntityTraits>;
using IDList = std::vector<ECS::EntityID>;

#define DECLARE_ENTITY_MANAGER(managerName)\
	auto * managerName = TestEntityManager::getInstance();\
	assert(managerName->getUsedIDCount() == 0 && "the test must ensure that the entityManager is empty.")

#define DECLARE_COMPONENT_MANAGER(managerName, ComponentType)\
	ECS::ComponentManager<ComponentType> managerName(TestEntityTraits::MaxEntityCount)

#pragma region test can we instantial the ComponentManager with correct or error comopnet type?
template class ECS::ComponentManager<IntComponent>;
// Next line of code should cause a compile error when enabled.
//template class ECS::ComponentManager<int>;
#pragma endregion

namespace ComponentManagerTool
{

template<typename COMPONENT_TYPE>
void DeleteComponents(ECS::ComponentManager<COMPONENT_TYPE>& cManager, std::vector<ECS::EntityID>& idList);


template<typename COMPONENT_TYPE>
void DeleteComponents(ECS::ComponentManager<COMPONENT_TYPE>& cManager, std::vector<ECS::EntityID>& idList)
{
	for (auto id : idList)
	{
		cManager.removeComponent(id);
	}
}

}

namespace TestUnit
{

void GetReady(){}

void AfterTest(){}

void AddTestUnit()
{

#pragma region all the function test for component manager
	TEST_UNIT_START(" all the function test for component manager")
		DECLARE_ENTITY_MANAGER(etManager);
		DECLARE_COMPONENT_MANAGER(intManager, IntComponent);

		assert(etManager->getSize() <= intManager.getMaxSize() && "component manager's size is to small");

		const size_t sumSize = etManager->getSize();
		IDList idList;
		IDList branch1;
		IDList branch2;

		std::vector<IntComponent*> cmpList(sumSize + 1, nullptr);


		for (int i = 0; i < 20; ++i)
		{
			branch1.resize(sumSize * (i + 1) / 21);
			branch2.resize(sumSize - branch1.size());

			// allocate the ids 
			EntityManagerTool::NewEntities(etManager, idList, sumSize);

			RandomTool::Func::Shuffle(idList, i);
			RandomTool::Func::Dispatch(idList, branch1, branch2);

			// for branch1, the component data is the id itself.
			for (auto id : branch1)
			{
				IntComponent* pNewCmp = intManager.newComponnet(id, id);

				errorLogger.LogIfEq(nullptr, pNewCmp);

				// store the component by its host id.
				cmpList[id] = pNewCmp;
			}

			// for branch2, the component data is the id * i.
			for (auto id : branch2)
			{
				IntComponent* pNewCmp = intManager.newComponnet(id, id * i);

				errorLogger.LogIfEq(nullptr, pNewCmp);

				cmpList[id] = pNewCmp;
			}

			// -------------------- READ COMPONENT BACK-------------------------------------------------------------------

			// now to get the component with the id in the branch1.
			for (auto id : branch1)
			{
				IntComponent* pNewCmp = intManager.getComponent(id);

				// first, the host id of the component must match.
				errorLogger.LogIfNotEq(id, pNewCmp->getHostID());

				// second, the data inside the component must be corrected.
				errorLogger.LogIfNotEq(id, pNewCmp->data);

				// finally, ensure that we are access to the same pointer as we allocate it.
				errorLogger.LogIfNotEq(cmpList[id], pNewCmp);
			}
			// this for loop is same functionaliy as the previous one, 
			// but use the operator [] to get component.
			// now to get the component with the id in the branch1.
			for (auto id : branch1)
			{
				IntComponent* pNewCmp = intManager[id];

				// first, the host id of the component must match.
				errorLogger.LogIfNotEq(id, pNewCmp->getHostID());

				// second, the data inside the component must be corrected.
				errorLogger.LogIfNotEq(id, pNewCmp->data);

				// finally, ensure that we are access to the same pointer as we allocate it.
				errorLogger.LogIfNotEq(cmpList[id], pNewCmp);
			}

			// Then, here is for the branch2, the same routine as branch1,
			// get component by function 'getComponent'

			// get compoent by operator '[]'
			for (auto id : branch2)
			{
				IntComponent* pNewCmp = intManager.getComponent(id);

				// first, the host id of the component must match.
				errorLogger.LogIfNotEq(id, pNewCmp->getHostID());

				// second, the data inside the component must be corrected.
				errorLogger.LogIfNotEq(id * i, pNewCmp->data);

				// finally, ensure that we are access to the same pointer as we allocate it.
				errorLogger.LogIfNotEq(cmpList[id], pNewCmp);
			}
			// this for loop is same functionaliy as the previous one, 
			// but use the operator [] to get component.
			// now to get the component with the id in the branch1.
			for (auto id : branch2)
			{
				IntComponent* pNewCmp = intManager[id];

				// first, the host id of the component must match.
				errorLogger.LogIfNotEq(id, pNewCmp->getHostID());

				// second, the data inside the component must be corrected.
				errorLogger.LogIfNotEq(id * i, pNewCmp->data);

				// finally, ensure that we are access to the same pointer as we allocate it.
				errorLogger.LogIfNotEq(cmpList[id], pNewCmp);
			}

			// delete components
			ComponentManagerTool::DeleteComponents(intManager, idList);

			// deallcote the ids.
			EntityManagerTool::DeleteEntities(etManager, idList);
		}// end for loop

	TEST_UNIT_END;
#pragma endregion 

}

}// namespace TestUnit


int main()
{

	TestUnit::testMain();

	return 0;
}

