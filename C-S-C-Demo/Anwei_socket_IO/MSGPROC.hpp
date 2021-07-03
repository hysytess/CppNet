#ifndef MSGPROC_HPP
#define MSGPROC_HPP

enum DataPackageType {
	DPT_EXIT = 0,
	DPT_MSG,
	DPT_NEW_USER_JOIN,
	DPT_ERROR
};

struct DataHeader {
	short dataLength;
	short cmd;
};

struct Message : public DataHeader {
	Message() {
		dataLength = sizeof(Message);
		cmd = DPT_MSG;
	}
	unsigned int sender;
	unsigned int receiver;
	char msg[126];
};

#endif // MSGPROC_HPP