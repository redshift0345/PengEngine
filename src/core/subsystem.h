#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <unordered_map>

// ReSharper disable once CppUnusedIncludeDirective
#include "subsystem_definition.h"

class Subsystem
{
public:
    template <std::derived_from<Subsystem> T>
    static void load();

    static void start_all();
    static void shutdown_all();

    Subsystem() = default;
    Subsystem(const Subsystem&) = delete;
    Subsystem(Subsystem&&) = delete;
    Subsystem& operator=(const Subsystem&) = delete;
    Subsystem& operator=(Subsystem&&) = delete;
    virtual ~Subsystem() = default;

    virtual void start() = 0;
    virtual void shutdown() = 0;

protected:
    template <std::derived_from<Subsystem> T>
    static T& get();

private:
    static std::vector<std::unique_ptr<Subsystem>> _subsystems;
    static std::unordered_map<const type_info*, size_t> _subsystem_map;
};

template <std::derived_from<Subsystem> T>
void Subsystem::load()
{
    const type_info* id = &typeid(T);
    assert(!_subsystem_map.contains(id));

    _subsystem_map[id] = _subsystems.size();
    _subsystems.push_back(std::make_unique<T>());
}

template <std::derived_from<Subsystem> T>
T& Subsystem::get()
{
    const type_info* id = &typeid(T);
    assert(_subsystem_map.contains(id));

    return static_cast<T&>(*_subsystems[_subsystem_map[id]]);
}