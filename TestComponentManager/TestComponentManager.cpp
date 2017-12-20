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

DECLARE_TEST_UNITS;

const int gComponentMaxSize = 1024;
#define DeclareComponentManager(managerName, ComponentType, MaxSize)\
	ECS::ComponentManager<ComponentType> managerName(MaxSize)

#define DeclareEntityManager(managerName)\
	ECS::EntityManager* managerName = ECS::EntityManager::getInstance()

struct TestStructA
	:public ECS::BaseComponent
{

	static int deconstructCount;
	static const int initDataA;
	static const float initDataB;
public:
	int dataA;
	float dataB;
	TestStructA() :dataA(initDataA), dataB(initDataB) {}
	TestStructA(int initDataA, float initDataB)
		:dataA(initDataA), dataB(initDataB){}
	~TestStructA()
	{
		// add one to the deconstructCount
		++deconstructCount;
	}
};

int TestStructA::deconstructCount = 0;
const int TestStructA::initDataA = 2;
const float TestStructA::initDataB = 2.222f;

template<typename COMPONENT_TYPE>
int newComponents(
	ECS::ComponentManager<COMPONENT_TYPE>& cManager,
	std::vector<ECS::EntityID>& idList,
	std::vector<COMPONENT_TYPE*> * compList)
{
	int error = 0;
	compList->clear();
	for (auto id : idList)
	{
		auto * newComp = cManager.newComponnet(id);
		if (nullptr == newComp)
		{
			error += 1;
			break;
		}
		compList->push_back(newComp);
	}
	return error;
}

// the main process is to allocate entityID
// and get a new component with the id, 
// then random delete components
// finally delete th entityIDs
// it will allocate all the entityID 
template<typename COMPONENT_TYPE>
int randomTestCompnets(
	ECS::EntityManager* eManager,
	ECS::ComponentManager<COMPONENT_TYPE>& cManager,
	size_t entityCount,
	size_t loopTime = 20, 
	bool randomPerBatch = false, 
	int randSeed = 1)
{
	TestUnit::ErrorLogger			errorLogger;
	std::vector<ECS::EntityID>		idList;			// store the entityIDs
	std::vector<COMPONENT_TYPE*>		cmpList;		// store the components pointer
	const size_t maxCmpSize = cManager.getMaxSize();

	// get the entityIDs
	newEntitis(eManager, &idList, maxCmpSize);
	RandomTool::RandomSet<size_t> randomIndices;
	RandomTool::RandomSet<size_t> deleteIndices;

	// set seed
	randomIndices.setSeed(randSeed);
	// make the delete indices different
	deleteIndices.setSeed(randSeed + 1);

	// init random indices
	randomIndices.randomSequence(maxCmpSize);
	deleteIndices.randomSequence(maxCmpSize);

	// loop allocat components, ensure ensure
	// to use the components out.
	for (size_t loopCount = 0; loopCount < loopTime; ++loopCount)
	{
		// each start, ensure the mananger don't allocate any component
		errorLogger += NOT_EQ(0, cManager.getUsedCount());
		// the random entities to get the components.
		if (randomPerBatch)
		{
			randomIndices.tickSeed();
			randomIndices.randomSequence(maxCmpSize);
		}
		
		// new components
		for (auto idIndex : randomIndices)
		{
			ECS::EntityID randID = idList[idIndex];
			auto * newCmp = cManager.newComponnet(randID);
			if (EQ(newCmp, nullptr))
			{
				break;
			}
			cmpList.push_back(newCmp);
		}

		// check each component is correct
		for (auto cmp : cmpList)
		{
			errorLogger += NOT_EQ(TestStructA::initDataA, cmp->dataA);
			errorLogger += NOT_EQ(TestStructA::initDataB, cmp->dataB);
		}

		// all the components should be used out
		errorLogger += NOT_EQ(cManager.getUsedCount(), cManager.getMaxSize());

		if (randomPerBatch)
		{
			deleteIndices.tickSeed();
			deleteIndices.randomSequence(maxCmpSize);
		}
		// delete all the components
		for (auto idIndex : deleteIndices)
		{
			ECS::EntityID randID = idList[idIndex];
			errorLogger += NOT_EQ(true, cManager.removeComponent(randID));
		}

		cmpList.clear();
	}

	destoryEntities(eManager, &idList);

	return errorLogger.getErrorCount();
	
}

