#include "entity.h"

#include <cassert>
#include <utils/utils.h>

#include "peng_engine.h"

Entity::Entity(std::string&& name, TickGroup tick_group)
	: _name(std::move(name))
	, _tick_group(tick_group)
	, _created(false)
	, _active_self(true)
	, _active_hierarchy(true)
{ }

Entity::Entity(const std::string& name, TickGroup tick_group)
	: Entity(utils::copy(name), tick_group)
{ }

void Entity::tick(float)
{
	assert(_tick_group != TickGroup::none);
}

TickGroup Entity::tick_group() const noexcept
{
	return _tick_group;
}

void Entity::post_create()
{
	_created = true;

	for (const peng::shared_ref<Component>& component : _deferred_components)
	{
		component->set_owner(peng::shared_ref(shared_from_this()));
		component->post_create();
	}

	_deferred_components.clear();
}

void Entity::pre_destroy()
{
	for (const peng::shared_ref<Component>& component : _components)
	{
		component->pre_destroy();
	}
}

void Entity::set_active(bool active)
{
	_active_self = active;
	propagate_active_change(true);
}

void Entity::set_parent(const peng::weak_ptr<Entity>& parent)
{
	const bool was_active_hierarchy = _active_hierarchy;

	if (parent == _parent)
	{
		return;
	}

	if (_parent.valid())
	{
		vectools::remove(_parent->_children, weak_this());
		_active_hierarchy = _active_self;
	}

	_parent = parent;

	if (_parent.valid())
	{
		_parent->_children.push_back(weak_this());
		_active_hierarchy = _active_self && _parent->active_in_hierarchy();
	}

	if (_active_hierarchy && !was_active_hierarchy)
	{
		post_enable();
	}
	else if (!_active_hierarchy && was_active_hierarchy)
	{
		post_disable();
	}
}

void Entity::destroy()
{
	const peng::shared_ref<Entity> strong_this = peng::shared_ref(shared_from_this());
	PengEngine::get().entity_manager().destroy_entity(strong_this);
}

peng::weak_ptr<const Entity> Entity::weak_this() const
{
	return peng::shared_ref(shared_from_this());
}

peng::weak_ptr<Entity> Entity::weak_this()
{
	return peng::shared_ref(shared_from_this());
}

math::Matrix4x4f Entity::transform_matrix() const noexcept
{
	const math::Matrix4x4f local_matrix = _local_transform.to_matrix();

	if (_parent)
	{
		return _parent->transform_matrix() * local_matrix;
	}

	return local_matrix;
}

math::Matrix4x4f Entity::transform_matrix_inv() const noexcept
{
	const math::Matrix4x4f local_matrix_inv = _local_transform.to_inverse_matrix();

	if (_parent)
	{
		return local_matrix_inv * _parent->transform_matrix_inv();
	}

	return local_matrix_inv;
}

void Entity::propagate_active_change(bool parent_active)
{
	const bool new_active = parent_active && _active_self;
	const bool require_enable = new_active && !_active_hierarchy;
	const bool require_disable = !new_active && _active_hierarchy;

	for (const peng::weak_ptr<Entity>& child : _children)
	{
		if (child)
		{
			child->propagate_active_change(new_active);
		}
	}

	_active_hierarchy = new_active;
	if (require_enable)
	{
		post_enable();
	}
	else if (require_disable)
	{
		post_disable();
	}
}

void Entity::cleanup_killed_children()
{
	vectools::remove_all<peng::weak_ptr<Entity>>(_children, [](const peng::weak_ptr<Entity>& child) {
		return !child.valid();
	});
}
