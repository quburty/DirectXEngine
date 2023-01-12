#pragma once
class Math
{
public:
	static const float PI;
	static const float EPSILON;

	static float Modulo(float val1, float val2);

	static float ToRadian(float degree);
	static float ToDegree(float radian);

	static int Random(int r1, int r2);
	static float Random(float r1, float r2);
	
	static D3DXVECTOR2 RandomVec2(float r1, float r2);
	static D3DXVECTOR3 RandomVec3(float r1, float r2);
	static D3DXCOLOR RandomColor3();
	static D3DXCOLOR RandomColor4();

	static bool IsNearlyZero(float value);
	static bool IsNearlySame(float v1, float v2);
	static bool IsNearlyZero(double value);
	static bool IsNearlySame(double v1, double v2);

	static float Clamp(float value, float min, float max);

	static void LerpMatrix(OUT D3DXMATRIX& out, const D3DXMATRIX& m1, const D3DXMATRIX& m2, float amount);

	static D3DXQUATERNION LookAt(const D3DXVECTOR3& origin, const D3DXVECTOR3& target, const D3DXVECTOR3& up);
	static float Gaussian(float val, UINT blurCount);

	static void MatrixDecompose(const D3DXMATRIX& m, OUT D3DXVECTOR3& S, OUT D3DXVECTOR3& R, OUT D3DXVECTOR3& T);
};