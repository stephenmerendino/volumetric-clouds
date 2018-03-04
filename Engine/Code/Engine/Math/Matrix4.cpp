#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>
#include <memory.h>

Matrix4::Matrix4()
{
	memset(data, 0, sizeof(data));
	data[0]  = 1.f;
	data[5]  = 1.f;
	data[10] = 1.f;
	data[15] = 1.f;
}

Matrix4::Matrix4(const float* array_of_floats)
{
	memcpy(data, array_of_floats, sizeof(data));
}

Matrix4::Matrix4(const Vector3& i, const Vector3& j, const Vector3& k, const Vector3& t)
{
	data[0] = i.x;
	data[4] = i.y;
	data[8] = i.z;
	data[12] = 0.0f;

	data[1] = j.x;
	data[5] = j.y;
	data[9] = j.z;
	data[13] = 0.0f;

	data[2] = k.x;
	data[6] = k.y;
	data[10] = k.z;
	data[14] = 0.0f;

	data[3] = t.x;
	data[7] = t.y;
	data[11] = t.z;
	data[15] = 1.0f;
}

Matrix4::Matrix4(const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& t)
{
	data[0] = i.x;
	data[4] = i.y;
	data[8] = i.z;
	data[12] = i.w;

	data[1] = j.x;
	data[5] = j.y;
	data[9] = j.z;
	data[13] = j.w;

	data[2] = k.x;
	data[6] = k.y;
	data[10] = k.z;
	data[14] = k.w;

	data[3] = t.x;
	data[7] = t.y;
	data[11] = t.z;
	data[15] = t.w;
}

Matrix4::Matrix4(const Matrix4& copy)
{
	memcpy(data, copy.data, sizeof(data));
}

Matrix4& Matrix4::operator=(const Matrix4& other)
{
	memcpy(data, other.data, sizeof(data));
	return *this;
}

void Matrix4::make_identity()
{
	*this = IDENTITY;
}

void Matrix4::transpose()
{
	Swap(&data[1], &data[4]);
	Swap(&data[2], &data[8]);
	Swap(&data[3], &data[12]);
	Swap(&data[6], &data[9]);
	Swap(&data[7], &data[13]);
	Swap(&data[11], &data[14]);
}

Matrix4 Matrix4::transposed() const
{
	Matrix4 transposed = *this;
	transposed.transpose();
	return transposed;
}

