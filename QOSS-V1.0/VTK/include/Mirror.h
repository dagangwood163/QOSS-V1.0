/*
*	created by liyun 2017/10/23
*   function 镜子的虚基类 
*   version 1.0
*/

#ifndef MIRROR_H  
#define MIRROR_H

#include "util/GraphTrans.h"
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <QTreeWidgetItem>
#include <QString>

#include <vector>
#include <../util/Vector3.h>

#include "BasicParameters.h"
#include "Restriction.h"

enum MirrorsType
{
	PLANEMIRROR = 0,
	QUADRICSURFACE,
	PARABOLICCYLINDER,
	PARABOLOID,
	ELLIPSOID,
};

class actor;
class Mirror : public BasicParameters
{
public:
	Mirror();
	virtual ~Mirror();

	MirrorsType getMirrorsType() const;

	// cal(计算) and get actor 
	vtkSmartPointer<vtkActor> getActor() const;
	virtual void calActor();

	// cal(计算) and get polyData 
	vtkSmartPointer<vtkPolyData> getPolyData() const;
	virtual void calPolyData(double ds = 0) = 0;

	virtual void updateData() = 0;

	// 输出入射光线计算交点和法线
	virtual void calcReflectedRay(const Vector3&, Vector3&, Vector3&) = 0;

	void setSelected(bool);

	virtual QTreeWidgetItem* getTree() { return nullptr; };

	void addRestriction(Restriction*);
	void setRestriction(int,Restriction*);
	void removeRestriction(int);
	Restriction* getRestriction(int) const;

	void switchIsTransparent();
	bool getIsTransparent() const { return isTransparent; }

	void switchIsShow();
	bool getIsShow() const { return isShow; }

protected:

	QTreeWidgetItem* getTransformTree();

	MirrorsType type;

	//保存每个模型的显示和剖分数据
	vtkSmartPointer<vtkActor> actor;
	vtkSmartPointer<vtkPolyData> polyData;

	bool isTransparent; // 是否透明
	bool isShow; // 是否显示
	vtkSmartPointer<vtkProperty> property;

	vector<Restriction*> restrictions;
private:

};



#endif // MIRROR_H