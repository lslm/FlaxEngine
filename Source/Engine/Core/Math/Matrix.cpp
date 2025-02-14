// Copyright (c) 2012-2022 Wojciech Figat. All rights reserved.

#include "Matrix.h"
#include "Vector2.h"
#include "Quaternion.h"
#include "Transform.h"
#include "../Types/String.h"

static_assert(sizeof(Matrix) == 4 * 4 * 4, "Invalid Matrix type size.");

const Matrix Matrix::Zero(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const Matrix Matrix::Identity(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

String Matrix::ToString() const
{
    return String::Format(TEXT("{}"), *this);
}

float Matrix::GetDeterminant() const
{
    const float temp1 = M33 * M44 - M34 * M43;
    const float temp2 = M32 * M44 - M34 * M42;
    const float temp3 = M32 * M43 - M33 * M42;
    const float temp4 = M31 * M44 - M34 * M41;
    const float temp5 = M31 * M43 - M33 * M41;
    const float temp6 = M31 * M42 - M32 * M41;
    return M11 * (M22 * temp1 - M23 * temp2 + M24 * temp3) - M12 * (M21 * temp1 -
                M23 * temp4 + M24 * temp5) + M13 * (M21 * temp2 - M22 * temp4 + M24 * temp6) -
            M14 * (M21 * temp3 - M22 * temp5 + M23 * temp6);
}

float Matrix::RotDeterminant() const
{
    return
            Values[0][0] * (Values[1][1] * Values[2][2] - Values[1][2] * Values[2][1]) -
            Values[1][0] * (Values[0][1] * Values[2][2] - Values[0][2] * Values[2][1]) +
            Values[2][0] * (Values[0][1] * Values[1][2] - Values[0][2] * Values[1][1]);
}

void Matrix::NormalizeScale()
{
    const float scaleX = 1.0f / Vector3(M11, M21, M31).Length();
    const float scaleY = 1.0f / Vector3(M12, M22, M32).Length();
    const float scaleZ = 1.0f / Vector3(M13, M23, M33).Length();

    M11 *= scaleX;
    M21 *= scaleX;
    M31 *= scaleX;

    M12 *= scaleY;
    M22 *= scaleY;
    M32 *= scaleY;

    M13 *= scaleZ;
    M23 *= scaleZ;
    M33 *= scaleZ;
}

void Matrix::Decompose(float& yaw, float& pitch, float& roll) const
{
    pitch = Math::Asin(-M32);
    if (Math::Cos(pitch) > 1e-12f)
    {
        roll = Math::Atan2(M12, M22);
        yaw = Math::Atan2(M31, M33);
    }
    else
    {
        roll = Math::Atan2(-M21, M11);
        yaw = 0.0f;
    }
}

void Matrix::Decompose(Vector3& scale, Vector3& translation) const
{
    // Get the translation
    translation = Vector3(M41, M42, M43);

    // Scaling is the length of the rows
    scale = Vector3(
        Math::Sqrt(M11 * M11 + M12 * M12 + M13 * M13),
        Math::Sqrt(M21 * M21 + M22 * M22 + M23 * M23),
        Math::Sqrt(M31 * M31 + M32 * M32 + M33 * M33));
}

void Matrix::Decompose(Transform& transform) const
{
    Matrix rotationMatrix;
    Decompose(transform.Scale, rotationMatrix, transform.Translation);
    Quaternion::RotationMatrix(rotationMatrix, transform.Orientation);
}

void Matrix::Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const
{
    Matrix rotationMatrix;
    Decompose(scale, rotationMatrix, translation);
    Quaternion::RotationMatrix(rotationMatrix, rotation);
}

void Matrix::Decompose(Vector3& scale, Matrix& rotation, Vector3& translation) const
{
    // Get the translation
    translation = Vector3(M41, M42, M43);

    // Scaling is the length of the rows
    scale = Vector3(
        Math::Sqrt(M11 * M11 + M12 * M12 + M13 * M13),
        Math::Sqrt(M21 * M21 + M22 * M22 + M23 * M23),
        Math::Sqrt(M31 * M31 + M32 * M32 + M33 * M33));

    // If any of the scaling factors are zero, than the rotation matrix can not exist
    rotation = Identity;
    if (scale.IsAnyZero())
        return;

    // Calculate an perfect orthonormal matrix (no reflections)
    const auto at = Vector3(M31 / scale.Z, M32 / scale.Z, M33 / scale.Z);
    const auto up = Vector3::Cross(at, Vector3(M11 / scale.X, M12 / scale.X, M13 / scale.X));
    const auto right = Vector3::Cross(up, at);
    rotation.SetRight(right);
    rotation.SetUp(up);
    rotation.SetBackward(at);

    // In case of reflexions
    scale.X = Vector3::Dot(right, GetRight()) > 0.0f ? scale.X : -scale.X;
    scale.Y = Vector3::Dot(up, GetUp()) > 0.0f ? scale.Y : -scale.Y;
    scale.Z = Vector3::Dot(at, GetBackward()) > 0.0f ? scale.Z : -scale.Z;
}

Matrix Matrix::Transpose(const Matrix& value)
{
    Matrix result;
    result.M11 = value.M11;
    result.M12 = value.M21;
    result.M13 = value.M31;
    result.M14 = value.M41;
    result.M21 = value.M12;
    result.M22 = value.M22;
    result.M23 = value.M32;
    result.M24 = value.M42;
    result.M31 = value.M13;
    result.M32 = value.M23;
    result.M33 = value.M33;
    result.M34 = value.M43;
    result.M41 = value.M14;
    result.M42 = value.M24;
    result.M43 = value.M34;
    result.M44 = value.M44;
    return result;
}

void Matrix::Transpose(const Matrix& value, Matrix& result)
{
    Matrix temp;
    temp.M11 = value.M11;
    temp.M12 = value.M21;
    temp.M13 = value.M31;
    temp.M14 = value.M41;
    temp.M21 = value.M12;
    temp.M22 = value.M22;
    temp.M23 = value.M32;
    temp.M24 = value.M42;
    temp.M31 = value.M13;
    temp.M32 = value.M23;
    temp.M33 = value.M33;
    temp.M34 = value.M43;
    temp.M41 = value.M14;
    temp.M42 = value.M24;
    temp.M43 = value.M34;
    temp.M44 = value.M44;
    result = temp;
}

void Matrix::Invert(const Matrix& value, Matrix& result)
{
    const float b0 = value.M31 * value.M42 - value.M32 * value.M41;
    const float b1 = value.M31 * value.M43 - value.M33 * value.M41;
    const float b2 = value.M34 * value.M41 - value.M31 * value.M44;
    const float b3 = value.M32 * value.M43 - value.M33 * value.M42;
    const float b4 = value.M34 * value.M42 - value.M32 * value.M44;
    const float b5 = value.M33 * value.M44 - value.M34 * value.M43;

    const float d11 = value.M22 * b5 + value.M23 * b4 + value.M24 * b3;
    const float d12 = value.M21 * b5 + value.M23 * b2 + value.M24 * b1;
    const float d13 = value.M21 * -b4 + value.M22 * b2 + value.M24 * b0;
    const float d14 = value.M21 * b3 + value.M22 * -b1 + value.M23 * b0;

    float det = value.M11 * d11 - value.M12 * d12 + value.M13 * d13 - value.M14 * d14;
    if (Math::Abs(det) <= 1e-12f)
    {
        result = Zero;
        return;
    }

    det = 1.0f / det;

    const float a0 = value.M11 * value.M22 - value.M12 * value.M21;
    const float a1 = value.M11 * value.M23 - value.M13 * value.M21;
    const float a2 = value.M14 * value.M21 - value.M11 * value.M24;
    const float a3 = value.M12 * value.M23 - value.M13 * value.M22;
    const float a4 = value.M14 * value.M22 - value.M12 * value.M24;
    const float a5 = value.M13 * value.M24 - value.M14 * value.M23;

    const float d21 = value.M12 * b5 + value.M13 * b4 + value.M14 * b3;
    const float d22 = value.M11 * b5 + value.M13 * b2 + value.M14 * b1;
    const float d23 = value.M11 * -b4 + value.M12 * b2 + value.M14 * b0;
    const float d24 = value.M11 * b3 + value.M12 * -b1 + value.M13 * b0;

    const float d31 = value.M42 * a5 + value.M43 * a4 + value.M44 * a3;
    const float d32 = value.M41 * a5 + value.M43 * a2 + value.M44 * a1;
    const float d33 = value.M41 * -a4 + value.M42 * a2 + value.M44 * a0;
    const float d34 = value.M41 * a3 + value.M42 * -a1 + value.M43 * a0;

    const float d41 = value.M32 * a5 + value.M33 * a4 + value.M34 * a3;
    const float d42 = value.M31 * a5 + value.M33 * a2 + value.M34 * a1;
    const float d43 = value.M31 * -a4 + value.M32 * a2 + value.M34 * a0;
    const float d44 = value.M31 * a3 + value.M32 * -a1 + value.M33 * a0;

    result.M11 = +d11 * det;
    result.M12 = -d21 * det;
    result.M13 = +d31 * det;
    result.M14 = -d41 * det;
    result.M21 = -d12 * det;
    result.M22 = +d22 * det;
    result.M23 = -d32 * det;
    result.M24 = +d42 * det;
    result.M31 = +d13 * det;
    result.M32 = -d23 * det;
    result.M33 = +d33 * det;
    result.M34 = -d43 * det;
    result.M41 = -d14 * det;
    result.M42 = +d24 * det;
    result.M43 = -d34 * det;
    result.M44 = +d44 * det;
}

void Matrix::Billboard(const Vector3& objectPosition, const Vector3& cameraPosition, const Vector3& cameraUpVector, const Vector3& cameraForwardVector, Matrix& result)
{
    Vector3 crossed;
    Vector3 final;
    Vector3 difference = cameraPosition - objectPosition;

    const float lengthSq = difference.LengthSquared();
    if (Math::IsZero(lengthSq))
        difference = -cameraForwardVector;
    else
        difference *= 1.0f / Math::Sqrt(lengthSq);

    Vector3::Cross(cameraUpVector, difference, crossed);
    crossed.Normalize();
    Vector3::Cross(difference, crossed, final);

    result.M11 = crossed.X;
    result.M12 = crossed.Y;
    result.M13 = crossed.Z;
    result.M14 = 0.0f;

    result.M21 = final.X;
    result.M22 = final.Y;
    result.M23 = final.Z;
    result.M24 = 0.0f;

    result.M31 = difference.X;
    result.M32 = difference.Y;
    result.M33 = difference.Z;
    result.M34 = 0.0f;

    result.M41 = objectPosition.X;
    result.M42 = objectPosition.Y;
    result.M43 = objectPosition.Z;
    result.M44 = 1.0f;
}

void Matrix::LookAt(const Vector3& eye, const Vector3& target, const Vector3& up, Matrix& result)
{
    Vector3 xaxis, yaxis, zaxis;
    Vector3::Subtract(target, eye, zaxis);
    zaxis.Normalize();
    Vector3::Cross(up, zaxis, xaxis);
    xaxis.Normalize();
    Vector3::Cross(zaxis, xaxis, yaxis);

    result = Identity;

    result.M11 = xaxis.X;
    result.M21 = xaxis.Y;
    result.M31 = xaxis.Z;

    result.M12 = yaxis.X;
    result.M22 = yaxis.Y;
    result.M32 = yaxis.Z;

    result.M13 = zaxis.X;
    result.M23 = zaxis.Y;
    result.M33 = zaxis.Z;

    result.M41 = -Vector3::Dot(xaxis, eye);
    result.M42 = -Vector3::Dot(yaxis, eye);
    result.M43 = -Vector3::Dot(zaxis, eye);
}

void Matrix::OrthoOffCenter(float left, float right, float bottom, float top, float zNear, float zFar, Matrix& result)
{
    const float zRange = 1.0f / (zFar - zNear);

    result = Identity;
    result.M11 = 2.0f / (right - left);
    result.M22 = 2.0f / (top - bottom);
    result.M33 = zRange;
    result.M41 = (left + right) / (left - right);
    result.M42 = (top + bottom) / (bottom - top);
    result.M43 = -zNear * zRange;
}

void Matrix::PerspectiveFov(float fov, float aspect, float zNear, float zFar, Matrix& result)
{
    const float yScale = 1.0f / Math::Tan(fov * 0.5f);
    const float xScale = yScale / aspect;

    const float halfWidth = zNear / xScale;
    const float halfHeight = zNear / yScale;

    PerspectiveOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar, result);
}

void Matrix::PerspectiveOffCenter(float left, float right, float bottom, float top, float zNear, float zFar, Matrix& result)
{
    const float zRange = zFar / (zFar - zNear);

    result = Zero;
    result.M11 = 2.0f * zNear / (right - left);
    result.M22 = 2.0f * zNear / (top - bottom);
    result.M31 = (left + right) / (left - right);
    result.M32 = (top + bottom) / (bottom - top);
    result.M33 = zRange;
    result.M34 = 1.0f;
    result.M43 = -zNear * zRange;
}

void Matrix::RotationX(float angle, Matrix& result)
{
    const float cosA = Math::Cos(angle);
    const float sinA = Math::Sin(angle);
    result = Identity;
    result.M22 = cosA;
    result.M23 = sinA;
    result.M32 = -sinA;
    result.M33 = cosA;
}

void Matrix::RotationY(float angle, Matrix& result)
{
    const float cosA = Math::Cos(angle);
    const float sinA = Math::Sin(angle);
    result = Identity;
    result.M11 = cosA;
    result.M13 = -sinA;
    result.M31 = sinA;
    result.M33 = cosA;
}

void Matrix::RotationZ(float angle, Matrix& result)
{
    const float cosA = Math::Cos(angle);
    const float sinA = Math::Sin(angle);
    result = Identity;
    result.M11 = cosA;
    result.M12 = sinA;
    result.M21 = -sinA;
    result.M22 = cosA;
}

void Matrix::RotationAxis(const Vector3& axis, float angle, Matrix& result)
{
    const float x = axis.X;
    const float y = axis.Y;
    const float z = axis.Z;
    const float cosA = Math::Cos(angle);
    const float sinA = Math::Sin(angle);
    const float xx = x * x;
    const float yy = y * y;
    const float zz = z * z;
    const float xy = x * y;
    const float xz = x * z;
    const float yz = y * z;

    result = Identity;
    result.M11 = xx + cosA * (1.0f - xx);
    result.M12 = xy - cosA * xy + sinA * z;
    result.M13 = xz - cosA * xz - sinA * y;
    result.M21 = xy - cosA * xy - sinA * z;
    result.M22 = yy + cosA * (1.0f - yy);
    result.M23 = yz - cosA * yz + sinA * x;
    result.M31 = xz - cosA * xz + sinA * y;
    result.M32 = yz - cosA * yz - sinA * x;
    result.M33 = zz + cosA * (1.0f - zz);
}

void Matrix::RotationQuaternion(const Quaternion& rotation, Matrix& result)
{
    const float xx = rotation.X * rotation.X;
    const float yy = rotation.Y * rotation.Y;
    const float zz = rotation.Z * rotation.Z;
    const float xy = rotation.X * rotation.Y;
    const float zw = rotation.Z * rotation.W;
    const float zx = rotation.Z * rotation.X;
    const float yw = rotation.Y * rotation.W;
    const float yz = rotation.Y * rotation.Z;
    const float xw = rotation.X * rotation.W;

    result.M11 = 1.0f - 2.0f * (yy + zz);
    result.M12 = 2.0f * (xy + zw);
    result.M13 = 2.0f * (zx - yw);
    result.M14 = 0;

    result.M21 = 2.0f * (xy - zw);
    result.M22 = 1.0f - 2.0f * (zz + xx);
    result.M23 = 2.0f * (yz + xw);
    result.M24 = 0;

    result.M31 = 2.0f * (zx + yw);
    result.M32 = 2.0f * (yz - xw);
    result.M33 = 1.0f - 2.0f * (yy + xx);
    result.M34 = 0;

    result.M41 = 0;
    result.M42 = 0;
    result.M43 = 0;
    result.M44 = 1;
}

void Matrix::RotationYawPitchRoll(float yaw, float pitch, float roll, Matrix& result)
{
    Quaternion quaternion;
    Quaternion::RotationYawPitchRoll(yaw, pitch, roll, quaternion);
    RotationQuaternion(quaternion, result);
}

Matrix Matrix::Translation(const Vector3& value)
{
    Matrix result = Identity;
    result.M41 = value.X;
    result.M42 = value.Y;
    result.M43 = value.Z;
    return result;
}

void Matrix::Translation(const Vector3& value, Matrix& result)
{
    result = Identity;
    result.M41 = value.X;
    result.M42 = value.Y;
    result.M43 = value.Z;
}

void Matrix::Translation(float x, float y, float z, Matrix& result)
{
    result = Identity;
    result.M41 = x;
    result.M42 = y;
    result.M43 = z;
}

void Matrix::Skew(float angle, const Vector3& rotationVec, const Vector3& transVec, Matrix& matrix)
{
    // http://elckerlyc.ewi.utwente.nl/browser/Elckerlyc/Hmi/HmiMath/src/hmi/math/Mat3f.java
    const float MINIMAL_SKEW_ANGLE = 0.000001f;

    Vector3 e0 = rotationVec;
    Vector3 e1;
    Vector3::Normalize(transVec, e1);

    const float rv1 = Vector3::Dot(rotationVec, e1);
    e0 += rv1 * e1;
    const float rv0 = Vector3::Dot(rotationVec, e0);
    const float cosa = Math::Cos(angle);
    const float sina = Math::Sin(angle);
    const float rr0 = rv0 * cosa - rv1 * sina;
    const float rr1 = rv0 * sina + rv1 * cosa;

    ASSERT(rr0 >= MINIMAL_SKEW_ANGLE);

    const float d = rr1 / rr0 - rv1 / rv0;

    matrix = Identity;
    matrix.M11 = d * e1.X * e0.X + 1.0f;
    matrix.M12 = d * e1.X * e0.Y;
    matrix.M13 = d * e1.X * e0.Z;
    matrix.M21 = d * e1.Y * e0.X;
    matrix.M22 = d * e1.Y * e0.Y + 1.0f;
    matrix.M23 = d * e1.Y * e0.Z;
    matrix.M31 = d * e1.Z * e0.X;
    matrix.M32 = d * e1.Z * e0.Y;
    matrix.M33 = d * e1.Z * e0.Z + 1.0f;
}

void Matrix::Transformation(const Vector3& scaling, const Quaternion& rotation, const Vector3& translation, Matrix& result)
{
    // Equivalent to:
    //result =
    //    Matrix.Scaling(scaling)
    //    *Matrix.RotationX(rotation.X)
    //    *Matrix.RotationY(rotation.Y)
    //    *Matrix.RotationZ(rotation.Z)
    //    *Matrix.Position(translation);

    // Rotation
    const float xx = rotation.X * rotation.X;
    const float yy = rotation.Y * rotation.Y;
    const float zz = rotation.Z * rotation.Z;
    const float xy = rotation.X * rotation.Y;
    const float zw = rotation.Z * rotation.W;
    const float zx = rotation.Z * rotation.X;
    const float yw = rotation.Y * rotation.W;
    const float yz = rotation.Y * rotation.Z;
    const float xw = rotation.X * rotation.W;
    result.M11 = 1.0f - 2.0f * (yy + zz);
    result.M12 = 2.0f * (xy + zw);
    result.M13 = 2.0f * (zx - yw);
    result.M21 = 2.0f * (xy - zw);
    result.M22 = 1.0f - 2.0f * (zz + xx);
    result.M23 = 2.0f * (yz + xw);
    result.M31 = 2.0f * (zx + yw);
    result.M32 = 2.0f * (yz - xw);
    result.M33 = 1.0f - 2.0f * (yy + xx);

    // Position
    result.M41 = translation.X;
    result.M42 = translation.Y;
    result.M43 = translation.Z;

    // Scale
    result.M11 *= scaling.X;
    result.M12 *= scaling.X;
    result.M13 *= scaling.X;
    result.M21 *= scaling.Y;
    result.M22 *= scaling.Y;
    result.M23 *= scaling.Y;
    result.M31 *= scaling.Z;
    result.M32 *= scaling.Z;
    result.M33 *= scaling.Z;

    result.M14 = 0.0f;
    result.M24 = 0.0f;
    result.M34 = 0.0f;
    result.M44 = 1.0f;
}

void Matrix::AffineTransformation(float scaling, const Quaternion& rotation, const Vector3& translation, Matrix& result)
{
    result = Scaling(scaling) * RotationQuaternion(rotation) * Translation(translation);
}

void Matrix::AffineTransformation(float scaling, const Vector3& rotationCenter, const Quaternion& rotation, const Vector3& translation, Matrix& result)
{
    result = Scaling(scaling) * Translation(-rotationCenter) * RotationQuaternion(rotation) * Translation(rotationCenter) * Translation(translation);
}

void Matrix::AffineTransformation2D(float scaling, float rotation, const Vector2& translation, Matrix& result)
{
    result = Scaling(scaling, scaling, 1.0f) * RotationZ(rotation) * Translation((Vector3)translation);
}

void Matrix::AffineTransformation2D(float scaling, const Vector2& rotationCenter, float rotation, const Vector2& translation, Matrix& result)
{
    result = Scaling(scaling, scaling, 1.0f) * Translation((Vector3)-rotationCenter) * RotationZ(rotation) * Translation((Vector3)rotationCenter) * Translation((Vector3)translation);
}

void Matrix::Transformation(const Vector3& scalingCenter, const Quaternion& scalingRotation, const Vector3& scaling, const Vector3& rotationCenter, const Quaternion& rotation, const Vector3& translation, Matrix& result)
{
    Matrix sr;
    RotationQuaternion(scalingRotation, sr);
    result = Translation(-scalingCenter) * Transpose(sr) * Scaling(scaling) * sr * Translation(scalingCenter) * Translation(-rotationCenter) * RotationQuaternion(rotation) * Translation(rotationCenter) * Translation(translation);
}

void Matrix::Transformation2D(Vector2& scalingCenter, float scalingRotation, const Vector2& scaling, const Vector2& rotationCenter, float rotation, const Vector2& translation, Matrix& result)
{
    result = Translation((Vector3)-scalingCenter) * RotationZ(-scalingRotation) * Scaling((Vector3)scaling) * RotationZ(scalingRotation) * Translation((Vector3)scalingCenter) * Translation((Vector3)-rotationCenter) * RotationZ(rotation) * Translation((Vector3)rotationCenter) * Translation((Vector3)translation);
    result.M33 = 1.0f;
    result.M44 = 1.0f;
}

Matrix Matrix::CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up)
{
    Matrix result;
    Vector3 vector3, vector31, vector32;

    Vector3::Normalize(forward, vector3);
    vector3.Negate();
    Vector3::Normalize(Vector3::Cross(up, vector3), vector31);
    Vector3::Cross(vector3, vector31, vector32);

    result.M11 = vector31.X;
    result.M12 = vector31.Y;
    result.M13 = vector31.Z;
    result.M14 = 0.0f;

    result.M21 = vector32.X;
    result.M22 = vector32.Y;
    result.M23 = vector32.Z;
    result.M24 = 0.0f;

    result.M31 = vector3.X;
    result.M32 = vector3.Y;
    result.M33 = vector3.Z;
    result.M34 = 0.0f;

    result.M41 = position.X;
    result.M42 = position.Y;
    result.M43 = position.Z;
    result.M44 = 1.0f;

    return result;
}

