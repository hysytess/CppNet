#include "HelloWorldScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    /**  you can create scene with following comment code instead of using csb file.
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    **/
    
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto rootNode = CSLoader::createNode("MainScene.csb");

    addChild(rootNode);

	// 启用场景刷新
	scheduleUpdate();

	//////////////////////////////EasyTcpClient///////////////////////////
	
	this->Connect("127.0.0.1", 4567);

    return true;
}

int HelloWorld::sendStream()
{
	CellWriteStream byteStream;

	byteStream.setNetCmd(CMD_LOGOUT);

	byteStream.WriteInt8(5);
	byteStream.WriteInt16(6);
	byteStream.WriteInt32(7);
	byteStream.WriteInt64(8);

	byteStream.WriteFloat(14.0);
	byteStream.WriteDouble(15.0);

	char str[]{ "client." };
	char str0[5] = "abc";
	int pos0[]{ 5,6,7 };
	int pos[2]{ 1,2 };
	byteStream.WriteArray(str, strlen(str));
	byteStream.WriteArray(pos, 2);

	byteStream.WriteString(str0);
	byteStream.WriteArray(pos0, 3);
	byteStream.finsh();

	CellLog::Instance().setLogPath("BitStreamForClientGoLog.txt", "w");


	return SendData(byteStream.data(), byteStream.length());

}

void HelloWorld::update(float delta)
{
	sendStream();
	this->OnRun();
}

void HelloWorld::OnNetMsg(netmsg_DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGOUT_RESULT:
	{
		CellReadStream byteStream(header);
		byteStream.ReadInt16();
		byteStream.getNetCmd();
		auto a1 = byteStream.ReadInt8();
		auto a2 = byteStream.ReadInt16();
		auto a3 = byteStream.ReadInt32();
		auto a4 = byteStream.ReadInt64();
		auto a5 = byteStream.ReadFloat();
		auto a6 = byteStream.ReadDouble();

		char str1[10]{};
		auto a7 = byteStream.ReadArray(str1, 8);
		int pos1[3]{};
		auto a9 = byteStream.ReadArray(pos1, 3);

		char str2[10]{};
		auto a8 = byteStream.ReadArray(str2, 10);
		int pos2[6]{};
		auto a10 = byteStream.ReadArray(pos2, 6);

		//printf("<socket=%d>收到服务器消息：CMD_LOGIN_RESULT,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
	}
	break;
	case CMD_ERROR:
	{
		netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
		//printf("<socket=%d>收到服务器消息：CMD_ERROR,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
	}
	break;
	default:
	{
		CellLog::Info("Error! <socket=%d>, dataLength: %d\n", (int)_pClient->sockfd(), header->dataLength);
	}
	}
}