// shamelessly lifted from https://github.com/jlyharia/Computer_GraphicsII/blob/master/gluInvertMatrix.h
void Matrix4::inverse()
{
	Matrix4 inverse;

	inverse.data[0] = data[5] * data[10] * data[15] -
		data[5] * data[11] * data[14] -
		data[9] * data[6] * data[15] +
		data[9] * data[7] * data[14] +
		data[13] * data[6] * data[11] -
		data[13] * data[7] * data[10];

	inverse.data[4] = -data[4] * data[10] * data[15] +
		data[4] * data[11] * data[14] +
		data[8] * data[6] * data[15] -
		data[8] * data[7] * data[14] -
		data[12] * data[6] * data[11] +
		data[12] * data[7] * data[10];

	inverse.data[8] = data[4] * data[9] * data[15] -
		data[4] * data[11] * data[13] -
		data[8] * data[5] * data[15] +
		data[8] * data[7] * data[13] +
		data[12] * data[5] * data[11] -
		data[12] * data[7] * data[9];

	inverse.data[12] = -data[4] * data[9] * data[14] +
		data[4] * data[10] * data[13] +
		data[8] * data[5] * data[14] -
		data[8] * data[6] * data[13] -
		data[12] * data[5] * data[10] +
		data[12] * data[6] * data[9];

	inverse.data[1] = -data[1] * data[10] * data[15] +
		data[1] * data[11] * data[14] +
		data[9] * data[2] * data[15] -
		data[9] * data[3] * data[14] -
		data[13] * data[2] * data[11] +
		data[13] * data[3] * data[10];

	inverse.data[5] = data[0] * data[10] * data[15] -
		data[0] * data[11] * data[14] -
		data[8] * data[2] * data[15] +
		data[8] * data[3] * data[14] +
		data[12] * data[2] * data[11] -
		data[12] * data[3] * data[10];

	inverse.data[9] = -data[0] * data[9] * data[15] +
		data[0] * data[11] * data[13] +
		data[8] * data[1] * data[15] -
		data[8] * data[3] * data[13] -
		data[12] * data[1] * data[11] +
		data[12] * data[3] * data[9];

	inverse.data[13] = data[0] * data[9] * data[14] -
		data[0] * data[10] * data[13] -
		data[8] * data[1] * data[14] +
		data[8] * data[2] * data[13] +
		data[12] * data[1] * data[10] -
		data[12] * data[2] * data[9];

	inverse.data[2] = data[1] * data[6] * data[15] -
		data[1] * data[7] * data[14] -
		data[5] * data[2] * data[15] +
		data[5] * data[3] * data[14] +
		data[13] * data[2] * data[7] -
		data[13] * data[3] * data[6];

	inverse.data[6] = -data[0] * data[6] * data[15] +
		data[0] * data[7] * data[14] +
		data[4] * data[2] * data[15] -
		data[4] * data[3] * data[14] -
		data[12] * data[2] * data[7] +
		data[12] * data[3] * data[6];

	inverse.data[10] = data[0] * data[5] * data[15] -
		data[0] * data[7] * data[13] -
		data[4] * data[1] * data[15] +
		data[4] * data[3] * data[13] +
		data[12] * data[1] * data[7] -
		data[12] * data[3] * data[5];

	inverse.data[14] = -data[0] * data[5] * data[14] +
		data[0] * data[6] * data[13] +
		data[4] * data[1] * data[14] -
		data[4] * data[2] * data[13] -
		data[12] * data[1] * data[6] +
		data[12] * data[2] * data[5];

	inverse.data[3] = -data[1] * data[6] * data[11] +
		data[1] * data[7] * data[10] +
		data[5] * data[2] * data[11] -
		data[5] * data[3] * data[10] -
		data[9] * data[2] * data[7] +
		data[9] * data[3] * data[6];

	inverse.data[7] = data[0] * data[6] * data[11] -
		data[0] * data[7] * data[10] -
		data[4] * data[2] * data[11] +
		data[4] * data[3] * data[10] +
		data[8] * data[2] * data[7] -
		data[8] * data[3] * data[6];

	inverse.data[11] = -data[0] * data[5] * data[11] +
		data[0] * data[7] * data[9] +
		data[4] * data[1] * data[11] -
		data[4] * data[3] * data[9] -
		data[8] * data[1] * data[7] +
		data[8] * data[3] * data[5];

	inverse.data[15] = data[0] * data[5] * data[10] -
		data[0] * data[6] * data[9] -
		data[4] * data[1] * data[10] +
		data[4] * data[2] * data[9] +
		data[8] * data[1] * data[6] -
		data[8] * data[2] * data[5];

	float det = data[0] * inverse.data[0] + data[1] * inverse.data[4] + data[2] * inverse.data[8] + data[3] * inverse.data[12];
	if(det == 0.0f){
		*this = Matrix4::IDENTITY;
		return;
	}

	det = 1.0f / det;

	for(int i = 0; i < 16; i++){
		inverse.data[i] *= det;
	}

	*this = inverse;
}

Matrix4 Matrix4::get_inverse() const
{
	Matrix4 copy(*this);
	copy.inverse();
	return copy;
}

void Matrix4::orthonormalize()
{
	Vector3 i_basis = get_i_basis().xyz;
	Vector3 j_basis = get_j_basis().xyz;
	Vector3 k_basis = get_k_basis().xyz;

	Vector3 new_i_basis = i_basis;
	Vector3 new_j_basis = j_basis - (DotProduct(j_basis, i_basis) * i_basis);
}

Vector4 Matrix4::get_i_basis() const
{
	return Vector4(data[0], data[4], data[8], data[12]);
}

Vector4 Matrix4::get_j_basis() const
{
	return Vector4(data[1], data[5], data[9], data[13]);
}

Vector4 Matrix4::get_k_basis() const
{
	return Vector4(data[2], data[6], data[10], data[14]);
}

