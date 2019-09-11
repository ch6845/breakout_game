#pragma once

//#include <iostream>

class Vec3f {
public:
	Vec3f() {
		val[0] = 0;
		val[1] = 0;
		val[2] = 0;
	}

	Vec3f(float x, float y, float z) {
		this->val[0] = x;
		this->val[1] = y;
		this->val[2] = z;
	}


	//operator overloading	
	float& operator[](int n) {
		return val[n];
	}
	Vec3f operator-(Vec3f a) {
		Vec3f temp;
		for (int i = 0; i < 3; i++)
			temp.val[i] = val[i] - a.val[i];
		return temp;
	}
	Vec3f operator-=(Vec3f& a) {
		for (int i = 0; i < 3; i++)
			val[i] = val[i] - a.val[i];
		return *this;
	}

	Vec3f operator+(Vec3f& a) {
		Vec3f temp;
		for (int i = 0; i < 3; i++)
			temp.val[i] = val[i] + a.val[i];
		return temp;
	}
	Vec3f operator+=(Vec3f a) {
		for (int i = 0; i < 3; i++)
			val[i] = val[i] + a.val[i];
		return *this;
	}
	Vec3f operator*(float mag) {
		return Vec3f(val[0] * mag, val[1] * mag, val[2] * mag);
	}
	friend Vec3f operator*(float mag, const Vec3f &vec) {
		return Vec3f(vec.val[0] * mag, vec.val[1] * mag, vec.val[2] * mag);
	}
	Vec3f operator/(float mag) {
		return Vec3f(val[0] / mag, val[1] / mag, val[2] / mag);
	}
	friend Vec3f operator/(float mag, const Vec3f &vec) {
		return Vec3f(vec.val[0] * mag, vec.val[1] * mag, vec.val[2] * mag);
	}

	bool operator==(Vec3f &a) {
		if (val[0] == a.val[0] && val[1] == a.val [1] && val[2] == a.val[2])
			return true;
		else return false;
	}

	bool operator!=(Vec3f &a) {
		return !(*this == a);
	}

	void setposition(float x, float y, float z) {
		val[0] = x;
		val[1] = y;
		val[2] = z;

	}

	float val[3];
};


float dotoperator(Vec3f vec1, Vec3f vec2) {
	float sum = 0;
	for (int i = 0; i<3; i++) {
		sum += vec1[i] * vec2[i];
	}
	return sum;
}