void Matrix::CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up, Matrix& result)
{
    Vector3 vector3, vector31, vector32;

    Vector3::Normalize(forward, vector3);
    vector3.Negate();
    Vector3::Normalize(Vector3::Cross(up, vector3), vector31);
    Vector3::Cross(vector3, vector31, vector32);

    result.M11 = vector31.X;
    result.M12 = vector31.Y;
    result.M13 = vector31.Z;
    result.M14 = 0.0f;

    result.M21 = vector32.X;
    result.M22 = vector32.Y;
    result.M23 = vector32.Z;
    result.M24 = 0.0f;

    result.M31 = vector3.X;
    result.M32 = vector3.Y;
    result.M33 = vector3.Z;
    result.M34 = 0.0f;

    result.M41 = position.X;
    result.M42 = position.Y;
    result.M43 = position.Z;
    result.M44 = 1.0f;
}

Matrix Matrix::CreateFromAxisAngle(const Vector3& axis, float angle)
{
    Matrix matrix;

    const float x = axis.X;
    const float y = axis.Y;
    const float z = axis.Z;
    const float single = Math::Sin(angle);
    const float single1 = Math::Cos(angle);
    const float single2 = x * x;
    const float single3 = y * y;
    const float single4 = z * z;
    const float single5 = x * y;
    const float single6 = x * z;
    const float single7 = y * z;

    matrix.M11 = single2 + single1 * (1.0f - single2);
    matrix.M12 = single5 - single1 * single5 + single * z;
    matrix.M13 = single6 - single1 * single6 - single * y;
    matrix.M14 = 0.0f;

    matrix.M21 = single5 - single1 * single5 - single * z;
    matrix.M22 = single3 + single1 * (1.0f - single3);
    matrix.M23 = single7 - single1 * single7 + single * x;
    matrix.M24 = 0.0f;

    matrix.M31 = single6 - single1 * single6 + single * y;
    matrix.M32 = single7 - single1 * single7 - single * x;
    matrix.M33 = single4 + single1 * (1.0f - single4);
    matrix.M34 = 0.0f;

    matrix.M41 = 0.0f;
    matrix.M42 = 0.0f;
    matrix.M43 = 0.0f;
    matrix.M44 = 1.0f;

    return matrix;
}

