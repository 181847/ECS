#pragma once
#include <ECS/ECS.h>
#include <ECS/EntityManager.h>
#include <ECS/ComponentManager.h>
#include <MyTools/RandomTool.h>
#include <MyTools/UnitTestModules.h>
#include <algorithm>

// this struct limit an entity manager's max entity count.
struct TestEntityTraits {
	static const size_t MaxEntityCount = 2048;
	static const size_t MaxComponentTypes = 32;
	typedef std::bitset<MaxComponentTypes> ComponentMask;
	typedef ECS::ComponentIDGenerator ComponentIDGenerator;
};

namespace EntityManagerTool
{
template<typename EMTraits>
void NewEntities(ECS::EntityManager<EMTraits>* pEntityManager, std::vector<ECS::EntityID>& idList, const int count);

template<typename EMTraits>
void DeleteEntities(ECS::EntityManager<EMTraits>* pEntityManager, std::vector<ECS::EntityID>& idList);

template<typename ...COMPONENT_TYPE_LIST, typename EMTraits>
void MaskComponent(ECS::EntityManager<EMTraits>* pEntityManager, std::vector<ECS::EntityID>& idList);



template<typename EMTraits>
void NewEntities(ECS::EntityManager<EMTraits>* pEntityManager, std::vector<ECS::EntityID>& idList, const int count)
{
	idList.resize(count);

	for (int i = 0; i < count; ++i)
	{
		idList[i] = pEntityManager->newEntity();
	}
}

template<typename EMTraits>
void DeleteEntities(ECS::EntityManager<EMTraits>* pEntityManager, std::vector<ECS::EntityID>& idList)
{
	for (auto id : idList)
	{
		pEntityManager->destoryEntity(id);
	}
}

template<typename ...COMPONENT_TYPE_LIST, typename EMTraits>
void MaskComponent(ECS::EntityManager<EMTraits>* pEntityManager, std::vector<ECS::EntityID>& idList)
{
	for (auto id : idList)
	{
		pEntityManager->maskComponentType<COMPONENT_TYPE_LIST...>(id);
	}
}



} // namespace EntityManagerTool

/*
template<typename ...COMPONENT_TYPES>
inline int maskIdListWithComponentTypes(
	ECS::EntityManager<TestEnityTrait>* pManager,
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
inline CheckComponentsResult HelpTraverseEntities(ECS::EntityManager<TestEnityTrait>* pManager)
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
		ECS::EntityManager<TestEnityTrait>* pManager,
		const std::vector<ECS::EntityID>& idList)
	{
		return maskIdListWithComponentTypes<COMPONENT_TYPES...>(pManager, idList);
	}

	static CheckComponentsResult HelpCheck(
		ECS::EntityManager<TestEnityTrait>* pManager)
	{
		return HelpTraverseEntities<COMPONENT_TYPES...>(pManager);
	}
};
// make the name of ComponentTypeContainer shorter.
template<typename...CMP_TYPES> using CMPS =
ComponentTypeContainer<CMP_TYPES...>;

template<typename ...CMP_CONTAINER>
inline int maskIdListWithComponentContainer(
	ECS::EntityManager<TestEnityTrait>* pManager,
	const std::vector<std::vector<ECS::EntityID>>& idListArray)
{
	int error = 0;

	if (sizeof...(CMP_CONTAINER) != idListArray.size())
		return 1;

	size_t idListNumber = 0;

	bool arg[] = { (error += CMP_CONTAINER::maskIdList(pManager, idListArray[idListNumber++]), false)... };
	return error;
}

template<typename...CMP_CONTAINER>
inline CheckComponentsResult checkMaskResultWithComponentContainer(
	ECS::EntityManager<TestEnityTrait>* pManager)
{
	CheckComponentsResult retResult;
	bool arg[] = { (retResult += CMP_CONTAINER::HelpCheck(pManager), false)... };
	return retResult;
}

inline int randomIdListArray(
	ECS::EntityManager<TestEntityTraits>* pManager,
	std::vector<ECS::EntityID>* pTotalIDList,				// all the id are in here.
	std::vector<std::vector<ECS::EntityID>>* pIdListArray,	// get the dispatched ids
	const size_t idListArrayCount = 8,
	const size_t minListSize = 64,
	const size_t maxListSize = 128,
	const size_t idListSizeSeed = 1, 
	const size_t dispatchIDSeed = 2)
{
	int error = 0;
	pTotalIDList->clear();
	pIdListArray->resize(idListArrayCount);
	// for each idList set a random size for it.
	RandomTool::RandomSet<size_t> randomIdListSize;
	randomIdListSize.setSeed(idListSizeSeed);
	randomIdListSize.RandomNumbers(idListArrayCount, minListSize, maxListSize);

	// allocate all the IDs for later use.
	size_t totalIDCount = 0;
	for (auto size : randomIdListSize)
	{
		totalIDCount += size;
	}
	EntityManagerTool::NewEntities(pManager, *pTotalIDList, totalIDCount);

	// ready to randomly dispatch all the id to different idList
	RandomTool::RandomSet<size_t> randomIDDisaptchIndices;
	randomIDDisaptchIndices.setSeed(dispatchIDSeed);
	randomIDDisaptchIndices.RandomSequence(totalIDCount);

	// dispatch the ids
	size_t randomDispatchIndexLocation = 0;
	for (size_t idListNumber = 0; idListNumber < idListArrayCount; ++idListNumber)
	{
		for (size_t dispatchCount = 0; dispatchCount < randomIdListSize[idListNumber]; ++dispatchCount)
		{
			// get the id.
			ECS::EntityID id = (*pTotalIDList)[randomIDDisaptchIndices
				[randomDispatchIndexLocation++]];
			// dispatch it.
			(*pIdListArray)[idListNumber].push_back(id);
		}
	}
	return error;
}

// This enum is used in some component test,
// in this test we can get multiple idList which is in a idListArray,
// and mask them with different component type,
// use this enum to access the idListArray
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

*/