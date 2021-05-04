#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "../depends/include/CellMsgStream.hpp"
#include "../depends/include/EasyTcpClient2_3.hpp"

class HelloWorld : public cocos2d::Layer,public EasyTcpClient
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

	void update(float delta);

	int sendStream();

	virtual void OnNetMsg(netmsg_DataHeader* header);
};

#endif // __HELLOWORLD_SCENE_H__
