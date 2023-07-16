#ifndef __UECS_HPP__
#define __UECS_HPP__
#include "UMEMORY.h"
#include <queue>
#include <set>
#include <map>
/* USG (c) July 16th, 2023.
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
namespace uecs
{
	typedef unsigned long long entity_id;
	typedef unsigned long long component_key;
	typedef std::map<int, std::vector<_UM UANY*>> system_comps;

	static component_key key_max = 0;
	static std::map<const char*, component_key> regitKeys;
	template<typename T> static component_key find_key(void)
	{
		if (regitKeys.find(typeid(T).name()) == regitKeys.end())
			regitKeys.insert(std::make_pair(typeid(T).name(), (++key_max)));
		ULOG(UERROR, "GET KEY[%s] IS [%d]", typeid(T).name(), regitKeys.at(typeid(T).name()));
		return regitKeys.at(typeid(T).name());
	}

	class world
	{
	private:
		entity_id id_max = 100;
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
			uecs::world* const get_world(void)
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

		std::map<entity_id,	_UM UANY> entities;
		std::map<std::pair<entity_id, component_key>, _UM UANY> components;

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
		world& SetComp(Entity* const it, T& const comp) noexcept
		{
			component_key key = find_key<T>();
			components[std::make_pair(it->get_id(), key)] = _UM UANY(comp);
			return *this;
		}
		template<typename T>
		T& GetComp(Entity* const it)
		{
			component_key key = find_key<T>();				
			return components[key][it->get_id()].GET<T>();
		}

		struct Table
		{
			template<typename T>
			void GetKey(std::vector<component_key>& keys)
			{
				component_key key = find_key<T>();
				keys.emplace_back(key);
			}
			template<typename T, typename F>
			void GetKey(std::vector<component_key>& keys)
			{
				component_key key1 = find_key<T>();
				component_key key2 = find_key<F>();
				keys.emplace_back(key1);
				keys.emplace_back(key2);
			}
			template<typename T, typename ...Args>
			void GetKey(std::vector<component_key>& keys)
			{
				component_key key = find_key<T>();
				keys.emplace_back(key);
				GetKey<Args...>(keys);
			}
		};


		bool IsKeyInKeys(const std::vector<component_key>& vec, const std::set<component_key>& s) {
			return std::all_of(vec.begin(), vec.end(), [&s](int value) {
				return s.find(value) != s.end();
				});
		}
		template<typename ...Args>
		void SYSTEM()
		{
			std::vector<component_key> keys;
			//Table::GetKey<Args...>(keys);
			//std::vector<entity_id> trg;
			//for (auto entity : entities)
			//{
			//	if (IsKeyInKeys(keys, *(entity.second).GET<Entity>().get_keys()))
			//		trg.emplace_back(entity.first);
			//}
			//	
			//printf("test");
			//for (auto src : trg)
			//{
			//	printf("%d", src);
			//}
		}

		void remove(entity_id id)
		{
			ULOG(UPOINT, "Entity Remove [%lld]", id);
			entities.erase(id);
		}

	};
}

#endif