void Matrix::CreateFromAxisAngle(const Vector3& axis, float angle, Matrix& result)
{
    const float x = axis.X;
    const float y = axis.Y;
    const float z = axis.Z;
    const float single = Math::Sin(angle);
    const float single1 = Math::Cos(angle);
    const float single2 = x * x;
    const float single3 = y * y;
    const float single4 = z * z;
    const float single5 = x * y;
    const float single6 = x * z;
    const float single7 = y * z;

    result.M11 = single2 + single1 * (1.0f - single2);
    result.M12 = single5 - single1 * single5 + single * z;
    result.M13 = single6 - single1 * single6 - single * y;
    result.M14 = 0.0f;

    result.M21 = single5 - single1 * single5 - single * z;
    result.M22 = single3 + single1 * (1.0f - single3);
    result.M23 = single7 - single1 * single7 + single * x;
    result.M24 = 0.0f;

    result.M31 = single6 - single1 * single6 + single * y;
    result.M32 = single7 - single1 * single7 - single * x;
    result.M33 = single4 + single1 * (1.0f - single4);
    result.M34 = 0.0f;

    result.M41 = 0.0f;
    result.M42 = 0.0f;
    result.M43 = 0.0f;
    result.M44 = 1.0f;
}

Vector4 Matrix::TransformPosition(const Matrix& m, const Vector3& v)
{
    return Vector4(
        m.Values[0][0] * v.Raw[0] + m.Values[1][0] * v.Raw[1] + m.Values[2][0] * v.Raw[2] + m.Values[3][0],
        m.Values[0][1] * v.Raw[0] + m.Values[1][1] * v.Raw[1] + m.Values[2][1] * v.Raw[2] + m.Values[3][1],
        m.Values[0][2] * v.Raw[0] + m.Values[1][2] * v.Raw[1] + m.Values[2][2] * v.Raw[2] + m.Values[3][2],
        m.Values[0][3] * v.Raw[0] + m.Values[1][3] * v.Raw[1] + m.Values[2][3] * v.Raw[2] + m.Values[3][3]
    );
}

Vector4 Matrix::TransformPosition(const Matrix& m, const Vector4& v)
{
    return Vector4(
        m.Values[0][0] * v.Raw[0] + m.Values[1][0] * v.Raw[1] + m.Values[2][0] * v.Raw[2] + m.Values[3][0] * v.Raw[3],
        m.Values[0][1] * v.Raw[0] + m.Values[1][1] * v.Raw[1] + m.Values[2][1] * v.Raw[2] + m.Values[3][1] * v.Raw[3],
        m.Values[0][2] * v.Raw[0] + m.Values[1][2] * v.Raw[1] + m.Values[2][2] * v.Raw[2] + m.Values[3][2] * v.Raw[3],
        m.Values[0][3] * v.Raw[0] + m.Values[1][3] * v.Raw[1] + m.Values[2][3] * v.Raw[2] + m.Values[3][3] * v.Raw[3]
    );
}
