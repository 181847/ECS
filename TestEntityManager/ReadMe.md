## 测试项目
### 测试目标： EntityManager
检查EntityManager的相关功能。

测试单元序号	| 相关文字	| 解释
----|----|------
1 | always success test unit	| 和项目无关的单元，总是成功
2 | check IDGenerator's newID() and getID()	|	和EntityManager无关的测试单元，用于测试IDGenerator能够根据不同的类型返回不同的ID值
3 |	create EntityManager	|	测试能否创建和获取EntityManager，简单地从里面分配一个EntityID。
4 | generate ID, and random destroy	|	创建ID并且随机释放测试。
5 | test checkInvalid EntityID	| 检测EntityManager检测ID是否激活的功能，创建一组ID，将其中一部分删除，检测EntityManager能否对删除的和未删除的作出正确判断。
6 | test componentMask use EntityManager	|	测试EntityManager为Entity蒙版组件类型的功能，首先对IDList分成两个部分，分别蒙版不同的类型，接着对前一个部分蒙版重复类型，查看能否返回*冗余信号*， 后一部分蒙版新类型，查看能否返回成功。
7 | get mask of components	|	这个测试单元**不是**对应EntityManager的功能，而是用来测试ECS命名空间下的一个函数能否返回正确的组件类型蒙版。
8 |	check componentType of the entity	|	测试EntityManager\:\:haveComponent()函数能否正确判断实体所包含的组件类型。
9 | get entityIter	|	简单的测试一个EntityManager::RangeEntities()的返回情况，实际作用不大
10 |	test traverse the entity.	|	测试EntityManager遍历指定组件类型Entity的功能，首先创建随机entityID， 接着蒙版不同的Component类型，在遍历的过程中检查相应的组件数量是否和创建的一致。

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