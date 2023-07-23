#ifndef __UECS_HPP__	
#define __UECS_HPP__
/* Entity Component System version 1.0.1*/
#include "UMEMORY.h"
#include <queue>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <random>
#include <chrono>
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
		template <typename T1, typename T2>
		bool operator() (const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs) const {
			return lhs.first == rhs.first && lhs.second == rhs.second;
		}
	};


	struct World
	{
	private:
		unsigned int tick;
		std::chrono::high_resolution_clock::time_point lastTime;
		std::chrono::milliseconds timeInterval;
		std::chrono::milliseconds minTime;
		std::chrono::milliseconds maxTime;

		mutable std::unordered_map<uint32_t, const char*> hash_name;

		uint32_t hash_bytes(const char* ptr, size_t size)
		{
			uint32_t hash = 5381;
			const unsigned char* byte_ptr = (const unsigned char*)(ptr);
			for (size_t i = 0; i < size; ++i) {
				hash = ((hash << 5) + hash) + byte_ptr[i]; // DJB2 해시 함수
			}
			return hash;
		}

		uint32_t getHash(const char* name)
		{
			auto length = strlen(name);
			auto hash = hash_bytes(name, length);
			hash_name[hash] = name;
			return hash;
		}

	public:
		// Represents a unique object or entity in the ECS architecture.
		struct Entity  
		{
		private:
			World* world;
			entity_id id;
			type_id_list types;					
		friend struct World;

		public:
			Entity() : world(nullptr), id(0), types({}) {}
			Entity(World* world, entity_id id, type_id_list types) : world(world), id(id), types(types) {}
			rule2(Entity) : Entity(other.world, other.id, other.types) {}
			rule3(Entity) { world = std::exchange(other.world, world); id = std::exchange(other.id, 0); types = std::exchange(other.types, {}); }
			rule4(Entity) { return *this = Entity(other.world, other.id, other.types); }
			rule5(Entity) { world = std::exchange(other.world, world); id = std::exchange(other.id, 0); types = std::exchange(other.types, {});  return *this; }

			template<typename T>
			Entity& set(T const& comp)
			{
				world->set(id, comp);
				return *this;
			}

			template<typename T>
			T* get()
			{
				return world->get<T>(id);
			}

			World* get_world()
			{
				return world;
			}

			template<typename T>
			void remove()
			{
				world->remove<T>(id);
			}

			void destroy()
			{
				world->destroy(id);
			}
		};
	private:
		// Data-only unit that holds specific attributes of an entity in ECS.
		struct Comp
		{
		private:
			umemory::UANY comp;
			friend struct World;
		public:

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
		mutable std::unordered_map<data_index, Comp, PairHash, PairEqual> component_list;
		mutable std::queue<uint32_t> unique_id_q;
		// Responsible for the logic and behavior of entities with relevant components in ECS
		struct System
		{
		private:
			World* world = nullptr;
			systemfn func = nullptr;
			type_id_list types;
			bool active = true;
			friend struct World;
		public:

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

			bool HasComps(const type_id_list& entity_types, const type_id_list& system_types)
			{
				for (const auto& element : system_types) {
					if (entity_types.find(element) == entity_types.end()) {
						return false;
					}
				}
				return true;
			}

			void run() 
			{
				if (active)
					for (auto& entity : world->entities)
						if (HasComps(entity.second.types, types))
							func(entity.second);
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

		
		void updateTime()
		{
			std::chrono::steady_clock::time_point currentTime;
			do
			{
				currentTime = std::chrono::high_resolution_clock::now();
				if (lastTime == std::chrono::high_resolution_clock::time_point())
					lastTime = currentTime;
				timeInterval = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
			}
			while(timeInterval < minTime);
			lastTime = currentTime;
			if(timeInterval > maxTime)
				timeInterval = maxTime;
		}

		void updateTick()
		{
			tick += 1;  // increasing tick
			if (tick == 0xffffffff) tick = 0;
		}

		template<typename T>
		void set(entity_id _entity_id, T const& data)
		{
			// Setting component
			component_type_id type_id = getHash(typeid(T).name());
			data_index data_id = std::make_pair(_entity_id, type_id);
			ULOG(UDEBUG, "comp %s id %d connected to entity %d", typeid(T).name(), type_id, data_id);
			UASSERT(component_list.count(data_id) == 0, "comp %s id %d already exists in entity %d !",typeid(T).name(), type_id, data_id);
			component_list[data_id] = Comp(umemory::UANY(data));
			entities[_entity_id].types.insert(getHash(typeid(T).name()));
		}

		template<typename T>
		T* get(entity_id _entity_id)
		{
			component_type_id type_id = getHash(typeid(T).name());
			data_index data_id = std::make_pair(_entity_id, type_id);
			UASSERT(component_list.find(data_id) != component_list.end(), "get entity %d doesn't have given %s component!", _entity_id, typeid(T).name());
			return component_list.at(data_id).comp.Get<T>();
		}

		template<typename T>
		void remove(entity_id _entity_id)
		{
			component_type_id type_id = getHash(typeid(T).name());
			data_index data_id = std::make_pair(_entity_id, type_id);
			UASSERT(component_list.find(data_id) != component_list.end(), "remove entity %d doesn't have given %s component!", _entity_id, type_id);
			component_list.erase(component_list.find(data_id));
			entities[_entity_id].types.erase(type_id);
		}

		void destroy(entity_id _entity_id)
		{
			auto types = entities[_entity_id].types;
			for (auto type_id : types)
			{
				component_list[{ _entity_id, type_id }].comp.release();
				component_list.erase( component_list.find({ _entity_id, type_id}) );
			}
			entities.erase( entities.find(_entity_id) );
			unique_id_q.push(_entity_id);
		}

	public:
		// create empty world
		// the maximum number of entity is 65535
		World()
		{
			for (uint32_t i = 1; i < 0xFFFF; ++i)
			{
				unique_id_q.push(i);
			}
			lastTime = std::chrono::high_resolution_clock::time_point();
			timeInterval = std::chrono::milliseconds(0);
			minTime = std::chrono::milliseconds(0);
			maxTime = std::chrono::milliseconds(0);
		}
		// set system in specific Phase
		template<typename ...Args>
		void system(systemfn func, PHASE Phase)
		{
			type_id_list type_list;
			getTypeName<Args...>(type_list);
			System sys(this, func, type_list);
			systems[Phase].emplace_back(sys);
		}
		// disable specific system 
		void system_disable(systemfn func, PHASE Phase)
		{
			auto trg = std::find(systems[Phase].begin(), systems[Phase].end(), func);
			if(trg != systems[Phase].end())
				(*trg).disable();
		}
		// disable specific system 
		void system_enable(systemfn func, PHASE Phase)
		{
			auto trg = std::find(systems[Phase].begin(), systems[Phase].end(), func);
			if (trg != systems[Phase].end())
				trg->enable();
		}
		// setting Timer 
		// minTimer is the minimum millisecond for deltaTime
		// maxTimer is the maximum millisecond for deltatTime
		void TimerSetting(const long long& minTimer, const long long& maxTimer)
		{
			minTime = std::chrono::milliseconds(minTimer);
			maxTime = std::chrono::milliseconds(maxTimer);
		}
		// create entity with hash number
		Entity& entity()
		{
			entity_id id = unique_id_q.front(); 
			unique_id_q.pop();
			UASSERT(entities.count(id) == 0, "unique id already exists");
			entities[id] = Entity(this, id, {});
			return entities.at(id);
		}
		// If the name does not exist, create an entity with the specified name; 
		// otherwise, return the named entity.
		Entity& entity(const char* name)
		{			
			entity_id id = getHash(name);
			if (entities.count(id) == 0)
				entities[id] = Entity(this, id, {});				
			return entities.at(id);
		}
		// get current deltaTime
		float getDeltaTime()
		{
			std::chrono::duration<float, std::milli> duration_float = timeInterval;
			return duration_float.count();
		}
		// get current tick 
		// if tick is greater or equal than 4294967295
		// the given tick becomes 0
		unsigned int getTick()
		{
			return tick;
		}
		// running the Update type phase 
		// for OnUpdate -> Update -> OffUpdate
		void update_progress()
		{
			updateTick();
			updateTime();
			for (auto temp : systems[uecs::PHASE::OnUpdate])
				temp.run();
			for (auto temp : systems[uecs::PHASE::Update])
				temp.run();
			for (auto temp : systems[uecs::PHASE::OffUpdate])
				temp.run();
		}
		// running the Once type phase 
		// for Awake -> OnStart -> Start -> OffStart
		void once_progress() {
			for(auto temp : systems[uecs::PHASE::Awake])
				temp.run();	
			for(auto temp : systems[uecs::PHASE::OnStart])
				temp.run();
			for(auto temp : systems[uecs::PHASE::Start])
				temp.run();
			for(auto temp : systems[uecs::PHASE::OffStart])
				temp.run();
		}
	};

	using Entity = World::Entity;
}

#endif
