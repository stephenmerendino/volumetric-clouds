#pragma once
#pragma warning(disable: 4201)

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

// NOTE---
// stored column major (basis layout)
// i.x j.x k.x t.x
// i.y j.y k.y t.y
// i.z j.z k.z t.z
// i.w j.w k.w t.w

// all matrix math is done row-major
// eg. object_to_world = scale * rotate * translate

// array indexes
// 0  1  2  3 
// 4  5  6  7 
// 8  9  10 11 
// 12 13 14 15 

class Matrix4
{
public:
	union
	{
		struct
		{
			Vector4 first_row;
			Vector4 second_row;
			Vector4 third_row;
			Vector4 fourth_row;
		};
		float data[16];
	};

public:
	Matrix4();
	explicit Matrix4(const float* array_of_floats);
	explicit Matrix4(const Vector3& i, const Vector3& j, const Vector3& k, const Vector3& t = Vector3(0.f, 0.f, 0.f));
	explicit Matrix4(const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& t = Vector4(0.f, 0.f, 0.f, 1.f));

	Matrix4(const Matrix4& copy);
	Matrix4& operator=(const Matrix4& other);

	void make_identity();
	void transpose();
	Matrix4 transposed() const;

	void inverse();
	Matrix4 get_inverse() const;
	
	void orthonormalize();

	Vector4 get_i_basis() const;
	Vector4 get_j_basis() const;
	Vector4 get_k_basis() const;
	Vector4 get_translation() const;

	void set_i_basis(const Vector3& i_basis, float homogenous_coordinate);
	void set_j_basis(const Vector3& j_basis, float homogenous_coordinate);
	void set_k_basis(const Vector3& k_basis, float homogenous_coordinate);
	void set_translation(const Vector3& translation, float homogenous_coordinate);

	void set_i_basis(const Vector4& i_basis);
	void set_j_basis(const Vector4& j_basis);
	void set_k_basis(const Vector4& k_basis);
	void set_translation(const Vector4& translation);

	Vector3 apply_rotation(const Vector3& vec3) const;
	Vector4 apply_rotation(const Vector4& vec4) const;

	Vector3 apply_translation(const Vector3& vec3) const;
	Vector4 apply_translation(const Vector4& vec4) const;

	Vector3 apply_transformation(const Vector3& vec3) const;
	Vector4 apply_transformation(const Vector4& vec4) const;

	void operator*=(const Matrix4& other);
	Matrix4 operator*(const Matrix4& other) const;

	void translate(float x, float y, float z);
	void translate(const Vector3& translation);

	void scale(const Vector3& scale_factors);
	void scale_uniform(float scale);
	void scale_x(float x_scale);
	void scale_y(float y_scale);
	void scale_z(float z_scale);
	void scale_translation(float trans_scale);

	void rotate_x_degrees(float x_degrees);
	void rotate_y_degrees(float y_degrees);
	void rotate_z_degrees(float z_degrees);

	void rotate_x_radians(float x_radians);
	void rotate_y_radians(float y_radians);
	void rotate_z_radians(float z_radians);

	static Matrix4 make_translation(float x, float y, float z);
	static Matrix4 make_translation(const Vector3& translation);

	static Matrix4 make_scale_uniform(float scale);
	static Matrix4 make_scale(const Vector3& scale_factors);
	static Matrix4 make_scale_x(float x_scale);
	static Matrix4 make_scale_y(float y_scale);
	static Matrix4 make_scale_z(float z_scale);

	static Matrix4 make_rotation_x_degrees(float x_degrees);
	static Matrix4 make_rotation_y_degrees(float y_degrees);
	static Matrix4 make_rotation_z_degrees(float z_degrees);
	static Matrix4 make_rotation_x_radians(float x_radians);
	static Matrix4 make_rotation_y_radians(float y_radians);
	static Matrix4 make_rotation_z_radians(float z_radians);

	friend Vector4 operator*(const Vector4& vec4, const Matrix4& transform);
	friend void operator*=(Vector4& vec4, const Matrix4& transform);
	friend Matrix4 nlerp(const Matrix4& a, const Matrix4& b, float t);

	static Matrix4 IDENTITY;
};

template<>
inline
bool BinaryStream::write(const Matrix4& v)
{
	return write(v.first_row) && write(v.second_row) && write(v.third_row) && write(v.fourth_row);
}

template<>
inline
bool BinaryStream::read(Matrix4& v)
{
	return read(v.first_row) && read(v.second_row) && read(v.third_row) && read(v.fourth_row);
}