// add one component to the entity,
// and mask it,
// return how many errors happened.
template<typename COMPONENT_TYPE, typename...CONSTRUCT_ARGS>
int addComponentAndMask(
	ECS::EntityManager* eManager,
	ECS::ComponentManager<COMPONENT_TYPE>& cManager,
	ECS::EntityID id,
	CONSTRUCT_ARGS&& ...args
)
{
	if (!eManager->isValid(id))
	{
		// one error, the id is not valid,
		// quit the function immediately.
		return 1;
	}
	int error = 0;
	error += NOT_EQ(
		ECS::MaskResultFlag::Success, 
		eManager->maskComponentType<COMPONENT_TYPE>(id));

	error += EQ(
		nullptr, 
		cManager.newComponnet(id, std::forward<CONSTRUCT_ARGS>(args)...));

	return error;
}

template<typename COMPONENT_TYPE, typename...CONSTRUCT_ARGS>
int addComponentAndMask(
	ECS::EntityManager* eManager,
	ECS::ComponentManager<COMPONENT_TYPE>& cManager,
	std::vector<ECS::EntityID>& idList,
	CONSTRUCT_ARGS&& ...args)
{
	int error = 0;
	for (auto & id : idList)
	{
		error += addComponentAndMask(eManager, cManager, id, std::forward<CONSTRUCT_ARGS>(args)...);
	}
	return error;
}

// checking one entity id that must have the component type,
// and the component data must also be the same.
template<typename COMPONENT_TYPE>
int CheckComponentDataEqual(
	ECS::ComponentManager<COMPONENT_TYPE>& pManager, 
	ECS::EntityID id, 
	const COMPONENT_TYPE& compareComponent)
{
	COMPONENT_TYPE* pComponent;
	pComponent = pManager.getComponent(id);
	if (EQ(nullptr, pComponent))
	{
		// one error
		return 1;
	}
	else if (EQ(*pComponent, compareComponent))
	{
		// no error.
		return 0;
	}

	// component data not equal, 
	// one error.
	return 1;
}

