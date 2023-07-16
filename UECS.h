#ifndef __UECS_HPP__
#define __UECS_HPP__
#include "UMEMORY.h"
#include <queue>
#include <set>
#include <map>
// July 15th, 2023. USG (c)
// MIT License
namespace uecs
{
	typedef unsigned long long entity_id;
	typedef unsigned long long component_key;

	class world
	{
	private:
		std::map<const char*, component_key> regitKeys;
		entity_id id_max = 100;
		component_key key_max = 0;
		template<typename T>
		component_key find_key(void)
		{			
			if (regitKeys.find(typeid(T).name()) == regitKeys.end())
				regitKeys.insert( std::make_pair(typeid(T).name(), (1 << key_max++)) );
			return regitKeys.at(typeid(T).name());
		}
		void check_id(void)
		{
			if (entity_id_list.empty())
				for (entity_id i = id_max; i < id_max * 2; ++i)
					entity_id_list.push(i);
		}
	protected:
		std::queue<entity_id> entity_id_list;
	public:

		struct Entity
		{
		private:
			world* world = nullptr;
			entity_id id = 0;
		public:
			Entity(uecs::world* _world, entity_id _id) : world(_world), id(_id) {};
			Entity(const Entity& other) : Entity(other.world, other.id){};
			Entity(Entity&& other) noexcept
			{
				world	= std::exchange(other.world, nullptr);
				id		= std::exchange(other.id, 0);
			}
			Entity& operator = (Entity const& other)
			{
				return *this = Entity(other);
			}
			Entity& operator = (Entity&& other) noexcept
			{
				world	=	std::exchange(other.world, nullptr);
				id		=	std::exchange(other.id, 0);
				return *this;
			}
			~Entity()
			{
				world = nullptr;
			}

			template<typename T> Entity& SET(T comp)
			{
				world->SetComp(this, comp);
				return *this;
			}
			const entity_id get_id(void)
			{
				return id;
			}
			uecs::world* get_world(void)
			{
				return this->world;
			}
			template<typename T> T& GET()
			{
				return world->GetComp<T>(this);
			}
			void remove(void)
			{
				world->remove(id);
			}
		};

		std::map<entity_id,		_UM UANY> entities;
		std::map<component_key, std::map<entity_id, _UM UANY>>	components;

		world(void)
		{
			for (entity_id i = 1; i < id_max; ++i)
			{
				entity_id_list.push(i);
			}
		}
		Entity& entity(void)
		{			
			check_id();
			entity_id id = entity_id_list.front();
			entity_id_list.pop();
			entities[id] = _UM UANY(Entity({ this, id }));
			return entities.at(id).GET<Entity>();
		}
		Entity& entity(entity_id id)
		{
			return entities.at(id).GET<Entity>();
		}

		template<typename T>
		world& SetComp(Entity* const it, T& const comp)
		{
			component_key key = find_key<T>();
			components[key][it->get_id()] = _UM UANY(comp);
			return *this;
		}
		template<typename T>
		T& GetComp(Entity* const it)
		{
			component_key key = find_key<T>();				
			return components[key][it->get_id()].GET<T>();
		}

		void remove(entity_id id)
		{
			ULOG(UPOINT, "Entity Remove [%lld]", id);
			entities.erase(id);
		}
	};
}

#endif
