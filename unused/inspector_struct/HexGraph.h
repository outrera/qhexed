/*
 * QHexEd - Simple Qt Based Hex Editor
 *
 * Copyright (C) 2007 Nikita Sadkov
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This code is free software.  You may redistribute copies of it under the terms of
 * the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.
 * There is NO WARRANTY, to the extent permitted by law.
*/

#ifndef HEXNET_H
#define HEXNET_H

#include <QHash>
#include <QVector>
#include <QString>
#include <QStack>
#include <HexPlugin.h>

template <class Key, class T>
class MultiHash : public QHash<Key, T>
{
public:
    MultiHash() {}
    MultiHash(const QHash<Key, T> &other) : QHash<Key, T>(other) {}

    inline typename QHash<Key, T>::iterator replace(const Key &key, const T &value)
    { return QHash<Key, T>::insert(key, value); }

    inline typename QHash<Key, T>::iterator insert(const Key &key, const T &value)
    { return QHash<Key, T>::insertMulti(key, value); }

    inline MultiHash &operator+=(const MultiHash &other)
    { unite(other); return *this; }
    inline MultiHash operator+(const MultiHash &other) const
    { MultiHash result = *this; result += other; return result; }

#ifndef Q_NO_USING_KEYWORD
    using QHash<Key, T>::contains;
    using QHash<Key, T>::remove;
    using QHash<Key, T>::count;
    using QHash<Key, T>::find;
    using QHash<Key, T>::constFind;
#else
    inline bool contains(const Key &key) const
    { return QHash<Key, T>::contains(key); }
    inline int remove(const Key &key)
    { return QHash<Key, T>::remove(key); }
    inline int count(const Key &key) const
    { return QHash<Key, T>::count(key); }
    inline int count() const
    { return QHash<Key, T>::count(); }
    inline typename QHash<Key, T>::iterator find(const Key &key)
    { return QHash<Key, T>::find(key); }
    inline typename QHash<Key, T>::const_iterator find(const Key &key) const
    { return QHash<Key, T>::find(key); }
    inline typename QHash<Key, T>::const_iterator constFind(const Key &key) const
    { return QHash<Key, T>::constFind(key); }
#endif

    bool contains(const Key &key, const T &value) const;

    int remove(const Key &key, const T &value);

    int count(const Key &key, const T &value) const;

    typename QHash<Key, T>::iterator find(const Key &key, const T &value) {
        typename QHash<Key, T>::iterator i(find(key));
        typename QHash<Key, T>::iterator end(this->end());
        while (i != end && i.key() == key) {
            if (i.value() == value)
                return i;
            ++i;
        }
        return end;
    }
    typename QHash<Key, T>::const_iterator find(const Key &key, const T &value) const {
        typename QHash<Key, T>::const_iterator i(constFind(key));
        typename QHash<Key, T>::const_iterator end(QHash<Key, T>::constEnd());
        while (i != end && i.key() == key) {
            if (i.value() == value)
                return i;
            ++i;
        }
        return end;
    }
    typename QHash<Key, T>::const_iterator constFind(const Key &key, const T &value) const
        { return find(key, value); }
private:
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;
};

template <class Key, class T>
Q_INLINE_TEMPLATE bool MultiHash<Key, T>::contains(const Key &key, const T &value) const
{
    return constFind(key, value) != QHash<Key, T>::constEnd();
}

template <class Key, class T>
Q_INLINE_TEMPLATE int MultiHash<Key, T>::remove(const Key &key, const T &value)
{
    int n = 0;
    typename QHash<Key, T>::iterator i(find(key));
    typename QHash<Key, T>::const_iterator end(QHash<Key, T>::constEnd());
    while (i != end && i.key() == key) {
        if (i.value() == value) {
            i = erase(i);
            ++n;
        } else {
            ++i;
        }
    }
    return n;
}

