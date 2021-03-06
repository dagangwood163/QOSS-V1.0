/*
*	created by liyun 2017/10/26
*   function 生产各种镜子的类
*   version 1.0
*/

#ifndef MIRRORFACTORY_H  
#define MIRRORFACTORY_H

#include "Mirror.h"

#include <vector>
#include <vtkjsoncpp/json/json.h>

class MirrorFactory
{
public:

	static Mirror* getMirror(MirrorsType type, const GraphTrans& graphTrans, 
		const std::vector<double>& parameter);
	static Mirror* getMirror(MirrorsType type, const GraphTrans& graphTrans);
	static Mirror* cloneMirror(Mirror*);

	static Mirror* getDefaultMirror(const GraphTrans& graphTrans);

	static Mirror* getMirrorByJson(const Json::Value & js);
private:
	MirrorFactory() {};
	~MirrorFactory() {};

};



#endif // MIRRORFACTORY_H
