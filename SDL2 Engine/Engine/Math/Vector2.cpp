#include "Vector2.h"

#include <math.h>

float Vector2::Dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

Vector2 Vector2::Project(const Vector2& a, const Vector2& b)
{
	return Vector2(	(Vector2::Dot(a, b) / ((float)pow(b.x, 2) + (float)pow(b.y, 2))) * b.x,
					(Vector2::Dot(a, b) / ((float)pow(b.x, 2) + (float)pow(b.y, 2))) * b.y);
}

Vector2::Vector2()
{
	x = 0;
	y = 0;
}

Vector2::Vector2(const Vector2& other)
{
	x = other.x;
	y = other.y;
}

Vector2::Vector2(float _x)
{
	x = _x;
	y = _x;
}

Vector2::Vector2(float _x, float _y)
{
	x = _x;
	y = _y;
}

Vector2& Vector2::operator=(const Vector2& other)
{
	x = other.x;
	y = other.y;

	return *this;
}

Vector2 Vector2::operator+(const Vector2& other)
{
	Vector2 ret;

	ret.x = x + other.x;
	ret.y = y + other.y;

	return ret;
}

Vector2 Vector2::operator-(const Vector2& other)
{
	Vector2 ret;

	ret.x = x - other.x;
	ret.y = y - other.y;

	return ret;
}


Vector2 Vector2::operator*(const Vector2& other)
{
	Vector2 ret;

	ret.x = x * other.x;
	ret.y = y * other.y;

	return ret;
}


Vector2 Vector2::operator*(float _x)
{
	Vector2 ret;

	ret.x = x * _x;
	ret.y = y * _x;

	return ret;
}

bool Vector2::operator==(const Vector2& other)
{
	return (x == other.x && y == other.y);
}

bool Vector2::operator!=(const Vector2& other)
{
	return !operator==(other);
}
