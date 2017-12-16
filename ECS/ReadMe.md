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
