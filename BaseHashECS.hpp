//##############################################
//## GNU Affero 3.0 Useop Gim April 23, 2023. ##
//##############################################
#ifndef _BASE_HASH_TABLE_H_
#define _BASE_HASH_TABLE_H_
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <iostream>
#include <any>
#include <functional>


using _usg_componets = std::unordered_map<std::type_index, std::any>;
using _usg_buckets = std::unordered_map<std::string_view,_usg_componets>;
#define _usg_any_cast(_TYPE, KEY) std::any_cast<_TYPE&>(bucket_list.at(KEY).at(typeid(_TYPE)))

class HASH_TABLE{
    _usg_buckets bucket_list;
    template<typename _T1> 
    void _Add(const std::string_view key, _T1& _obj1){
        bucket_list[key][typeid(_T1)] =_obj1;
    }
    
    template<typename _T1, typename ...Args> 
    void _Add(const std::string_view key, _T1& _obj1, Args& ...args){
        bucket_list[key][typeid(_T1)] = _obj1;
        _Add<Args...>(key, args...);
    }
    
    template<class _Tuple, size_t N, typename _T1> 
    void _Filter(const std::string_view key, size_t& n, _Tuple& tup){
        if constexpr (std::is_pointer_v<_T1>){
            using _Trp = std::remove_pointer_t<_T1>;
            if(bucket_list.at(key).count(typeid(_Trp)) && ++n){
                std::get<N>(tup) = &_usg_any_cast(_Trp, key);
            }
        }
        else if(bucket_list.at(key).count(typeid(_T1)) && ++n){
            std::get<N>(tup) = _usg_any_cast(_T1, key);
        }         
    }
    
    template<class _Tuple, size_t N, typename _T1, typename _T2, typename ...Args>
    void _Filter(const std::string_view key, size_t& n, _Tuple& tup){
        if constexpr (std::is_pointer_v<_T1>){
            using _Trp = std::remove_pointer_t<_T1>;
            if(bucket_list.at(key).count(typeid(_Trp)) && ++n){
                std::get<N>(tup) = &_usg_any_cast(_Trp, key);
            }
        }
        else if(bucket_list.at(key).count(typeid(_T1)) && ++n){
            std::get<N>(tup) = _usg_any_cast(_T1, key);
        }
        _Filter<_Tuple, N + 1, _T2, Args...>(key, n, tup);
    }

public:

    template<typename... Args>
    void Add(const std::string_view key, Args... args){
        _Add<Args...>(key, args...);
    }

    void Delete(const std::string_view key){
        bucket_list.erase(key);
    }

    template<typename T>
    T* Get(const std::string_view key){
        assert(bucket_list.count(key));
        assert(bucket_list.at(key).count(typeid(T)));
        return &std::any_cast<T>(bucket_list.at(key).at(typeid(T)));
    }

    template<typename ...Args>
    void Filter(void(*func)(Args...)){
        for(auto& temp : bucket_list){
            std::tuple<Args...> tup;
            size_t check = 0;
            _Filter<std::tuple<Args...>, 0, Args...>(temp.first, check, tup);
            if(check == sizeof...(Args))
                std::apply(func, tup);
        }
    }
};

#endif