template <class Key, class T>
Q_INLINE_TEMPLATE int MultiHash<Key, T>::count(const Key &key, const T &value) const
{
    int n = 0;
    typename QHash<Key, T>::const_iterator i(constFind(key));
    typename QHash<Key, T>::const_iterator end(QHash<Key, T>::constEnd());
    while (i != end && i.key() == key) {
        if (i.value() == value)
            ++n;
        ++i;
    }
    return n;
}


template <typename TNode, typename TSig=int, typename TLabel=int> class HexGraph {
public:

	class Node;
	typedef void (*FunPtr)(...);

	struct Command {
		Command() {}
		Command(const Command &cmd) {*this = cmd;}
		Command(Node *src_, Node *dst_, FunPtr dstSlot_, void *data_) {
			src = src_;
			dst = dst_;
			dstSlot = dstSlot_;
			data = data_;
		}
		Node *src;
		Node *dst;
		FunPtr dstSlot;
		void *data;
	};
	struct Dst {
		Dst() {}
		Dst(const Dst &dst) {*this = dst;}
		Dst(Node *obj, FunPtr met) {
			object = obj;
			method = met;
		}
		inline bool operator==(const Dst &d) {
			return object == d.object && method == d.method;
		}
		Node *object;
		FunPtr method;
	};

	struct Dst;
	typedef QStack<Command> Stack;
	typedef QVector<TNode*> NodeList;
	typedef QHash<Node*,TLabel> NodeTable;
	typedef MultiHash<TSig,Dst> ConnTable;

	class Node {
	public:
		enum {
			BASE_NODE,
			USER_NODE,
		};

		enum {
			INIT_SIGNAL,
			USER_SIGNAL,
		};

		Node(HexGraph *g) {
#ifndef HEX_GRAPH_STATIC
			mGraph = g;
#endif
			nodes()[this] = TLabel();
		}

		Node(HexGraph *g, TLabel label) {
#ifndef HEX_GRAPH_STATIC
			mGraph = g;
#endif
			nodes()[this] = TLabel();
		}

		virtual ~Node() {
			nodes().remove(this);
		}

		virtual int nodeType() {
			return BASE_NODE;
		}

		HexGraph *graph() {
#ifndef HEX_GRAPH_STATIC
			return mGraph;
#else
			return &HexGraph::graph();
#endif
		}

		void clearConns() {
			conns().clear();
		}

		//template <typename T> void conn(TSig sig, Node *dst, T method) {
		//	conns().insertMulti(sig, Dst(dst, ptm(method)));
		//}

		//template <typename T> void dconn(TSig sig, Node *dst, T method) {
		//	conns().remove(sig, Dst(dst, ptm(method)));
		//}

#define conn(src,sig,dst,met) ({ \
		typedef typeof(*(dst)) _ThisClass; \
		typedef typeof(&_ThisClass::met) _ThisClassFun; \
		typedef typeof(*graph()) ThisGraph; \
		typeof(src) _src = (src); \
		typeof(sig) _sig = (sig); \
		typeof(dst) _dst = (dst); \
		_ThisClassFun _met = &_ThisClass::met; \
		_src->conns().insertMulti(_sig, ThisGraph::Dst(_dst, ptm(_met))); \
		true; \
})

