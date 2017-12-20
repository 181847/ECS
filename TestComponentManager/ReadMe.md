## 测试项目
### 测试目标： Componentmanager
检查ComonentManager的相关功能。

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
9 |	traverser components with Entity Manager	| 使用EntityManager遍历Entity的功能来查看能否正确遍历所有指定的Component类型，检查是否有遗漏和数据错误。