/*
*	created by liyun 2018/1/23
*   function 计算反射面 输入镜子的指针 输出反射光线和交点以及是否相交
*   输入 源Ex_In,Ey_In, 源位置、传播方向，反射面
*   依赖RayTracing做光线追踪部分
*   version 1.0
*/
#ifndef PVVA_H
#define PVVA_H

#include <iostream>
#include <vector>
#include <cassert>
#include <vector>
#include <list>
#include <complex>

#include "../util/Vector3.h"
#include "../util/Vector2.h"
#include "../util/GEMS_Memory.h"
#include "../util/Constant_Var.h"
#include "FFTDI.h"

#include "VTK/include/Field.h"
#include "VTK/include/Mirror.h"
#include "RayTracing.h"



using namespace std;

namespace calculation
{
	class PVVA
	{
	public:
		PVVA();
		~PVVA();

		void Initialization(); // 数据初始化

		void setUnit(double factor); // 设置单位

		void setSource(const Field* _field); // 设置源

		void setFre(double _fre);

		void setMirror(Mirror * mirror);

		void CalZ0Theta(); // 计算z0 和 theta

						   // 得到源的中点坐标以及传播方向
		void getSourcePoint(Vector3 & interPoint, Vector3 & n_Point) const;

		// 源的中点坐标以及传播方向
		void setSourcePoint(const Vector3 & interPoint, const Vector3 & n_Point);

		static GraphTrans getSourceGraphTrans(const Vector3 & n_Point);

		bool CheckModle(Vector3 & interPoint, Vector3 & n_Point); // check是否能计算

		void getField(Field * _field); // 返回结果

		void CalPlane(double dis); //计算平面传播 并更新平面位置

		void Reflect(); // 计算反射 改变了Exyz_R的相位和极化 更新了反射坐标

		void CalAmplitude(); // 计算反射前后的投影面积 更新Exyz_R幅度变换

		void InterVal(); // 对反射回来的波进行插值，使其落入标准网格中

		void Result(double dis = 1.0); // 得到结果面



		void getPlane(Vector3 ** &_org, Vector3 ** &_n) const; // 得到平面点和点的方向

	private:

		void Poynting(); //计算面的坡应廷矢量

		void AllocateMemory();  //分配内存

		void updateSource_n(Vector3 new_n); // 更新源的法向量以及旋转信息


		//复数的共轭相乘 A * B(共轭)
		complex<double> ConjugateMul(const complex<double> &A, const complex<double> &B) const;
		//复数的相乘 A * B
		complex<double> ComMul(const complex<double> &A, const complex<double> &B) const;

		double CalDistance(const Vector3 &a, const Vector3 &b) const;
	
		/************************************************
		* 直线与平面相交检测 orig直线起点 dir 射线方向
		* 返回t t>0 相交在直接前方 t<0 相交在直线后方
		* 平面为点法式 n*(P-P0) = 0
		* P0为平面上的一点 n为平面法线
		***************************************************/
		double IntersectPlane(const Vector3 &orig, const Vector3 &dir, const Vector3 &Plane_org,
			const Vector3 &Plane_n, Vector3 &intersection);

		double IntersectPoint(const Vector3 &orig, const Vector3 &dir, const Vector3 &v0,
			const Vector3 &E1, const Vector3 &E2, Vector3 &intersection);

		// 计算反射电场 输入法向量 入射电场ExEyEz 
		void CalReflectExyz(const Vector3 &n, const complex<double> &Ex, const complex<double> &Ey,
			const complex<double> &Ez, complex<double> &Ex_out, complex<double> &Ey_out, complex<double> &Ez_out);

		// 计算四个点围城的面积  等效于计算三角形ABC 和 三角形ADC的面积和
		double CalSquare(const Vector3 &A, const Vector3 &B, const Vector3 &C, const Vector3 &D) const;
		double CalSquare(const Vector3 &A, const Vector3 &B, const Vector3 &C) const;

		// 插值用的——判断射线与线段是否相交
		bool InterVal_IsInsect(const Vector2 &A, const Vector2 &B, const Vector2 &C, const Vector2 &D);


	private:
		int N, M; // 计算矩形的长宽 N*M
		double f; // 频率 默认 10.65e9
		double lamda; // 波长
		double k;
		double theta; // 经过反射后源相对于z正轴的夹角
		double ds;
		double w0; // 束腰
		double z0; //目标距离

		double unit_factor; // 保存模型的单位转化为米的系数

		Mirror * mirror;

		//模型参数 (解析式用)
		//vector<float> ModlePara; // 模型参数
		//GraphTrans ModleGraphTrans; // 旋转参数
		//int ModleIndex;

		GraphTrans SourceGraphTrans;// 源的旋转参数

		complex<double> ** Px, ** Py, ** Pz;	// 坡应廷矢量 
		complex<double> ** Ex1, ** Ey1, ** Ez1;	// 经过calculation计算后的到的电磁场
		complex<double> ** Hx1, ** Hy1, ** Hz1;	// 

		complex<double> ** Ex_In, ** Ey_In;	// 源 

		complex<double> ** Ex_R, ** Ey_R, ** Ez_R;	// 经过反射计算后的到的电场
		complex<double> ** Ex_R_Inter, ** Ey_R_Inter, ** Ez_R_Inter;	// 插值后的电场

		string ProjName;   // 数据的地址
		int NodesNum, EleNum;  // .dat的元素和节点的个数

		Vector3 Org_Source, n_Source; //Org_Source平面中点 n_Source平面法向量
		double theta_Source;   // 源与-Z轴的夹角

		Vector3 **Plane;  // 平面各点的坐标（绝对坐标）
		Vector3 **n_Plane;  // 平面传播各点后的法向量
		Vector3 **R_Plane; // 反射后的各点的坐标

		Vector3 **InterVal_Plane; // 插值平面
		struct coordinate_ij
		{
			int i, j;
		};
		vector<coordinate_ij> **Interpolation; //插值用的中间变量
											   // test用 删
		Vector3 InterPoint_test; //
		Vector3 **Plane_org;   // 反射后的向量
		Vector3 **test_Plane;

		vector<double> Analytic_data; // 解析解用的所有数据0~6为模型的位置数据
	};
}



#endif