#define dconn(src,sig,dst,met) ({ \
		typedef typeof(*(dst)) _ThisClass; \
		typedef typeof(&_ThisClass::met) _ThisClassFun; \
		typedef typeof(*graph()) ThisGraph; \
		typeof(src) _src = (src); \
		typeof(sig) _sig = (sig); \
		typeof(dst) _dst = (dst); \
		_ThisClassFun _met = &_ThisClass::met; \
		_src->conns().remove(_sig, ThisGraph::Dst(_dst, ptm(_met))); \
		true; \
})
	
		void dconnAll(TSig sig, Node *dst) {
			for(auto(i,conns().find(sig)); i != conns().end(); ) {
				if(i.value().object == dst) i = conns().erase(i);
				else i++;
			}
		}
	
		// returns list of all connections with signal 'sig'
		template <typename T> NodeList conns(TSig sig) {
			NodeList nl;
			for(auto(i,conns().find(sig)); i != conns().end() && i.key() == sig; i++)
				nl.append((TNode*)i.value().object);

			return nl;
		}

		template <typename T0, typename T1> void removeByValue(T0 &hash, T1 &val) {
			for(auto(i,hash.begin()); i != hash.end(); ) {
				if(i.value().object == val) i = hash.erase(i);
				else i++;
			}
		}
	
		void fillStack(TSig sig, void *data) {
			for(auto(i, conns().find(sig)); i != conns().end() && i.key() == sig; i++)
				stack().push(Command(this, i.value().object, i.value().method, data));
		}

		// sets continuation
		template <typename T> void cont(T cont, void *data=0) {
			stack().push(Command(this, this, ptm(cont), data));
		}

		// sets continuation
		template <typename T> void contFar(Node *n, T cont, void *data=0) {
			stack().push(Command(this, n, ptm(cont), data));
		}

		// broadcast signal 'sig' to all subscribed nodes
		void bcast(TSig sig, void *data=0) {
			fillStack(sig, data);
			if(!graph()->looping()) {
				graph()->mLooping = true;
				while(stack().size()) {
					Command cmd = stack().pop();
					cmd.dstSlot((TNode*)cmd.dst, (TNode*)cmd.src, cmd.data);
				}
				graph()->mLooping = false;
			}
		}

		template <typename T> FunPtr ptm(T x) {
			return (FunPtr)x;
			/*union {
				T t;
				struct {
					FunPtr method;
					void *unknown;
				};
			} u;
			u.t = x;
			return u.method;*/
		}


	protected:
		friend class HexGraph;

		NodeTable &nodes() {return graph()->nodes();}
		ConnTable &conns() {return mConns;}
		Stack &stack() {return graph()->stack();}

		ConnTable mConns;

#ifndef HEX_GRAPH_STATIC
		HexGraph *mGraph;
#endif
	};


#ifdef HEX_GRAPH_STATIC
private:
#endif
	HexGraph() {
		mLooping = false;
		mCounter = 0;
	}

	~HexGraph() {
		clear();
	}
#ifdef HEX_GRAPH_STATIC
public:
#endif

	int size() {
		return nodes().size();
	}

	bool looping() {
		return mLooping;
	}

	int totalConns() {
		int total = 0;
		for(auto(i,nodes().begin()); i != nodes().end(); i++)
			total += i.key()->conns().size();
		return total;
	}

	NodeTable &nodes() {
		return mNodes;
	}

	/*ConnTable &conns() {
		return mConns;
	}*/

	Stack &stack() {
		return mStack;
	}

	void squeeze() {
		nodes().squeeze();
		//conns().squeeze();
		stack().squeeze();
	}

	void clear() {
		QVector<Node*> v;
		for(auto(i,nodes().begin()); i != nodes().end(); i++)
			v.append(i.key());

		foreach(Node *n, v) {
			if(nodes().find(n) != nodes().end())
				delete n;
		}

		nodes().clear();
		stack().clear();
		squeeze();
		mCounter = 0;
		mLooping = false;
	}

	QString debugInfo() {
		return QString("nodes:%1").arg(nodes().size())
			+ QString(", conns:%1").arg(totalConns())
		;
	}
#ifdef HEX_GRAPH_STATIC
	static HexGraph &graph() {
		return mGraph;
	}
#endif


private:
	friend class Node;
	bool mLooping;
	int mCounter;
	NodeTable mNodes;
	QStack<Command> mStack;
#ifdef HEX_GRAPH_STATIC
	static HexGraph mGraph;
#endif
};

#ifdef HEX_GRAPH_STATIC
template <typename TNode, typename TLabel>
HexGraph<TNode,TLabel> HexGraph<TNode,TLabel>::mGraph;
#endif

#endif

