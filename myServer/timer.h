#pragma once
#include <memory>
#include <queue>

using namespace std;
class httpData;
class timerNode{
public:
	timerNode(shared_ptr<httpData> requestData, int timeout);
	~timerNode();
	bool isValid();
	void clearRequests();
	void setDeleted(){deleted = true;}
	//默认内联(头文件内实现)
	bool isDeleted()
	{
		return deleted;
	}
	void update(int timeout);
	size_t getExpiredTimer() const{return expiredTimer;}
private:
	bool deleted;
	size_t expiredTimer;
	shared_ptr<httpData> spHttpData;
};

struct TimerCmp{
	bool operator()(shared_ptr<timerNode> &a, shared_ptr<timerNode> &b){
		return a->getExpiredTimer() > b->getExpiredTimer();
	}
};

class timerNodeManager{
public:
	// timerNodeManager();
	// ~timerNodeManager();
	void addTimer(shared_ptr<httpData> spHttpData, int timeout);
	void handleExpiredEvent();

private:
	typedef shared_ptr<timerNode> spTimerNode;
	priority_queue<spTimerNode, deque<spTimerNode>, TimerCmp> timerNodeQueue;
};