Vector4 Matrix4::get_translation() const
{
	return Vector4(data[3], data[7], data[11], data[15]);
}

void Matrix4::set_i_basis(const Vector3& i_basis, float homogenous_coordinate)
{
	data[0] = i_basis.x;
	data[4] = i_basis.y;
	data[8] = i_basis.z;
	data[12] = homogenous_coordinate;
}

void Matrix4::set_j_basis(const Vector3& j_basis, float homogenous_coordinate)
{
	data[1] = j_basis.x;
	data[5] = j_basis.y;
	data[9] = j_basis.z;
	data[13] = homogenous_coordinate;
}

void Matrix4::set_k_basis(const Vector3& k_basis, float homogenous_coordinate)
{
	data[2] = k_basis.x;
	data[6] = k_basis.y;
	data[10] = k_basis.z;
	data[14] = homogenous_coordinate;
}

void Matrix4::set_translation(const Vector3& translation, float homogenous_coordinate)
{
	data[3] = translation.x;
	data[7] = translation.y;
	data[11] = translation.z;
	data[15] = homogenous_coordinate;
}

void Matrix4::set_i_basis(const Vector4& i_basis)
{
	data[0] = i_basis.x;
	data[4] = i_basis.y;
	data[8] = i_basis.z;
	data[12] = i_basis.w;
}

void Matrix4::set_j_basis(const Vector4& j_basis)
{
	data[1] = j_basis.x;
	data[5] = j_basis.y;
	data[9] = j_basis.z;
	data[13] = j_basis.w;
}

void Matrix4::set_k_basis(const Vector4& k_basis)
{
	data[2] = k_basis.x;
	data[6] = k_basis.y;
	data[10] = k_basis.z;
	data[14] = k_basis.w;
}

void Matrix4::set_translation(const Vector4& translation)
{
	data[3] = translation.x;
	data[7] = translation.y;
	data[11] = translation.z;
	data[15] = translation.w;
}

Vector3 Matrix4::apply_rotation(const Vector3& vec3) const
{
	Vector3 rotated_vec3;
	rotated_vec3.x = DotProduct(vec3, first_row.xyz);
	rotated_vec3.y = DotProduct(vec3, second_row.xyz);
	rotated_vec3.z = DotProduct(vec3, third_row.xyz);
	return rotated_vec3;
}

Vector4 Matrix4::apply_rotation(const Vector4& vec4) const
{
	Vector3 rotated_vec3;
	rotated_vec3.x = DotProduct(vec4.xyz, first_row.xyz);
	rotated_vec3.y = DotProduct(vec4.xyz, second_row.xyz);
	rotated_vec3.z = DotProduct(vec4.xyz, third_row.xyz);
	return Vector4(rotated_vec3, vec4.w);
}

Vector3 Matrix4::apply_translation(const Vector3& vec3) const
{
	Vector4 translation = get_translation();
	return vec3 + translation.xyz;
}

Vector4 Matrix4::apply_translation(const Vector4& vec4) const
{
	Vector4 translation = get_translation();
	return vec4 + translation;
}

Vector3 Matrix4::apply_transformation(const Vector3& vec3) const
{
	Vector4 transformed_vec = Vector4(vec3, 1.0f) * (*this);
	return transformed_vec.xyz;
}

Vector4 Matrix4::apply_transformation(const Vector4& vec4) const
{
	return vec4 * (*this);
}

