#include "mesh_renderer.h"

#include <entities/camera.h>
#include <entities/point_light.h>
#include <core/logger.h>
#include <utils/utils.h>

using namespace entities;
using namespace components;
using namespace rendering;
using namespace math;

MeshRenderer::MeshRenderer(
	peng::shared_ref<const Mesh>&& mesh,
	peng::shared_ref<Material>&& material
)
	: Component(TickGroup::render)
	, _mesh(std::move(mesh))
	, _material(std::move(material))
{ }

MeshRenderer::MeshRenderer(
	const peng::shared_ref<const Mesh>& mesh,
	const peng::shared_ref<Material>& material
)
	: MeshRenderer(
		utils::copy(mesh),
		utils::copy(material)
	)
{ }

void MeshRenderer::tick(float delta_time)
{
	Component::tick(delta_time);

	if (_cached_model_matrix >= 0)
	{
		const Matrix4x4f model_matrix = owner().transform_matrix();
		_material->set_parameter(_cached_model_matrix, model_matrix);
	}

	if (_cached_view_matrix >= 0)
	{
		const Matrix4x4f view_matrix = Camera::current() ? Camera::current()->view_matrix() : Matrix4x4f::identity();
		_material->set_parameter(_cached_view_matrix, view_matrix);
	}

	if (_cached_light_pos >= 0)
	{
		const Vector3f light_pos = PointLight::current()
			? PointLight::current()->transform_matrix().get_translation()
			: Vector3f::zero();

		_material->set_parameter(_cached_light_pos, light_pos);
	}

	_material->use();
	_mesh->render();
}

void MeshRenderer::post_create()
{
	Component::post_create();

	_cached_model_matrix = _material->shader()->get_uniform_location("model_matrix");
	if (_cached_model_matrix < 0)
	{
		Logger::warning("Material has no 'model_matrix' parameter so rendering may be incorrect");
	}

	_cached_view_matrix = _material->shader()->get_uniform_location("view_matrix");
	if (_cached_view_matrix < 0)
	{
		Logger::warning("Material has no 'view_matrix' parameter so rendering may be incorrect");
	}

	// TODO: only get light pos if the shader has a SHADER_LIT symbol
	_cached_light_pos = _material->shader()->get_uniform_location("light_pos");
}
