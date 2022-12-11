#include <iostream>
#include <typeindex>
#include <stdarg.h>
template<class _Tp>
class CustomArray
{
private:
    _Tp** _Cta = (_Tp**)malloc(sizeof(_Tp*));
    uint32_t _Cta_sz = 0;
    uint32_t _Ctm_sz = 1;
public:
    template<typename ...Args>
    _Tp* push(const Args ... var)
    {
        _Tp* _Src_p = new _Tp({ var... });
        _Cta[_Cta_sz++] = _Src_p;
        if (_Ctm_sz <= _Cta_sz)
        {
            _Ctm_sz += 2;
            _Cta = (_Tp**)realloc(_Cta, sizeof(_Tp*) * _Ctm_sz);
        }
        return _Src_p;
    }
    _Tp* operator[](const uint32_t index)
    {
        if (index < _Cta_sz)
            return _Cta[index];
        return nullptr;
    }
    _Tp* search(const _Tp* _Src)
    {
        if (_Cta_sz != 0)
            for (_Tp* _Ps = _Cta[0]; _Ps != _Cta[_Cta_sz - 1]; _Ps++)
                if (_Ps == _Src)
                    return _Ps;
        return nullptr;
    }
    uint32_t size()
    {
        return _Cta_sz;
    }
    ~CustomArray()
    {
        for (_Tp* _Ps = _Cta[0]; _Ps != _Cta[_Cta_sz - 1]; _Ps++)
            free(_Ps);
        free(_Cta);
    }
};

class ENTITY;

struct COMPONENT
{
    std::type_index _tag;
    void* _cmp_ptr;
    ENTITY* _ent_ptr;
};

class ECS
{
private:
public:
    CustomArray<ENTITY> _Ct_entities;
    CustomArray<COMPONENT> _Ct_components;
    ENTITY& entity(const char* name)
    {
        std::cout << "entity is created" << std::endl;
        return *_Ct_entities.push<void*, std::string>((void*)this, name);
    }
    template<typename _Tp, typename ..._Args> constexpr
        void component(ENTITY* const _Sentity, _Args ..._vars)
    {
        std::cout << "component is created" << std::endl;
        _Tp* _Tcmp = new _Tp({ _vars ... });
        _Ct_components.push<std::type_index, void*, ENTITY*>(typeid(_Tp), _Tcmp, _Sentity);
        std::cout << "cmp is applied" << std::endl;
    }
    template<class ...Args, typename _Tf>
    void run(_Tf _fn)
    {
        auto temp = sizeof ...(Args);
        std::cout << "run is start with tag " << temp << std::endl;
        va_list ap;
        va_start(ap, temp);
        for (int j = 0; j < temp; ++j)
        {
            arg = va_arg(ap, Args)
            for (int i = 0; i < _Ct_components.size(); ++i)
            {
                if (_Ct_components[i] == )
            }
        }
    }
};

class ENTITY
{
public:
    void* _world;
    std::string _name;
    ENTITY(void* ptr, const std::string name)
    {
        _world = ptr;
        _name = name;
    }
    template<class _Tp, typename ..._Args>
    ENTITY& add(_Args ... vars)
    {
        ((ECS*)_world)->component<_Tp>(this, vars...);
        std::cout << "register entity with component" << std::endl;
        return *this;
    }
};

struct Velocity
{
    float x = 0;
    float y = 0;
};
struct Position
{
    float x = 0;
    float y = 0;
};

bool MOVE(Position a, Velocity b)
{
    a.x += b.x;
    a.y += b.y;
    return false;
}

int main()
{
    ECS world;
    auto k = world.entity("test");
    k.add<Velocity, float, float>(1.0, 1.0);
    world.run<Position, Velocity>(MOVE);
    return 0;
}
