///////////////////////////////////////////////////////////////////////////////////////////////////////
// GNU Affero 3.0 April 23, 2023. 
// Copyright (C) 2022-2023 Useop Gim
// 
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _BASE_HASH_TABLE_H_
#define _BASE_HASH_TABLE_H_
////////////
// HEADER //
////////////
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <iostream>
#if __cplusplus >= 201703L

#include <any>
#include <typeindex>

using _usg_componets = std::unordered_map<std::type_index, std::any>;
using _usg_buckets = std::unordered_map<std::string, _usg_componets>;
#define _usg_any_cast(_TYPE, KEY) std::any_cast<_TYPE&>(bucket_list.at(KEY).at(typeid(_TYPE)))
//This contorls the table for ECS 
class HASH_TABLE{
    _usg_buckets bucket_list; //to control the object
    // Add in bucket by key and the object
    template<typename _T1> 
    void _Add(const std::string& key, _T1& _obj1){
        bucket_list[key][typeid(_T1)] =_obj1;
    }
    // Add in bucket by key and the object
    template<typename _T1, typename ...Args> 
    void _Add(const std::string& key, _T1& _obj1, Args& ...args){
        bucket_list[key][typeid(_T1)] = _obj1;
        _Add<Args...>(key, args...);
    }
    // Filtering by key from bucket 
    template<class _Tuple, size_t N, typename _T1> 
    void _Filter(const std::string& key, size_t& n, _Tuple& tup){
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
    // Filtering by key from bucket 
    template<class _Tuple, size_t N, typename _T1, typename _T2, typename ...Args>
    void _Filter(const std::string& key, size_t& n, _Tuple& tup){
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
    //Add the struct in the bucket
    template<typename... Args>
    void Add(const std::string& key, Args... args){
        _Add<Args...>(key, args...);
    }
    //Delete the components by given key
    void Delete(const std::string& key){
        bucket_list.erase(key);
    }
    //Get the components thrugh the key
    template<typename T>
    T* Get(const std::string& key){
        assert(bucket_list.count(key));
        assert(bucket_list.at(key).count(typeid(T)));
        return &std::any_cast<T>(bucket_list.at(key).at(typeid(T)));
    }
    //Find the components through the given typenames
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
    //Shoe the entity and components
    void View(){
        std::cout << bucket_list.size() << std::endl;
        for(auto& temp : bucket_list){
            std::cout << temp.first << " has "; 
            for(auto& tmp : temp.second){
                std::cout << tmp.first.name() << std::endl;
            }
            std::cout << std::endl;
        }
    }
};
#endif
#endif
