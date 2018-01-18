#pragma once
#include <ECS/ECS.h>
#include <ECS/EntityManager.h>
#include <ECS/ComponentManager.h>
#include <MyTools/RandomTool.h>
#include <MyTools/UnitTestModules.h>
#include <algorithm>

// this struct limit an entity manager's max entity count.
struct TestEnityTrait {
	static const size_t MaxEntityCount = 2048;
	static const size_t MaxComponentTypes = 32;
	typedef std::bitset<MaxComponentTypes> ComponentMask;
	typedef ECS::ComponentIDGenerator ComponentIDGenerator;
};


// get batch Entities,
// return errors count, (if no error, return 0)
int newEntitis(
	ECS::EntityManager<TestEnityTrait>* eManager,
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

// destory the batch Entitys£¬ and clear the idList.
// return errors count, (if no error, return 0)
int destoryEntities(
	ECS::EntityManager<TestEnityTrait>* eManager,
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
	ECS::EntityManager<TestEnityTrait>* pManager,
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
	ECS::EntityManager<TestEnityTrait>* pManager,
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
	randomIdListSize.randomNumbers(idListArrayCount, minListSize, maxListSize);

	// allocate all the IDs for later use.
	size_t totalIDCount = 0;
	for (auto size : randomIdListSize)
	{
		totalIDCount += size;
	}
	error += newEntitis(pManager, pTotalIDList, totalIDCount);

	// ready to randomly dispatch all the id to different idList
	RandomTool::RandomSet<size_t> randomIDDisaptchIndices;
	randomIDDisaptchIndices.setSeed(dispatchIDSeed);
	randomIDDisaptchIndices.randomSequence(totalIDCount);

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
