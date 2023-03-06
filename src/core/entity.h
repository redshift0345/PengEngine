#pragma once

#include <vector>
#include <memory>

#include <memory/weak_ptr.h>
#include <math/transform.h>

#include "tickable.h"
#include "component.h"
#include "entity_relationship.h"
#include "entity_definition.h"

class Entity : public ITickable, public std::enable_shared_from_this<Entity>
{
	DECLARE_ENTITY(Entity);

public:
	explicit Entity(std::string&& name, TickGroup tick_group = TickGroup::standard);
	explicit Entity(const std::string& name, TickGroup tick_group = TickGroup::standard);

	Entity(const Entity&) = delete;
	Entity(Entity&&) = delete;
	virtual ~Entity() = default;

	void tick(float delta_time) override;
	[[nodiscard]] TickGroup tick_group() const noexcept override;

	virtual void post_create();
	virtual void pre_destroy();
	virtual void post_enable() { }
	virtual void post_disable() { }

	void set_active(bool active);
	void set_parent(const peng::weak_ptr<Entity>& parent, EntityRelationship relationship = EntityRelationship::full);
	void destroy();

	template <std::derived_from<Component> T, typename...Args>
	peng::weak_ptr<T> add_component(Args&&...args);

	// TODO: add get_component and friends

	[[nodiscard]] peng::weak_ptr<const Entity> weak_this() const;
	[[nodiscard]] peng::weak_ptr<Entity> weak_this();

	[[nodiscard]] const std::string& name() const noexcept { return _name; }
	[[nodiscard]] bool active_in_hierarchy() const noexcept { return _active_hierarchy; }
	[[nodiscard]] bool active_self() const noexcept { return _active_self; }

	[[nodiscard]] peng::weak_ptr<Entity> parent() noexcept { return _parent; }
	[[nodiscard]] peng::weak_ptr<const Entity> parent() const noexcept { return _parent; }
	[[nodiscard]] const std::vector<peng::weak_ptr<Entity>>& children() const noexcept { return _children; }

	[[nodiscard]] bool has_parent() const noexcept;
	[[nodiscard]] bool has_spatial_parent() const noexcept;
	[[nodiscard]] bool has_activity_parent() const noexcept;

	[[nodiscard]] math::Matrix4x4f transform_matrix() const noexcept;
	[[nodiscard]] math::Matrix4x4f transform_matrix_inv() const noexcept;
	[[nodiscard]] math::Vector3f world_position() const noexcept;
	[[nodiscard]] math::Transform& local_transform() noexcept { return _local_transform; }
	[[nodiscard]] const math::Transform& local_transform() const noexcept { return _local_transform; }
	[[nodiscard]] const std::vector<peng::shared_ref<Component>>& components() const noexcept { return _components; }

	template <std::derived_from<Entity> T>
	[[nodiscard]] static peng::weak_ptr<T> weak_from(T& entity);

	template <std::derived_from<Entity> T>
	[[nodiscard]] static peng::weak_ptr<const T> weak_from(const T& entity);

protected:
	std::string _name;
	TickGroup _tick_group;
	math::Transform _local_transform;

private:
	void propagate_active_change(bool parent_active);

	bool _created;
	bool _active_self;
	bool _active_hierarchy;

	peng::weak_ptr<Entity> _parent;
	EntityRelationship _parent_relationship;

	std::vector<peng::weak_ptr<Entity>> _children;
	std::vector<peng::shared_ref<Component>> _components;
	std::vector<peng::shared_ref<Component>> _deferred_components;
};

template <std::derived_from<Component> T, typename ... Args>
peng::weak_ptr<T> Entity::add_component(Args&&... args)
{
	peng::shared_ref<T> component = peng::make_shared<T>(std::forward<Args>(args)...);
	_components.push_back(component);

	if (_created)
	{
		component->set_owner(peng::shared_ref(shared_from_this()));
		component->post_create();
	}
	else
	{
		_deferred_components.push_back(component);
	}

	return component;
}

template <std::derived_from<Entity> T>
peng::weak_ptr<T> Entity::weak_from(T& entity)
{
	return peng::weak_ptr<T>(std::static_pointer_cast<T>(entity.shared_from_this()));
}

template <std::derived_from<Entity> T>
peng::weak_ptr<const T> Entity::weak_from(const T& entity)
{
	return peng::weak_ptr<const T>(std::static_pointer_cast<const T>(entity.shared_from_this()));
}
