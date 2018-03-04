#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/UIntVector4.hpp"
#include "Engine/Core/Rgba.hpp"

class Vertex3{
public:
	Vector3		m_position;
	Rgba		m_color;
	Vector2		m_texCoords;
	Vector3		m_normal;
	Vector3		m_tangent;
	Vector3		m_bitangent;
	Vector4		m_bone_weights;
	UIntVector4	m_bone_indices;

public:
	Vertex3(const Vector3& initialPosition = Vector3::ZERO, 
			const Rgba& initialColor = Rgba::WHITE, 
			const Vector2& initialTexCoords = Vector2::ZERO, 
			const Vector3& normal = Vector3::ZERO,
			const Vector3& tangent = Vector3::ZERO,
			const Vector3& bitangent = Vector3::ZERO,
			const Vector4& boneWeights = Vector4::ZERO,
			const UIntVector4& boneIndices = UIntVector4::ZERO);
};

inline Vertex3::Vertex3(const Vector3& initialPosition, 
						const Rgba& initialColor, 
						const Vector2& initialTexCoords, 
						const Vector3& normal, 
						const Vector3& tangent, 
						const Vector3& bitangent,
						const Vector4& boneWeights,
						const UIntVector4& boneIndices)
	:m_position(initialPosition)
	,m_color(initialColor)
	,m_texCoords(initialTexCoords)
	,m_normal(normal)
	,m_tangent(tangent)
	,m_bitangent(bitangent)
	,m_bone_weights(boneWeights)
	,m_bone_indices(boneIndices)
{
}

template<>
inline
bool BinaryStream::write(const Vertex3& vertex)
{
	return  write(vertex.m_position) && 
			write(vertex.m_color) &&
			write(vertex.m_texCoords) &&
			write(vertex.m_normal) &&
			write(vertex.m_tangent) &&
			write(vertex.m_bitangent) &&
			write(vertex.m_bone_weights) &&
			write(vertex.m_bone_indices);
}

template<>
inline
bool BinaryStream::read(Vertex3& vertex)
{
	return  read(vertex.m_position) && 
			read(vertex.m_color) &&
			read(vertex.m_texCoords) &&
			read(vertex.m_normal) &&
			read(vertex.m_tangent) &&
			read(vertex.m_bitangent) &&
			read(vertex.m_bone_weights) &&
			read(vertex.m_bone_indices);
}