# 功能
* EntityManager：分配EntityID，每个EntityID包括一个类型蒙版表明这个Entity包含哪些类型的组件
* ComponentManager：通过EntityID存取组件对象
* ECSWorld：管理以上两种类型的Manager，统一规范（**尚未完全实现**）

#### EntityManager\<Traits\>
<div style="background: #999999">

##### 模板Traits

此模板应该定义以下常量或类型：

常量或类型 | 常量名 | 作用
---------|--------|-------
const size_t | MaxEntityCount | EntityManager可以分配的最大Entity数量
const size_t | MaxComponentTypes | EntityManager可以区分的最大数量的组件类型
类型声明    | **ComponentMask** | 定义蒙版类型，用来标记Entity所拥有的蒙版，就目前而言，必须使用std::bitset
类型声明    | **ComponentIDGenerator** | 组件类型ID生成器，用于生成组件**类型**的ID值，必须实现**size_t IDOf\<T\>()**，考虑使用TypeTool::IDGenerator

<div style="background: #aaaaaa">

##### 函数：
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

#### CompoentManager\<COMPONENT_TYPE, Traits\>
<div style="background: #999999">

##### 模板Traits

此模板应该定义以下常量：

常量类型 | 常量名 | 作用
---------|--------|-------
const size_t | MaxSize | 可以分配的最大的Component的数量。

<div style="background: #aaaaaa">

##### 函数：
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

