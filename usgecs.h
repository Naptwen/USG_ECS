# MIT License(c) Useop Gim 2023 Feb 19, 2023
# This is simple ECS system code
# The apply code for < ++17 is 
# referenced from 
# htps://stackoverflow.com/questions/687490/how-do-i-expand-a-tuple-into-variadic-template-functions-arguments 
#include <iostream>
#include <functional>
#include <bitset>
#include <queue>

#if __cplusplus  == 201402L or   __cplusplus  == 199711L or  __cplusplus  == 1
# Basic Apply Code reference from DRayX
# htps://stackoverflow.com/questions/687490/how-do-i-expand-a-tuple-into-variadic-template-functions-arguments
namespace std
{
	template<size_t N>
	struct Apply {
		template<typename F, typename T, typename... A>
		static inline auto apply(F&& f, T&& t, A &&... a)
			-> decltype(Apply<N - 1>::apply(std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...))
		{
			return Apply<N - 1>::apply(std::forward<F>(f), std::forward<T>(t),
				std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...
			);
		}
	};

	template<>
	struct Apply<0> {
		template<typename F, typename T, typename... A>
		static inline auto apply(F&& f, T&&, A &&... a)
			-> decltype(std::forward<F>(f)(std::forward<A>(a)...))
		{
			return std::forward<F>(f)(std::forward<A>(a)...);
		}
	};

	template<typename F, typename T>
	inline auto apply(F&& f, T&& t)
		-> decltype(Apply< std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t)))
	{
		return Apply< std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t));
	}
}
#endif

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
			T mVar;
			using Result = typename T;
			explicit _Body(T const& var) : mVar(var) {}
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
			KeyList& GetKey(void) { return mKey; }
			bool CheckKey(const KeyList& cmp) const
			{
				for (int i = 0; i < MAX_COMPONENTS; ++i)
					if (mKey[i] ^ cmp[i]) return false;
				return true;
			}
		};
		std::queue<EntityID> entity_id_list;
		std::queue<CompID> comp_id_list;
		std::map<std::string, CompID> mCompIDs;

		std::map<EntityID, Entity> mEntities;
		std::map<EntityID, std::map<CompID, _Any>> mComponents;


		template<typename _Return, class _FUNC, class _TUPLE, size_t N, class F>
		_Return runSystem(_FUNC& _Func, _TUPLE& values, const EntityID entiyId)
		{
			using _T1 = std::remove_pointer_t<F>;
			auto name = typeid(_T1).name();
			auto compId = mCompIDs.at(name);
			assert((mComponents.at(entiyId).count(compId), "There are no such variable in given Entity"));
			std::get<N>(values) = mComponents.at(entiyId).at(compId).get<_T1>();
			return std::apply(_Func, values); //c++17 requires
		}
		template<typename _Return, class _FUNC, class _TUPLE, size_t N, class F, class S, class ...Vars>
		_Return runSystem(_FUNC& _Func, _TUPLE& values, const EntityID entiyId)
		{
			using _T1 = std::remove_pointer_t<F>;
			auto name = typeid(_T1).name();
			auto compId = mCompIDs.at(name);
			assert((mComponents.at(entiyId).count(compId), "There are no such variable in given Entity"));
			mComponents.at(entiyId).at(compId).get<_T1>();
			std::get<N>(values) = mComponents.at(entiyId).at(compId).get<_T1>();
			return runSystem<_Return, _FUNC, _TUPLE, N + 1, S, Vars...>(_Func, values, entiyId);
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
		R system(R(*func)(Args...))
		{
			std::tuple<Args...> values;
			return runSystem<R, R(*)(Args...), std::tuple<Args...>, 0, Args...>(func, values, 2);
		}
		void Show(void)
		{
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
