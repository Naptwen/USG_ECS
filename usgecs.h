// GNU Affero License(c) Useop Gim 2023 Feb 19, 2023
// This is simple ECS system code 
// The version of c++ > ++17 is required!

#ifndef __USG_ECS_H__
#define __USG_ECS_H__

#include <iostream>
#include <functional>
#include <bitset>
#include <queue>

#define MAX_ENTITIES 4080
#define MAX_COMPONENTS 40

using EntityID = std::uint32_t;
using CompID = std::uint32_t;
using KeyList = std::bitset<MAX_COMPONENTS>;

namespace ECS
{
	class _Any
	{
	private:
		struct _Base
		{
			virtual ~_Base(void) = default;
		};

		template<class T>
		struct _Body : _Base
		{
			explicit _Body(T const& var) : mVar(var) {}
			T mVar;
			using Result = typename T;
		};
		_Base* mData;
	public:
		_Any(void) = default;
		template<class _Type>
		explicit _Any(_Type const& val) : mData(new _Body<_Type>(val)) {}
		template<class _Trans>
		_Trans* get(void)
		{
			using _Ry = std::remove_pointer_t<_Trans>;
			return &static_cast<_Body<_Ry>*>(mData)->mVar;
		}
		~_Any()
		{
			if (!mData)
				delete mData;
		}
	};

	class World
	{
	private:
		class Entity
		{
		protected:
			World* mWorld = nullptr;
			EntityID mID = 0;
			KeyList mKey;
			friend class World;
		public:
			explicit Entity(EntityID id, World* world) : mWorld(world), mID(id) {}
			Entity() = default;
			template<class T>
			Entity& add(T comp)
			{
				mKey[mWorld->getCompID<T>(mID, comp)] = true;
				return *this;
			}
			EntityID& GetID(void) { return mID; }
			void remove(void)
			{
				assert((mWorld->mEntities.count(mID), "There are no such Entity Id!"));
				mWorld->mComponents.erase(mID);
				mWorld->entity_id_list.push(mID);
				mWorld->mEntities.erase(mID);
			}
			template<typename T>
			void disable(void)
			{
				auto name = typeid(T).name();
				assert((mWorld->mCompIDs.count(name), "The component is not registed!"));
				if (mWorld->mComponents.at(mID).count(mWorld->mCompIDs.at(name)))
					mKey[mWorld->mCompIDs.at(name)] = false;
			}
			template<typename T>
			void enable(void)
			{
				auto name = typeid(T).name();
				assert((mWorld->mCompIDs.count(name), "The component is not registed!"));
				if (mWorld->mComponents.at(mID).count(mWorld->mCompIDs.at(name)))
					mKey[mWorld->mCompIDs.at(name)] = true;
			}
			template<typename T>
			void erase(void)
			{
				auto name = typeid(T).name();
				assert((mWorld->mCompIDs.count(name), "The component is not registed!"));
				if (mWorld->mComponents.at(mID).count(mWorld->mCompIDs.at(name)))
				{
					mKey[mWorld->mCompIDs.at(name)] = false;
					mWorld->mComponents.at(mID).erase(mWorld->mCompIDs.at(name));
				}
			}
			template<typename T>
			T& get()
			{
				auto compID = mWorld->mCompIDs.at(typeid(T).name());
				return std::ref(*mWorld->mComponents.at(mID).at(compID).get<T>());
			}
			KeyList& GetKey(void) { return mKey; }
			bool CheckKey(const KeyList& cmp, size_t num) const
			{
				size_t same = 0;
				for (int i = 0; i < MAX_COMPONENTS; ++i)
					if (mKey[i] == cmp[i]) same++;
				return (num <= same)? true : false;
			}
		};
		std::queue<EntityID> entity_id_list;
		std::queue<CompID> comp_id_list;
		std::map<std::string, CompID> mCompIDs;

		std::map<EntityID, Entity> mEntities;
		std::map<EntityID, std::map<CompID, _Any>> mComponents;
		
		template<typename _T1>
		void make_key(KeyList& key) const
		{
			using _T = std::remove_pointer_t<_T1>;
			if (mCompIDs.count(typeid(_T).name()))
				key[mCompIDs.at(typeid(_T).name())] = true;
		}
		template<typename _T1, typename _T2, typename ...Args>
		void make_key(KeyList& key) const
		{
			using _T = std::remove_pointer_t<_T1>;
			if (mCompIDs.count(typeid(_T).name()))
				key[mCompIDs.at(typeid(_T).name())] = true;
			make_key<_T2, Args...>(key);
		}