// check all the component with a container,
// which implements the 'for range'.
template<typename COMPONENT_TYPE, typename ID_CONTAINER>
CheckComponentsResult CheckComponentDataEqual(
	ECS::ComponentManager<COMPONENT_TYPE>& pManager,
	ID_CONTAINER& idContainer,
	const COMPONENT_TYPE& compareComponent)
{
	CheckComponentsResult result;
	for (ECS::EntityID checkId : idContainer)
	{
		result.count++;
		result.error += 
			CheckComponentDataEqual<COMPONENT_TYPE>(pManager, checkId, compareComponent);
	}
	return result;
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
		// test errorLogger, this is not the test about ComponentManager.
		{
		TEST_UNIT_START("test errorLogger, this is not the test about ComponentManager.")
			int error = 0;

			error += NOT_EQ(0, errorLogger.getErrorCount());

			errorLogger++;
			error += NOT_EQ(1, errorLogger.getErrorCount());

			++errorLogger;
			error += NOT_EQ(2, errorLogger.getErrorCount());

			errorLogger.tick();
			error += NOT_EQ(3, errorLogger.getErrorCount());

			errorLogger += 1;
			error += NOT_EQ(4, errorLogger.getErrorCount());

			errorLogger += 2;
			error += NOT_EQ(6, errorLogger.getErrorCount());

			errorLogger.addErrorCount(1);
			error += NOT_EQ(7, errorLogger.getErrorCount());

			errorLogger.addErrorCount(2);
			error += NOT_EQ(9, errorLogger.getErrorCount());
			
			return EQ(0, error);
		TEST_UNIT_END;
		}

		// create a ComponentManager with TestStructA
		{
		TEST_UNIT_START("create a ComponentManager with TestStructA")
			int error = 0;
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			const size_t entityCount = 6;
			// allocate the ids 
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> componentList;

			error += newComponents(tacManager, idList, &componentList);

			error += NOT_EQ(idList.size(), componentList.size());

			// deallcote the ids, because the etManager is global。
			errorLogger += destoryEntities(etManager, &idList);

			return error == 0;
		TEST_UNIT_END;
		}

		// get component from ComponentManager 
		{
		TEST_UNIT_START("get component from ComponentManager")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			const size_t entityCount = 6;
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> componentList;


			errorLogger += newComponents(tacManager, idList, &componentList);

			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				auto id = idList[idIndex];
				errorLogger += NOT_EQ(tacManager.getComponent(id), componentList[idIndex]);
			}

			errorLogger += destoryEntities(etManager, &idList);

			return errorLogger.conclusion();
		TEST_UNIT_END;
		}
		
		// get component from ComponentManager using the operator []
		{
		TEST_UNIT_START("get component from ComponentManager using the operator []")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			const size_t entityCount = 6;
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> componentList;


			errorLogger += newComponents(tacManager, idList, &componentList);

			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				auto id = idList[idIndex];
				errorLogger += NOT_EQ(tacManager[id], componentList[idIndex]);
			}

			errorLogger += destoryEntities(etManager, &idList);

			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// can pass the argument to the component constructor?
		{
		TEST_UNIT_START("can pass the argument to the component constructor?")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			std::vector<ECS::EntityID> idList;
			size_t idIndex = 0;
			const size_t entityCount				= 5;				// avaliable entities
			const size_t defaultConstructorCount	= 3;				// the component use the default construct
			const size_t nonDefaultConstructorCount	= entityCount - 3;	// the component use the none default constructor
			errorLogger += newEntitis(etManager, &idList, entityCount);
			std::vector<TestStructA*> cmpList;
			int nonDefaultDataA = 3;
			float nonDefaultDataB = 4.2f;
			
			// generate the components
			for (size_t defaultIndex = 0; defaultIndex < defaultConstructorCount; ++defaultIndex)
			{
				cmpList.push_back(tacManager.newComponnet(idList[idIndex++]));
			}
			for (size_t nonDefaultIndex = 0; nonDefaultIndex < nonDefaultConstructorCount; ++nonDefaultIndex)
			{
				cmpList.push_back(tacManager.newComponnet(idList[idIndex++], nonDefaultDataA, nonDefaultDataB));
			}

			// check components
			size_t cmpIndex = 0;
			// default constructor
			for (size_t defaultIndex = 0; defaultIndex < defaultConstructorCount; ++defaultIndex)
			{
				errorLogger += NOT_EQ(TestStructA::initDataA, cmpList[cmpIndex]->dataA);
				errorLogger += NOT_EQ(TestStructA::initDataB, cmpList[cmpIndex]->dataB);
				++cmpIndex;
			}
			// none default constructor
			for (size_t nonDefaultIndex = 0; nonDefaultIndex < nonDefaultConstructorCount; ++nonDefaultIndex)
			{
				errorLogger += NOT_EQ(nonDefaultDataA, cmpList[cmpIndex]->dataA);
				errorLogger += NOT_EQ(nonDefaultDataB, cmpList[cmpIndex]->dataB);
				++cmpIndex;
			}
			
			errorLogger += destoryEntities(etManager, &idList);

			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// can the entityID be set into the component?
		{
		TEST_UNIT_START("can the entityID be set into the componet?")

			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);
			RandomTool::RandomSet<size_t> randomIndices;
			std::vector<TestStructA*> cmpList;
			std::vector<ECS::EntityID> idList;

			const size_t entityCount = 100;				// avaliable entities
			// allocate the ids
			errorLogger += newEntitis(etManager, &idList, entityCount);
			// allocate the components
			errorLogger += newComponents(tacManager, idList, &cmpList);

			// for each id check that the id is same as the component
			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				errorLogger +=
					NOT_EQ(idList[idIndex], cmpList[idIndex]->getHostID());
			}
			// free all the id.
			errorLogger += destoryEntities(etManager, &idList);
			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// ensure that when we remove the entityID from the componentManager, the deconstructor is called
		{
		TEST_UNIT_START("ensure that when we remove the entityID from the componentManager, the deconstructor is called")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);
			int beforeDeconstructCount = TestStructA::deconstructCount;
			RandomTool::RandomSet<size_t> randomIndices;
			std::vector<TestStructA*> cmpList;
			std::vector<ECS::EntityID> idList;

			const size_t entityCount = 100;				// available entities
			// allocate the ids
			errorLogger += newEntitis(etManager, &idList, entityCount);
			// allocate the components
			errorLogger += newComponents(tacManager, idList, &cmpList);

			// for each id check that the id is same as the component
			for (size_t idIndex = 0; idIndex < idList.size(); ++idIndex)
			{
				errorLogger += 
					NOT_EQ(idList[idIndex], cmpList[idIndex]->getHostID());
			}

			// free all the components
			for (auto id : idList)
			{
				errorLogger += NOT_EQ(true, tacManager.removeComponent(id));
			}

			// check the deallocate count
			int aftherDeconstructCount = TestStructA::deconstructCount;
			errorLogger += 
				NOT_EQ(entityCount, aftherDeconstructCount - beforeDeconstructCount);

			// free all the id.
			errorLogger += destoryEntities(etManager, &idList);
			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// random test on componentManager
		{
		TEST_UNIT_START("random test on componentManager")
			DeclareEntityManager(etManager);
			DeclareComponentManager(tacManager, TestStructA, gComponentMaxSize);

			errorLogger += randomTestCompnets(etManager, tacManager, gComponentMaxSize, 20, true);
			return errorLogger.conclusion();
		TEST_UNIT_END;
		}

		// traverser components with Entity Manager
		{
			TEST_UNIT_START("traverser components with Entity Manager")
				DeclareComponentManager(intCManager,	IntComponent,	1024);
				DeclareComponentManager(floatCManager,	FloatComponent, 1024);
				DeclareComponentManager(charCManager,	CharComponent,	1024);
				DeclareEntityManager(eManager);
				
				std::vector<ECS::EntityID> totalIDList;
				std::vector < std::vector<ECS::EntityID>> idListArray;
				using CN = ComponentNumber;
				// help function to get the entity count who have the components.
				// for instance, pass in CN::IntC, it will return the count that entity coutn
				// that have the IntComponent.
				auto getEntityNumberOfComponents = [&](CN type) ->size_t
				{
					static size_t singleInt				= idListArray[CN::IntC].size();
					static size_t singleFloat			= idListArray[CN::FloatC].size();
					static size_t singleChar			= idListArray[CN::CharC].size();
					static size_t combInt_Float			= idListArray[CN::Int_FloatC].size();
					static size_t combInt_Char			= idListArray[CN::Int_CharC].size();
					static size_t combFloat_Char		= idListArray[CN::Float_CharC].size();
					static size_t combInt_Float_Char	= idListArray[CN::Int_Float_CharC].size();

					switch (type)
					{
					case CN::IntC:
						return singleInt + combInt_Char + combInt_Float + combInt_Float_Char;
						break;
						
					case CN::FloatC:
						return singleFloat + combInt_Float + combFloat_Char + combInt_Float_Char;
						break;

					case CN::CharC:
						return singleChar + combInt_Char + combFloat_Char + combInt_Float_Char;
						break;

					case CN::Int_FloatC:
						return combInt_Float + combInt_Float_Char;
						break;

					case CN::Int_CharC:
						return combInt_Char + combInt_Float_Char;
						break;

					case CN::Float_CharC:
						return combFloat_Char + combInt_Float_Char;
						break;

					case CN::Int_Float_CharC:
						return combInt_Float_Char;
						break;

					default:
						ASSERT(false && "error componet type");
						return 0;
						break;
					}
				};
				errorLogger += randomIdListArray(eManager, &totalIDList, &idListArray, 8, 64, 128, 1, 2);
				
				IntComponent sourceIntComponet(1);
				FloatComponent sourceFloatComponet(2.2f);
				CharComponent sourceCharComponet('a');
				// add components
				{
					// mask with IntComponet
					errorLogger += addComponentAndMask<IntComponent>(eManager, intCManager,
						idListArray[ComponentNumber::IntC], sourceIntComponet);
					errorLogger += addComponentAndMask<IntComponent>(eManager, intCManager,
						idListArray[ComponentNumber::Int_FloatC], sourceIntComponet);
					errorLogger += addComponentAndMask<IntComponent>(eManager, intCManager,
						idListArray[ComponentNumber::Int_CharC], sourceIntComponet);
					errorLogger += addComponentAndMask<IntComponent>(eManager, intCManager,
						idListArray[ComponentNumber::Int_Float_CharC], sourceIntComponet);

					// mask with FloatComponent
					errorLogger += addComponentAndMask<FloatComponent>(eManager, floatCManager,
						idListArray[ComponentNumber::FloatC], sourceFloatComponet);
					errorLogger += addComponentAndMask<FloatComponent>(eManager, floatCManager,
						idListArray[ComponentNumber::Int_FloatC], sourceFloatComponet);
					errorLogger += addComponentAndMask<FloatComponent>(eManager, floatCManager,
						idListArray[ComponentNumber::Float_CharC], sourceFloatComponet);
					errorLogger += addComponentAndMask<FloatComponent>(eManager, floatCManager,
						idListArray[ComponentNumber::Int_Float_CharC], sourceFloatComponet);

					// mask with CharComponent
					errorLogger += addComponentAndMask<CharComponent>(eManager, charCManager,
						idListArray[ComponentNumber::CharC], sourceCharComponet);
					errorLogger += addComponentAndMask<CharComponent>(eManager, charCManager,
						idListArray[ComponentNumber::Int_CharC], sourceCharComponet);
					errorLogger += addComponentAndMask<CharComponent>(eManager, charCManager,
						idListArray[ComponentNumber::Float_CharC], sourceCharComponet);
					errorLogger += addComponentAndMask<CharComponent>(eManager, charCManager,
						idListArray[ComponentNumber::Int_Float_CharC], sourceCharComponet);
				}

				// use EntityManager.RangeEntities() method to iterate all the entities
				// that have the component .
				{
					IntComponent invalidIntComponent(2);
					FloatComponent invalidFloatComponent(3.3f);
					CharComponent invalidCharComponent('b');

					CheckComponentsResult result;

					// single component traverser. 
					result = CheckComponentDataEqual(
						intCManager, eManager->RangeEntities<IntComponent>(), sourceIntComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::IntC));
					result = CheckComponentDataEqual(
						floatCManager, eManager->RangeEntities<FloatComponent>(), sourceFloatComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::FloatC));
					result = CheckComponentDataEqual(
						charCManager, eManager->RangeEntities<CharComponent>(), sourceCharComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::CharC));
					// multiple component traverser.
					// int - float
					result = CheckComponentDataEqual(
						intCManager, eManager->RangeEntities<IntComponent, FloatComponent>(), sourceIntComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_FloatC));
					result = CheckComponentDataEqual(
						floatCManager, eManager->RangeEntities<IntComponent, FloatComponent>(), sourceFloatComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_FloatC));
					// int - char
					result = CheckComponentDataEqual(
						intCManager, eManager->RangeEntities<IntComponent, CharComponent>(), sourceIntComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_CharC));
					result = CheckComponentDataEqual(
						charCManager, eManager->RangeEntities<IntComponent, CharComponent>(), sourceCharComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_CharC));
					// float - char
					result = CheckComponentDataEqual(
						floatCManager, eManager->RangeEntities<FloatComponent, CharComponent>(), sourceFloatComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Float_CharC));
					result = CheckComponentDataEqual(
						charCManager, eManager->RangeEntities<FloatComponent, CharComponent>(), sourceCharComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Float_CharC));
					// int - float - char
					result = CheckComponentDataEqual(
						intCManager, eManager->RangeEntities<IntComponent, FloatComponent, CharComponent>(), sourceIntComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_Float_CharC));
					result = CheckComponentDataEqual(
						floatCManager, eManager->RangeEntities<IntComponent, FloatComponent, CharComponent>(), sourceFloatComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_Float_CharC));
					result = CheckComponentDataEqual(
						charCManager, eManager->RangeEntities<IntComponent, FloatComponent, CharComponent>(), sourceCharComponet);
					errorLogger += result.error;
					errorLogger.LogIfNotEq(result.count, getEntityNumberOfComponents(CN::Int_Float_CharC));
					



					result = CheckComponentDataEqual<IntComponent, ECS::EntityRange<IntComponent>>(
						intCManager, eManager->RangeEntities<IntComponent>(), invalidIntComponent);
					errorLogger.LogIfNotEq(result.count, result.error);
					result = CheckComponentDataEqual<FloatComponent, ECS::EntityRange<FloatComponent>>(
						floatCManager, eManager->RangeEntities<FloatComponent>(), invalidFloatComponent);
					errorLogger.LogIfNotEq(result.count, result.error);
					result = CheckComponentDataEqual<CharComponent, ECS::EntityRange<CharComponent>>(
						charCManager, eManager->RangeEntities<CharComponent>(), invalidCharComponent);
					errorLogger.LogIfNotEq(result.count, result.error);
				}

				errorLogger += destoryEntities(eManager, &totalIDList);
				return errorLogger.conclusion();
		TEST_UNIT_END;

		}
	}
}


int main()
{

	TestUnit::testMain();

	return 0;
}

