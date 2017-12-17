这是一个实现EntityComponentSystem的项目

# Entity
实体的主要作用用来标识一个实体拥有哪种类型的Component，所以其内部不会存储Component，而是使用二进制的01来表明Component的类型

# Component
Component拥有具体的数据，每一种Component分别存储在各自的ComponentManager中
* Component内部包含一个EntityI
对应的ComponentManager提供通过EntityID的方法来获取Component指针

# EntityManager
单例模式
EntityManager主要功能：
* 分配EntityID
* 回收EntityID
* 对每一个EntityID保存一串二进制码，每一位标志一种类型的Component 
内部使用类似池分配器的形式存储所有未分配的EntityID。

主要函数：

函数名 | 功能
-------|-----
EntityManager* getInstance()  | 返回EntityManager实例 
EntityID newEntity()  |  新分配一个EntityID，返回这个ID值
bool destoryEntity(EntityID destoriedID)  |删除一个Entity，返回是否删除成功  
size_t getSize()  |  返回能够分配的总的Entity的数量
size_t getUsedIDCount()  |  返回已经分配的Entity数量
bool isValid(EntityID checkID)  |  判断指定的entityID是否是已被分配的ID，或者说处于激活状态。
MaskResult maskSingleComponentType<CMP_TYPE>(EntityID)  |  为一个entity标志一种组件类型，返回的结果只可能是MaskResultFlag中的一种
MaskResult maskComponentType<...CMP_TYPES>(EntityID entityID);  |  接受变长模板参数，为一个entity标志多个组件类型。返回的值是每一种组件类型调用maskSingleComponentType的按位与的结果，只有最终结果 == MaskResultFlag::Success的时候才表明没有错误发生，否则需要程序员自行判断发生了哪一种错误。

#### MaskResult
在为entity标志组件类型的时候，可以同时标志多种组件，为了让所有标志过程的结果显示在一个变量中，使用bit组合的方式来表示  
MaskResult就是一个8个bit的unsigned char，每一位的含义在MaskResultFlag（枚举类型）中有定义：
#### MaskResultFlag
枚举可能的标志组件的结果：
```c++
enum MaskResultFlag
		: unsigned char
	{ 
		InvalidEntityID			= 1,	// 0001 entityID非法，这一项基本没用，因平常直接坐在这里断言
		Success					= 2,	// 0010 成功
		RedundancyComponent		= 4,	// 0100 重复标志组件类型
		InvalidComponentType	= 8		// 1000 组件类型不可用
	};
```

# ComponentManager<CMP_TYPE>
一个模板类，模板参数为Component的具体类型
模板参数必须继承**ECS::BaseComponent**  
## 功能：
* 根据EntityID来创建、保存、删除Component指针
* Component的内存由manager进行维护。
## 主要函数：

函数名     |       功能
----------|--------------
CMP_TYPE *	newComponnet(EntityID entityID, CONSTRUCT_ARGS&&...args)  |  使用对应的entityID来创建Component，如果数量超出上限，返回nullptr，如果在manager中已经有component使用了同样的entityID，将会触发断言。
getComponent(EntityID entityID)  |  返回指定entityID的component指针，如果不存在，返回nullptr
COMPONENT_TYPE *	operator \[\](EntityID entityID)  |  数组下标操作符重载，返回指定entityID的component指针，如果不存在，返回nullptr
bool removeComponent(EntityID removedID)  |  删除对应EntityID的Component指针，并且负责释放其内存（调用delete函数），如果删除失败（不存在对应组件），返回false，否则返回true。
size_t getMaxSize()  |  获取能够分配的component的最大数量
size_t getUsedCount()  |  获取已分配的component数量
