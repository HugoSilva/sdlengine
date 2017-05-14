#ifndef VECTOR2
#define VECTOR2

class Vector2
{
public:
	static float Dot(const Vector2& a, const Vector2& b);
	static Vector2 Project(const Vector2& a, const Vector2& b);

	Vector2();
	Vector2(const Vector2& other);
	Vector2(float _x);
	Vector2(float _x, float _y);

	Vector2& operator=(const Vector2& other);
	Vector2 operator+(const Vector2& other);
	Vector2 operator-(const Vector2& other);
	Vector2 operator*(const Vector2& other);
	Vector2 operator*(float _x);

	bool operator==(const Vector2& other);
	bool operator!=(const Vector2& other);

	float x;
	float y;
};

#endif
