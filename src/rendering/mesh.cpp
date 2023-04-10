#include "mesh.h"

#include <utils/utils.h>
#include <utils/strtools.h>
#include <utils/vectools.h>
#include <core/logger.h>
#include <profiling/scoped_event.h>

using namespace rendering;
using namespace math;

Mesh::Mesh(
    std::string&& name,
    std::vector<Vertex>&& vertices,
    std::vector<Vector3u>&& indices
)
    : _name(std::move(name))
    , _vertex_buffer(std::move(vertices))
    , _index_buffer(std::move(indices))
    , _num_indices(static_cast<GLuint>(_index_buffer.size() * 3))
{
    SCOPED_EVENT("Building mesh", _name.c_str());
    Logger::log("Building mesh '%s'", _name.c_str());

    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);
    glGenVertexArrays(1, &_vao);

    glBindVertexArray(_vao);
    glObjectLabel(GL_VERTEX_ARRAY, _vao, -1, _name.c_str());

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vectools::buffer_size(_vertex_buffer), _vertex_buffer.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vectools::buffer_size(_index_buffer), _index_buffer.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);
}

Mesh::Mesh(
    const std::string& name,
    const std::vector<Vertex>& vertices,
    const std::vector<Vector3u>& indices
)
    : Mesh(
        utils::copy(name),
        utils::copy(vertices),
        utils::copy(indices)
    )
{ }

Mesh::~Mesh()
{
    SCOPED_EVENT("Destroying mesh", _name.c_str());
    Logger::log("Destroying mesh '%s'", _name.c_str());

    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
}

void Mesh::render() const
{
    bind();
    draw();
    unbind();
}

void Mesh::bind() const
{
    glBindVertexArray(_vao);
}

void Mesh::unbind() const
{
    glBindVertexArray(0);
}

void Mesh::draw() const
{
    glDrawElements(GL_TRIANGLES, _num_indices, GL_UNSIGNED_INT, nullptr);
}

const std::string& Mesh::name() const noexcept
{
    return _name;
}

int32_t Mesh::num_triangles() const noexcept
{
    return _index_buffer.size();
}

