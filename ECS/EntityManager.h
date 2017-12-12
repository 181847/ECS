#pragma once

namespace ECS
{

// this class hold all the Entity
// it used the singleton pattern.
class EntityManager
{

public:
	EntityManager* getInstance();
private:
	EntityManager();
	~EntityManager();

	
};

}// namespace ECS


