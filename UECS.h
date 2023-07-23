#ifndef __UECS_HPP__	
#define __UECS_HPP__
#include "UMEMORY.h"
#include <queue>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <random>
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
	enum class PHASE
	{
		Awake = 717,
		OnStart,
		Start,
		OffStart,
		OnUpdate,
		Update,
		OffUpdate,
		Disalbe
	};

	using entity_id = uint32_t;
	using component_type_id = uint32_t;
	using component_id = uint32_t;
	using data_index = std::pair<entity_id, component_type_id>;

	using phase = const char*;
	using type_id_list = std::unordered_set<component_type_id>;

	struct PairHash {
		template <class T1, class T2>
		std::size_t operator()(const std::pair<T1, T2>& p) const {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			return h1 ^ h2;
		}
	};

	struct PairEqual {
		template <class T1, class T2>
		bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs) const {
			// 해당 PairEqual 구조체의 비교 함수를 정의
			return lhs.first == rhs.first && lhs.second == rhs.second;
		}
	};

	uint32_t hash_bytes(const void* ptr, size_t size) 
	{
		uint32_t hash = 5381;
		const unsigned char* byte_ptr = (const unsigned char*)ptr;
		for (size_t i = 0; i < size; ++i) {
			hash = ((hash << 5) + hash) + byte_ptr[i]; // DJB2 해시 함수
		}
		return hash;
	}

	uint32_t getHash(const char* name)
	{
		return hash_bytes(name, sizeof(name));
	}

	struct World
	{

	public:

		struct Entity  // has unique id and components id
		{
			World* world;
			entity_id id;
			type_id_list types;
			Entity() : world(nullptr) {}
			Entity(World* world, entity_id id, type_id_list types) : world(world), id(id), types(types) {}
			rule2(Entity) : Entity(other.world, other.id, other.types) {}
			rule3(Entity) { world = std::exchange(other.world, world); id = std::exchange(other.id, 0); types = std::exchange(other.types, {}); }
			rule4(Entity) { return *this = Entity(other.world, other.id, other.types); }
			rule5(Entity) { world = std::exchange(other.world, world); id = std::exchange(other.id, 0); types = std::exchange(other.types, {});  return *this; }

			template<typename T>
			Entity& set(T const& comp)
			{
				world->set(id, comp);
				types.insert(getHash(typeid(T).name()));
				return *this;
			}

			template<typename T>
			T* get()
			{
				return world->get<T>(id);
			}

			template<typename T>
			void remove()
			{
				world->remove<T>(id);
				types.erase( types.find( getHash( typeid(T).name() ) ) );
			}
		};
	private:
		struct Comp
		{
			umemory::UANY comp;
			Comp() = default;
			Comp(umemory::UANY comp) : comp(comp) {}
			rule2(Comp) : Comp(other.comp) {}
			rule3(Comp) { std::swap(other.comp, comp); }
			rule4(Comp) { return *this = Comp(other.comp); }
			rule5(Comp) { std::swap(other.comp, comp); return *this; }
			template<typename T>
			Comp(T& data)
			{
				comp = umemory::UANY<T>(data);
			}
		};

		using systemfn = void(*)(Entity&);
		mutable std::unordered_map<entity_id, Entity> entities;
		mutable std::unordered_map<data_index, Comp, PairHash> component_list;
		std::queue<uint32_t> unique_id_q;

		struct System
		{
			World* world = nullptr;
			systemfn func = nullptr;
			type_id_list types;
			bool active = true;

			System(World* world, systemfn func, type_id_list types, bool active = true) : world(world), func(func), types(types), active(active) {}
			rule2(System) : System(other.world, other.func, other.types, other.active) {}
			rule3(System) { std::swap(other.world, world); std::swap(other.func, func); std::swap(other.types, types); std::swap(other.active, active); }
			rule4(System) { return *this = System(other.world, other.func, other.types, other.active); }
			rule5(System) { std::swap(other.world, world); std::swap(other.func, func); std::swap(other.types, types); std::swap(other.active, active); return *this; }
			
			bool operator == (System const& other)
			{
				return func == other.func;
			}

			bool operator == (systemfn const& func)
			{
				return func == func;
			}

			void run() 
			{
				if (active)
				{
					for (auto& temp : world->entities)
					{
						if (temp.second.types == types)
						{
							func(temp.second);
						}	
					}	
				}					
			}
			void disable()
			{
				active = false;
			}
			void enable()
			{
				active = true;
			}
		};

		mutable std::map<PHASE, std::vector<System>> systems;

		template<typename T>
		void getTypeName(type_id_list& type_list) {
			type_list.insert(getHash(typeid(T).name()));
		}

		template<typename T, typename ...Args>
		typename std::enable_if<(sizeof...(Args) != 0)>::type
			getTypeName(type_id_list& type_list) {
			type_list.insert(getHash(typeid(T).name()));
			getTypeName<Args...>(type_list);
		}
	public:

		template<typename T>
		void set(entity_id entity_id, T const& data)
		{	
			// Setting component
			component_type_id type_id = getHash(typeid(T).name());
			data_index data_id = std::make_pair(entity_id, type_id);
			UASSERT(component_list.find(data_id) == component_list.end(), "unique comp id already exists!");
			component_list[data_id] = Comp(umemory::UANY(data));
		}

		template<typename T>
		T* get(entity_id entity_id)
		{
			component_type_id type_id = getHash(typeid(T).name());
			data_index data_id = std::make_pair(entity_id, type_id);
			UASSERT(component_list.find(data_id) != component_list.end(), "entity %d doesn't have given %s component!", entity_id, typeid(T).name());
			return component_list.at(data_id).comp.Get<T>();
		}

		template<typename T>
		void remove(entity_id entity_id)
		{
			component_type_id type_id = getHash(typeid(T).name());
			data_index data_id = std::make_pair(entity_id, type_id);
			UASSERT(component_list.find(data_id) == component_list.end(), "entity doesn't have given component!");
			component_list.erase(component_list.find(data_id));
		}

		template<typename ...Args>
		void system(systemfn func, PHASE Phase)
		{
			type_id_list type_list;
			getTypeName<Args...>(type_list);
			System sys(this, func, type_list);
			systems[Phase].emplace_back(sys);
		}

		void sys_disable(systemfn func, PHASE Phase)
		{
			auto trg = std::find(systems[Phase].begin(), systems[Phase].end(), func);
			if(trg != systems[Phase].end())
				(*trg).disable();
		}

		void sys_enable(systemfn func, PHASE Phase)
		{
			auto trg = std::find(systems[Phase].begin(), systems[Phase].end(), func);
			if (trg != systems[Phase].end())
				trg->enable();
		}

		World()
		{
			for (uint32_t i = 1; i < 100; ++i) 
			{
				unique_id_q.push(i);
			}
		}
		
		Entity& entity()
		{
			entity_id id = unique_id_q.front(); 
			unique_id_q.pop();
			UASSERT(entities.count(id) == 0, "unique id already exists");
			entities[id] = Entity(this, id, {});
			return entities.at(id);
		}

		Entity& entity(const char* name)
		{			
			entity_id id = getHash(name);
			UASSERT(entities.count(id) == 0, "hash id already exists");
			entities[id] = Entity(this, id, {});
			return entities.at(id);
		}

		void progress()
		{
			for (auto& phase : systems)
			{
				for (auto& sys : phase.second)
				{
					sys.run();
					if (phase.first != PHASE::OffUpdate && phase.first != PHASE::Update && phase.first != PHASE::OnUpdate)
					{
						sys.disable();
					}	
				}
			}
		}
	};

}

#endif
