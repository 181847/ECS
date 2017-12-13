这是一个实现EntityComponentSystem的项目

# Entity
实体的主要作用用来标识一个实体拥有哪种类型的Component，所以其内部不会存储Component，而是使用二进制的01来表明Component的类型

# Component
Component拥有具体的数据，每一种Component分别存储在各自的ComponentManager中
* Component内部包含一个EntityI
对应的ComponentManager提供通过EntityID的方法来获取Component指针

# EntityManager
EntityManager主要功能：
功能 | 
----|-----
分配EntityID |  
回收EntityID  |  
对每一个EntityID保存一串二进制码，每一位标志一种类型的Component  |  
内部使用类似池分配器的形式存储所有未分配的EntityID。
# ComponentManager
一个模板类，模板参数为Component的具体类型
* 内部自动维护一个Component的分配池，提供通过EntityID来返回对应的Component指针。
* 用哈希表来存储EntityID到Component的映射
 
