#include <QtCore>

//#define HEX_GRAPH_STATIC
#include "HexGraph.h"

class TestNode;
typedef HexGraph<TestNode> Graph;

class Class {
public:

	void test() {
	}

private:
};

class TestNode : public Graph::Node {
public:

	enum {
		TEST_NODE=USER_NODE
	};

	enum {
		SIG_DEL=USER_SIGNAL,
	};

	enum {
		DEPTH=5,
		NPL=10,
	};

	TestNode(Graph *g) : Node(g), mDelFlag(false)
	{
		//ptm(&TestNode::nodeType);
	}

	TestNode(TestNode *n) : Node(n->graph()), mDelFlag(false)
	{
		//typedef typeof(*this) ThisClass;
		//typedef typeof(&ThisClass::del) ThisClassFun;
		//ThisClassFun a = &ThisClass::del;
		conn(n, SIG_DEL, this, del);
		//dconn(n, SIG_DEL, this, del);
		//n->dconnAll(SIG_DEL, this);
	}

	virtual ~TestNode() {
	}

	void del(TestNode *src) {
		if(!mDelFlag) {
			mDelFlag = true;
			cont(&TestNode::delFinal);
			bcast(SIG_DEL);
		}
	}

	void delFinal(TestNode *src) {
		delete this;
	}

	void test() {
		counter = 0;
		conn(this, INIT_SIGNAL, this, testReal);
		bcast(INIT_SIGNAL);
	}

	void testReal(TestNode *src) {
		graph()->squeeze();
		TestNode *root = new TestNode(graph());
		root->populate(DEPTH);
		qDebug() << "step" << counter << stack().size() << graph()->debugInfo();
		if(++counter < 10) cont(&TestNode::testReal);
		contFar(root, &TestNode::del);
	}

	void populate(int level) {
		if(!level) return;

		for(int i = 0; i < NPL; i++)
			(new TestNode(this))->populate(level-1);
	}

	virtual int nodeType() {
		return TEST_NODE;
	}

private:
	int counter;
	bool mDelFlag;
};

int main() {
#ifdef HEX_GRAPH_STATIC
	Graph *g = &Graph::graph();
#else
	Graph *g = new Graph;
#endif
	Class *c = new Class;
	c->test();

	TestNode *n = new TestNode(g);
	n->test();


#ifndef HEX_GRAPH_STATIC
	delete g;
#endif

	return 0;
}