// array indexes (but its really basis vectors vs slices of basic vectors)
// THIS(rows)		// OTHER (columns)
// 0  1  2  3       // 0  1  2  3 
// 4  5  6  7       // 4  5  6  7 
// 8  9  10 11      // 8  9  10 11 
// 12 13 14 15      // 12 13 14 15 
void Matrix4::operator*=(const Matrix4& other)
{
	// data_{row}{col}
	float i_x = (data[0] * other.data[0]) + (data[4] * other.data[1]) + (data[8] * other.data[2]) + (data[12] * other.data[3]);
	float i_y = (data[0] * other.data[4]) + (data[4] * other.data[5]) + (data[8] * other.data[6]) + (data[12] * other.data[7]);
	float i_z = (data[0] * other.data[8]) + (data[4] * other.data[9]) + (data[8] * other.data[10]) + (data[12] * other.data[11]);
	float i_w = (data[0] * other.data[12]) + (data[4] * other.data[13]) + (data[8] * other.data[14]) + (data[12] * other.data[15]);

	float j_x = (data[1] * other.data[0]) + (data[5] * other.data[1]) + (data[9] * other.data[2]) + (data[13] * other.data[3]);
	float j_y = (data[1] * other.data[4]) + (data[5] * other.data[5]) + (data[9] * other.data[6]) + (data[13] * other.data[7]);
	float j_z = (data[1] * other.data[8]) + (data[5] * other.data[9]) + (data[9] * other.data[10]) + (data[13] * other.data[11]);
	float j_w = (data[1] * other.data[12]) + (data[5] * other.data[13]) + (data[9] * other.data[14]) + (data[13] * other.data[15]);

	float k_x = (data[2] * other.data[0]) + (data[6] * other.data[1]) + (data[10] * other.data[2]) + (data[14] * other.data[3]);
	float k_y = (data[2] * other.data[4]) + (data[6] * other.data[5]) + (data[10] * other.data[6]) + (data[14] * other.data[7]);
	float k_z = (data[2] * other.data[8]) + (data[6] * other.data[9]) + (data[10] * other.data[10]) + (data[14] * other.data[11]);
	float k_w = (data[2] * other.data[12]) + (data[6] * other.data[13]) + (data[10] * other.data[14]) + (data[14] * other.data[15]);

	float t_x = (data[3] * other.data[0]) + (data[7] * other.data[1]) + (data[11] * other.data[2]) + (data[15] * other.data[3]);
	float t_y = (data[3] * other.data[4]) + (data[7] * other.data[5]) + (data[11] * other.data[6]) + (data[15] * other.data[7]);
	float t_z = (data[3] * other.data[8]) + (data[7] * other.data[9]) + (data[11] * other.data[10]) + (data[15] * other.data[11]);
	float t_w = (data[3] * other.data[12]) + (data[7] * other.data[13]) + (data[11] * other.data[14]) + (data[15] * other.data[15]);

	data[0] = i_x;
	data[4] = i_y;
	data[8] = i_z;
	data[12] = i_w;

	data[1] = j_x;
	data[5] = j_y;
	data[9] = j_z;
	data[13] = j_w;

	data[2] = k_x;
	data[6] = k_y;
	data[10] = k_z;
	data[14] = k_w;

	data[3] = t_x;
	data[7] = t_y;
	data[11] = t_z;
	data[15] = t_w;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const
{
	Matrix4 copy = *this;
	copy *= other;
	return copy;
}

void Matrix4::translate(float x, float y, float z)
{
	translate(Vector3(x, y, z));
}

void Matrix4::translate(const Vector3& translation)
{
	data[3] += translation.x;
	data[7] += translation.y;
	data[11] += translation.z;
}

void Matrix4::scale(const Vector3& scale_factors)
{
	data[0]  *= scale_factors.x;
	data[5]  *= scale_factors.y;
	data[10] *= scale_factors.z;
}

void Matrix4::scale_uniform(float scale)
{
	data[0]  *= scale;
	data[5]  *= scale;
	data[10] *= scale;
}

void Matrix4::scale_x(float x_scale)
{
	data[0] *= x_scale;
}

void Matrix4::scale_y(float y_scale)
{
	data[5] *= y_scale;
}

void Matrix4::scale_z(float z_scale)
{
	data[10] *= z_scale;
}

void Matrix4::scale_translation(float trans_scale)
{
	data[3] *= trans_scale;
	data[7] *= trans_scale;
	data[11] *= trans_scale;
}

void Matrix4::rotate_x_degrees(float x_degrees)
{
	float x_rot_rads = ConvertDegreesToRadians(x_degrees);
	rotate_x_radians(x_rot_rads);
}

void Matrix4::rotate_y_degrees(float y_degrees)
{
	float y_rot_rads = ConvertDegreesToRadians(y_degrees);
	rotate_y_radians(y_rot_rads);
}

void Matrix4::rotate_z_degrees(float z_degrees)
{
	float z_rot_rads = ConvertDegreesToRadians(z_degrees);
	rotate_z_radians(z_rot_rads);
}

// array indexes
// THIS(rows)		// x-rotation (columns)
// 0  1  2  3       // 1    0    0    0 
// 4  5  6  7       // 0    cos  -sin  0 
// 8  9  10 11      // 0   sin  cos  0 
// 12 13 14 15      // 0    0    0    1 
void Matrix4::rotate_x_radians(float x_radians)
{
	float cos_rads = cosf(x_radians);
	float sin_rads = sinf(x_radians);
	
	float i_y = (data[4] * cos_rads) + (data[8] * -sin_rads);
	float j_y = (data[5] * cos_rads) + (data[9] * -sin_rads);
	float k_y = (data[6] * cos_rads) + (data[10] * -sin_rads);
	float t_y = (data[7] * cos_rads) + (data[11] * -sin_rads);

	float i_z = (data[4] * sin_rads) + (data[8] * cos_rads);
	float j_z = (data[5] * sin_rads) + (data[9] * cos_rads);
	float k_z = (data[6] * sin_rads) + (data[10] * cos_rads);
	float t_z = (data[7] * sin_rads) + (data[11] * cos_rads);

	data[4] = i_y;
	data[5] = j_y;
	data[6] = k_y;
	data[7] = t_y;

	data[8] = i_z;
	data[9] = j_z;
	data[10] = k_z;
	data[11] = t_z;
}

// array indexes
// THIS(rows)		// y-rotation (columns)
// 0  1  2  3       //  cos  0   sin  0 
// 4  5  6  7       //  0	 1   0    0 
// 8  9  10 11      //  -sin  0  cos  0 
// 12 13 14 15      //  0    0   0    1 
void Matrix4::rotate_y_radians(float y_radians)
{
	float cos_rads = cosf(y_radians);
	float sin_rads = sinf(y_radians);
	
	float i_x = (data[0] * cos_rads) + (data[8] * sin_rads);
	float j_x = (data[1] * cos_rads) + (data[9] * sin_rads);
	float k_x = (data[2] * cos_rads) + (data[10] * sin_rads);
	float t_x = (data[3] * cos_rads) + (data[11] * sin_rads);

	float i_z = (data[0] * -sin_rads) + (data[8] * cos_rads);
	float j_z = (data[1] * -sin_rads) + (data[9] * cos_rads);
	float k_z = (data[2] * -sin_rads) + (data[10] * cos_rads);
	float t_z = (data[3] * -sin_rads) + (data[11] * cos_rads);

	data[0] = i_x;
	data[1] = j_x;
	data[2] = k_x;
	data[3] = t_x;

	data[8] = i_z;
	data[9] = j_z;
	data[10] = k_z;
	data[11] = t_z;
}

// array indexes
// THIS(rows)		// x-rotation (columns)
// 0  1  2  3       // cos  -sin  0    0 
// 4  5  6  7       // sin   cos  0    0 
// 8  9  10 11      // 0     0    1    0
// 12 13 14 15      // 0     0    0    1 
void Matrix4::rotate_z_radians(float z_radians)
{
	float cos_rads = cosf(z_radians);
	float sin_rads = sinf(z_radians);
	
	float i_x = (data[0] * cos_rads) + (data[4] * -sin_rads);
	float j_x = (data[1] * cos_rads) + (data[5] * -sin_rads);
	float k_x = (data[2] * cos_rads) + (data[6] * -sin_rads);
	float t_x = (data[3] * cos_rads) + (data[7] * -sin_rads);

	float i_y = (data[0] * sin_rads) + (data[4] * cos_rads);
	float j_y = (data[1] * sin_rads) + (data[5] * cos_rads);
	float k_y = (data[2] * sin_rads) + (data[6] * cos_rads);
	float t_y = (data[3] * sin_rads) + (data[7] * cos_rads);

	data[0] = i_x;
	data[1] = j_x;
	data[2] = k_x;
	data[3] = t_x;

	data[4] = i_y;
	data[5] = j_y;
	data[6] = k_y;
	data[7] = t_y;

}

Matrix4 Matrix4::make_translation(float x, float y, float z)
{
	return make_translation(Vector3(x, y, z));
}

Matrix4 Matrix4::make_translation(const Vector3& translation)
{
	Matrix4 mat;
	mat.data[3] = translation.x;
	mat.data[7] = translation.y;
	mat.data[11] = translation.z;
	return mat;
}

Matrix4 Matrix4::make_scale_uniform(float scale)
{
	Matrix4 mat;
	mat.scale_uniform(scale);
	return mat;
}

Matrix4 Matrix4::make_scale(const Vector3& scale_factors)
{
	Matrix4 mat;
	mat.scale(scale_factors);
	return mat;
}

Matrix4 Matrix4::make_scale_x(float x_scale)
{
	Matrix4 mat;
	mat.scale_x(x_scale);
	return mat;
}

Matrix4 Matrix4::make_scale_y(float y_scale)
{
	Matrix4 mat;
	mat.scale_y(y_scale);
	return mat;
}

Matrix4 Matrix4::make_scale_z(float z_scale)
{
	Matrix4 mat;
	mat.scale_z(z_scale);
	return mat;
}

Matrix4 Matrix4::make_rotation_x_degrees(float x_degrees)
{
	Matrix4 mat;
	mat.rotate_x_degrees(x_degrees);
	return mat;
}

Matrix4 Matrix4::make_rotation_y_degrees(float y_degrees)
{
	Matrix4 mat;
	mat.rotate_y_degrees(y_degrees);
	return mat;
}

Matrix4 Matrix4::make_rotation_z_degrees(float z_degrees)
{
	Matrix4 mat;
	mat.rotate_z_degrees(z_degrees);
	return mat;
}

Matrix4 Matrix4::make_rotation_x_radians(float x_radians)
{
	Matrix4 mat;
	mat.rotate_z_radians(x_radians);
	return mat;
}

Matrix4 Matrix4::make_rotation_y_radians(float y_radians)
{
	Matrix4 mat;
	mat.rotate_y_radians(y_radians);
	return mat;
}

Matrix4 Matrix4::make_rotation_z_radians(float z_radians)
{
	Matrix4 mat;
	mat.rotate_z_radians(z_radians);
	return mat;
}

Vector4 operator*(const Vector4& vec4, const Matrix4& transform)
{
	Vector4 copy(vec4);
	copy *= transform;
	return copy;
}

void operator*=(Vector4& vec4, const Matrix4& transform)
{
	float new_x = DotProduct(vec4, transform.first_row);
	float new_y = DotProduct(vec4, transform.second_row);
	float new_z = DotProduct(vec4, transform.third_row);
	float new_w = DotProduct(vec4, transform.fourth_row);
	
	vec4.x = new_x;
	vec4.y = new_y;
	vec4.z = new_z;
	vec4.w = new_w;
}

Matrix4 nlerp(const Matrix4& a, const Matrix4& b, float t)
{
	//slerp the i,j,k basis vectors
	Vector3 a_i_basis = a.get_i_basis().xyz;
	Vector3 a_j_basis = a.get_j_basis().xyz;
	Vector3 a_k_basis = a.get_k_basis().xyz;

	Vector3 b_i_basis = b.get_i_basis().xyz;
	Vector3 b_j_basis = b.get_j_basis().xyz;
	Vector3 b_k_basis = b.get_k_basis().xyz;

	Vector3 a_trans = a.get_translation().xyz;
	Vector3 b_trans = b.get_translation().xyz;

	Vector3 i_slerped = slerp(a_i_basis, b_i_basis, t);
	Vector3 j_slerped = slerp(a_j_basis, b_j_basis, t);
	Vector3 k_slerped = slerp(a_k_basis, b_k_basis, t);

	//lerp the translation
	Vector3 trans_lerped = lerp(a_trans, b_trans, t);

	//renormalize
	i_slerped.Normalize();
	j_slerped.Normalize();
	k_slerped.Normalize();

	return Matrix4(i_slerped, j_slerped, k_slerped, trans_lerped);
}

Matrix4 Matrix4::IDENTITY = Matrix4();