		template<class _TUPLE, size_t N, class F>
		void _filter(_TUPLE& values, const EntityID entiyId) 
		{
			using _T1 = std::remove_pointer_t<F>;
			auto name = typeid(_T1).name();
			auto compId = mCompIDs.at(name);
			assert((mComponents.at(entiyId).count(compId), "There are no such variable in given Entity"));
			std::get<N>(values) = mComponents.at(entiyId).at(compId).get<_T1>();
		}
		template<class _TUPLE, size_t N, class F, class S, class ...Vars>
		void _filter(_TUPLE& values, const EntityID entiyId)
		{
			using _T1 = std::remove_pointer_t<F>;
			auto name = typeid(_T1).name();
			auto compId = mCompIDs.at(name);
			assert((mComponents.at(entiyId).count(compId), "There are no such variable in given Entity"));
			mComponents.at(entiyId).at(compId).get<_T1>();
			std::get<N>(values) = mComponents.at(entiyId).at(compId).get<_T1>();
			_filter<_TUPLE, N + 1, S, Vars...>(values, entiyId);
		}
	protected:
		template<class T>
		CompID getCompID(EntityID ID, T comp)
		{
			auto _Type = typeid(T).name();
			if (!mCompIDs.count(_Type))
			{
				mCompIDs[_Type] = comp_id_list.front();
				comp_id_list.pop();
			}
			mComponents[ID][mCompIDs[_Type]] = _Any(comp);
			auto compID = mCompIDs.at(typeid(T).name());
			return compID;
		}
		friend class Entity;
	public:
		World()
		{
			for (int i = 0; i < MAX_ENTITIES; ++i)
				entity_id_list.push(i);
			for (int i = 0; i < MAX_COMPONENTS; ++i)
				comp_id_list.push(i);
		}
		Entity& entity(void)
		{
			EntityID id = entity_id_list.front();
			entity_id_list.pop();
			mEntities[id] = Entity(id, this);
			return mEntities.at(id);
		}
		Entity& entity(const EntityID id)
		{
			assert((mEntities.count(id), "There are no such entity id!"));
			return mEntities.at(id);
		}
		template<typename R, typename ...Args>
		std::vector<EntityID> system(R(*func)(Args...))
		{
			std::vector<EntityID> entityID;
			std::vector<std::tuple<Args...>> vec;
			KeyList duplicated_key;
			make_key<Args...>(duplicated_key);
			for (auto& kv : mEntities)
			{
				if (kv.second.CheckKey(duplicated_key, sizeof ...(Args)))
				{
					std::tuple<Args...> values;
					_filter<std::tuple<Args...>, 0, Args...>(values, kv.first);
					vec.emplace_back(values);
					entityID.emplace_back(kv.first);
				}
			}
			std::for_each(vec.begin(), vec.end(), [&func](std::tuple<Args...>& _Val) {std::apply(func, _Val); });
			return entityID;
		}
		template<typename ...Args>
		struct _Lambda
		{
			World* mWorld = nullptr;
			_Lambda(World* world) : mWorld(world) {}

			template<typename F>
			void each(F&& func)
			{
				std::vector<std::tuple<Args...>> vec;
				KeyList duplicated_key;
				mWorld->make_key<Args...>(duplicated_key);
				for (auto& kv : mWorld->mEntities)
				{
					if (kv.second.CheckKey(duplicated_key, sizeof ...(Args)))
					{
						std::tuple<Args...> values;
						mWorld->_filter<std::tuple<Args...>, 0, Args...>(values, kv.first);
						vec.emplace_back(values);
					}
				}
				std::for_each(vec.begin(), vec.end(), [&func](std::tuple<Args...>& _Val) {std::apply(func, _Val); });
			}
		};

		template<typename ...Args>
		_Lambda<Args...> filter(void)
		{
			return _Lambda<Args...>(this);
		}
		
		void Show(void)
		{
			printf("_________________________________________________________\n");
			printf("Entity ID  | Key        |");
			for (const auto& compid : mCompIDs)
				printf(" %*s |", 11, compid.first.substr(6, 10).c_str());
			printf("\n");
			for (const auto& row : mComponents)
			{
				printf("%10d | ", row.first);
				printf("%10d | ", mEntities.at(row.first).mKey);
				for (const auto& compid : mCompIDs)
				{
					if (row.second.count(compid.second))
						printf(" TRUE       | ");
					else
						printf(" FALSE      | ");
				}

				printf("\n");
			}
		}
	};
}
#endif
