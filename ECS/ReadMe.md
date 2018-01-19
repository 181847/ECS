# 功能
* EntityManager：分配EntityID，每个EntityID包括一个类型蒙版表明这个Entity包含哪些类型的组件
* ComponentManager：通过EntityID存取组件对象
* ECSWorld：管理以上两种类型的Manager，统一规范（**尚未完全实现**）

## EntityManager\<Traits\>
<div style="background: #999999">

### 模板Traits

此模板应该定义以下常量或类型：

常量或类型 | 常量名 | 作用
---------|--------|-------
const size_t | MaxEntityCount | EntityManager可以分配的最大Entity数量
const size_t | MaxComponentTypes | EntityManager可以区分的最大数量的组件类型
类型声明    | **ComponentMask** | 定义蒙版类型，用来标记Entity所拥有的蒙版，就目前而言，必须使用std::bitset
类型声明    | **ComponentIDGenerator** | 组件类型ID生成器，用于生成组件**类型**的ID值，必须实现**size_t IDOf\<T\>()**，考虑使用TypeTool::IDGenerator

<div style="background: #aaaaaa">

### 函数：
* EntityID	newEntity()  
  * 申请一个新的Entity，返回EntityID，当返回0的时候代表分配失败。
* bool destoryEntity(EntityID destoriedID)
  * 回收一个EntityID，返回是否成功回收，不能把尚未分配的EntityID释放掉。
* bool		isValid(EntityID checkID)
  * 检查一个EntityID是否是可用的，或者说是被分配的，但是还没有被回收的。
* template\<T\> MaskResult maskSingleComponentType(EntityID entityID)
  * 为某个EntityID添加一个组件类型的蒙版，返回蒙版的结果。
* template\<...T\> MaskResult	maskComponentType(EntityID entityID)
  * 为某个EntityID同时蒙版多个组件类型，返回蒙版的结果。
* template\<...T\> bool haveComponent(EntityID entityID)
  * 检查某个EntityID是否包含指定的多个（或者单个）组件的蒙版。
  * 如果这个ID尚未被分配，则将触发断言。
* template\<...T\> auto RangeEntities();
  * 返回一个用于遍历EntityID的迭代器
  * 例如
```c++
for (EntityID & id : entityManager->RangeEntities<IntComponent>())
{
    // Do something with the id.
    // All the entities have the IntComponent.
}
```
* size_t getSize() const
  * 返回可使用的总的EntityID的数量
* size_t getUsedIDCount() const
  * 返回已经使用的EntityID的数量

</div>
</div>

<!--EntityManager-->

----

## CompoentManager\<COMPONENT_TYPE\>
<div style="background: #999999">

### 模板

<div style="background: #aaaaaa">

### 函数：
* 构造函数
  * 构造函数接受一个size_t类型的值，表示最大能够容纳的Component的数量_
* template<...CONSTRUCT_ARGS> COMPONENT_TYPE *	newComponnet(EntityID entityID, CONSTRUCT_ARGS&&...args)
  * 为一个EntityID创建一个对应的组件，返回这个组件的指针，一个EntityID只能创建对应的一个组件，当id重复或者已分配组件数量超出Traits::MaxSize，触发断言。
  * 此方法**不检查EntityID能否可用**
* COMPONENT_TYPE *	getComponent(EntityID entityID);
  * 通过EntityID来获取为其分配的组件对象
  * 当尚**未**分配的时候，返回nullptr
  * 此方法**不检查EntityID能否可用**
* COMPONENT_TYPE *	operator [](EntityID entityID)
  * 重载数组下标运算符，功能与*getComponent*一样
  * 通过EntityID来获取为其分配的组件对象
  * 当尚**未**分配的时候，返回nullptr
  * 此方法**不检查EntityID能否可用**
* bool removeComponent(EntityID removedID)
  * 将指定ID的Component对象删除，返回删除是否成功
  * 此方法**不检查EntityID能否可用**
* std::size_t getMaxSize()
  * 返回Traits::MaxSize，ComponentManager能够容纳的最大的组件数量
* std::size_t getUsedCount()
  * 已分配的组件数量

</div>
</div>


## ECSWorld\<EntityManagerTraits, ...ComponentType\>
<div style="background: #999999">

一个ECSWorld包含一个EntityManager，多个ComponentManager。

模板参数定义：

模板参数  | 说明
--|--
EntityManagerTraits | 唯一的一个EntityManager的属性模板，详情参考EntityManager的定义
CompopnentType  |   所有要用到的Component类型。

<div style="background: #aaaaaa">

### 函数：
* EntityID NewEntity()
  * 申请一个新的Entity，并返回它的ID，创建失败时触发断言
* template<...> void Foreach(std::function<...> theJob)
  * 遍历包含某个类型的EntityID，使用theJob来针对这个Entity执行操作
  * **theJob**的函数签名为void theJob(EntityID, ComponentTypeA, ComponentTypeB, ...)
  * Foreach是一个模板函数，参数中指定的需要遍历的组件类型，例如<ComponentTypeA, ComponentTypeB, ...>
  * 为了方便代码的定义，**theJob**中的参数类型定义可以使用auto进行自动推导。
    * ecsWorld->Foreach<IntComponent, FloatComponent>(\[\](EntityID id, auto * pInt, auto * pFlt){  ... THE CODE ...   });
* template<...> bool ForOne(EntityID id, std::function<...> theJob);
  * 针对某一个Entity执行theJob中的代码
  * 如果这个ID**无效**，返回**false**，否则返回**true**
  * **theJob** 的函数签名为 void theJob(ComponentTypeA, ComponentTypeB, ...)
  * ForOne的模板参数需要指定对应的组件类型
  * 此函数不确保某个**实体**必定包含某个类型的**组件**，但是保证这个**组件**一定被**ECSWorld**支持，如果相应的实体不存在某个组件，那么在**std::function**中相应类型的指针将被赋值为**nullptr**
* template<COMPONENT_TYPE, ...ARGS> bool AttachTo(EntityID targetID, ...args)
  * 创建一个新的组件，并且将这个组件绑定到指定的ID上
  * 如果这个ID**无效**，返回**false**，否则返回**true**
  * 如果相关组件管理者的容量不足，将会触发断言
* template<COMPONENT_TYPE_LIST...> bool DoHas(EntityID targetID)
  * 检查某个实体是否包含相应的组件类型 *（多个）*
  * 如果实体不存在，将会触发断言
  * 如果DoHas的模板参数为**空**，或者存在不被**ECSWorld**支持的类型，将会引发编译错误。

</div>
</div>

