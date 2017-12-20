## 测试项目
### 测试目标： EntityManager
检查EntityManager的相关功能。

测试单元序号	| 相关文字	| 解释
----|----|------
1	|test errorLogger, this is not the test about ComponentManager.| 这个测试单元与本项目基本无关，只是用来测试一下新加入测试工具的类
2 | create a ComponentManager with TestStructA	|	创建几个EntityID，然后在一个包含TestStructA的manager中创建组件实例，检查创建是否成功
3 |	get component from ComponentManager | 创建Entity，TestStructA类型的组件，使用EntityID从Manager中获取组件指针，检查能否获取成功
4 | get component from ComponentManager using the operator [] | 使用数组下标重载获取组件指针的函数，方便调用。
5 |	can pass the argument to the component constructor?	|	测试在创建Component的时候能否将传入的其他参数完美转发到TestStructA的构造函数中
6 | can the entityID be set into the component?	|	测试新创建的Component是否被正确设置EntityID。
7 | ensure that when we remove the entityID from the componentManager, the deconstructor is called	| 测试使用manager删除组件的时候，析构函数能够被正常调用。
8 | random test on componentManager	|	使用大量随机的EntityID创建Component，测试Component是否成功创建，并且循环测试多次。
9 |	traverser components wit

#### 其他可用文件：
##### TestComponentTypes.h  
定义了简单的Component类型，用于帮助测试Entity蒙版组件类型。

##### TestEntityManagerTool.h
定义了一些测试过程中方便使用的函数和结构体

结构体名	| 说明
----|-----
CheckComponentsResult	| 检查Component的结果，记录了发生错误的数量以及遍历的Component的数量。
ComponentTypeContainer	| 这个类型使用模板来记录多个组件的类型，并且封装另外两个帮助函数，它的主要作用是将一组组件类型封装在一起，能够方便的为Entity蒙版，并且检查Entity的蒙版类型。
CMPS	| ComponentTypeContainer的别名，减少代码量。
ComponentNumber	|	枚举类型，定义了三种组件类型的排列组合，分配对应Int、Float、Char Component， 每个枚举值对应其在一个IDListArray（已分配的EntityID的数组的数组：vector\<vector\<EntityID\>\>）中的元素序号，比如ComponentNumber::FloatC的值为2，默认认为IDListArray\[ComponentNumber\:\:FloatC\]中存储的EntityID都是只分配了FloatComponent的实体。


函数名 | 说明
------|-----
newEntitis	|	批量创建新的EntityID
destoryEntities	|	批量删除EntityID
testEntityManager	| 简单的测试EntityManager创建、删除EntityID的函数，在内部循环创建EntityID，然后随机删除全部，查看每一次循环的结果能否让EntityManager恢复初始状态。
maskIdListWithComponentTypes	| 为多个EntityID同时蒙版相同的组件类型。
HelpTraverseEntities	|	在EntityManager中遍历某一类包含相同类型组件的entity，使用entityManager的haveComponent函数检查这些entity的蒙版类型是否正确
maskIdListWithComponentContainer	| 对一个IDListArray（包含多个IDList的数组）中使用多个组件组合（*ComponentContainer*）进行蒙版，主要是为了方便测试代码的编写，能够在一句调用中实现对多个IDList的不同组件组合的蒙版。
checkMaskResultWithComponentContainer	|	对一个IDListArray（包含多个IDList的数组）中，使用多个组件组合进行检查，确保通过EntityManager遍历到的Entity均包含对应的Component类型蒙版。
randomIdListArray	|	创建随机的大小的多个IDList，为每个IDList分配随机的EntityID。