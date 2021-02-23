#ifndef GRAPH_H
#define GRAPH_H

#include "Object.h"
#include "SharedPointer.h"
#include "DynamicArray.h"
#include "Exception.h"
#include "LinkQueue.h"
#include "LinkStack.h"

namespace DTLib
{

template < typename E >
struct Edge: public Object
{
    int b;
    int e;
    E data;

    Edge(int i=-1,int j=-1)
    {
        b = i;
        e = j;
    }

    Edge(int i, int j, const E& value)
    {
        b = i;
        e = j;
        data = value;
    }

    bool operator == (const Edge& obj)
    {
        return (obj.b == this->b) && (obj.e == this->e);
    }

    bool operator != (const Edge& obj)
    {
        return !(*this == obj);
    }
};

template < typename V, typename E >
class Graph: public Object
{
protected:
    template< typename T >
    SharedPointer< Array<T> > toArray(LinkQueue<T>& queue)
    {
        DynamicArray<T>* ret = new DynamicArray<T>(queue.length());

        if(ret != NULL)
        {
            for(int i=0; i<ret->length(); i++, queue.remove())
            {
                ret->set(i, queue.front());
            }
        }
        else
        {
            THROW_EXCEPTION(NoEnoughMemonyException, "No memory to create ret object ...");
        }

        return ret;
    }
public:
    virtual V getVertex(int i) = 0;
    virtual bool getVertex(int i, V& value) = 0;
    virtual bool setVertex(int i, const V& value) = 0;
    virtual SharedPointer< Array<int> > getAdjacent(int i) = 0;
    virtual bool isAdjacent(int i, int j) = 0;
    virtual E getEdge(int i, int j) = 0;
    virtual bool getEdge(int i, int j, E& value) = 0;
    virtual bool setEdge(int i, int j, const E& value) = 0;
    virtual bool removeEdge(int i, int j) = 0;
    virtual int vCount() = 0;
    virtual int eCount() = 0;
    virtual int OD(int i) = 0;
    virtual int ID(int i) = 0;

    virtual int TD(int i)
    {
        return OD(i) + ID(i);
    }

    bool asUndirected()
    {
        bool ret = true;

        for(int i=0; i<vCount() && ret; i++)
        {
            for(int j=0; j<vCount() && ret; j++)
            {
                if(isAdjacent(i, j))
                {
                    ret = ret && (isAdjacent(j, i)) && (getEdge(i, j) == getEdge(j, i));
                }
            }
        }

        return ret;
    }

    SharedPointer< Array< Edge<E> > > prim(const E& LIMIT, const bool MINIUM=true)
    {
        LinkQueue< Edge<E> > ret;

        if(asUndirected())
        {
            DynamicArray<int> adjacent(vCount());
            DynamicArray<int> cost(vCount());
            DynamicArray<bool> mark(vCount());
            SharedPointer< Array<int> > aj;

            for(int i=0; i<vCount(); i++)
            {
                adjacent[i] = -1;
                cost[i] = LIMIT;
                mark[i] = false;
            }

            int v = 0;
            mark[v] = true;
            bool end = false;

            aj = getAdjacent(v);

            end = (aj->length() == 0);

            if(!end)
            {
                for(int i=0; i<aj->length(); i++)
                {
                    cost[ (*aj)[i] ] = getEdge(v, (*aj)[i]);
                    adjacent[ (*aj)[i] ] = v;
                }

                for(int i=0; i<vCount() && !end; i++)
                {
                    E k = LIMIT;
                    v = -1;

                    for(int j=0; j<aj->length(); j++)
                    {
                        if( !mark[ (*aj)[j] ] && (MINIUM ? cost[ (*aj)[j] ] < k : cost[ (*aj)[j] ] > k) )
                        {
                            k = cost[j];
                            v = (*aj)[j];
                        }
                    }

                    if(v >= 0)
                    {
                        mark[v] = true;

                        ret.add(Edge<E>(adjacent[v], v, getEdge( adjacent[v], v)));

                        aj = getAdjacent(v);

                        end = (aj->length() == 0);

                        if(!end)
                        {
                            for(int i=0; i<aj->length(); i++)
                            {
                                if( !mark[ (*aj)[i] ] && (MINIUM ? ( cost[ (*aj)[i] ] > getEdge(v, (*aj)[i]) ) : ( cost[ (*aj)[i] ] < getEdge(v, (*aj)[i]) ) ) )
                                {
                                    cost[ (*aj)[i] ] = getEdge(v, (*aj)[i]);
                                    adjacent[ (*aj)[i] ] = v;
                                }
                            }
                        }
                    }
                    else
                    {
                        end = true;
                    }
                }
            }
        }
        else
        {
            THROW_EXCEPTION(InvalidOperationException, "Prim only use in undirected graph ...");
        }

        if( ret.length() != (vCount() - 1) )
        {
            THROW_EXCEPTION(InvalidOperationException, "No enough edges for prim operation ...");
        }

        return toArray(ret);
    }

    SharedPointer< Array<int> > BFS(int i)
    {
        SharedPointer< Array<int> > ret;

        if( (0 <= i) && (i < vCount()) )
        {
            LinkQueue<int> v;
            LinkQueue<int> r;
            DynamicArray< bool > isVisit(vCount());

            for(int i=0; i<isVisit.length(); i++)
            {
                isVisit.set(i, false);
            }

            v.add(i);

            while (v.length() > 0)
            {
                int current = v.front();

                if(!isVisit[current])
                {
                    SharedPointer< Array<int> > ad = getAdjacent(current);

                    for(int i=0; i<ad->length(); i++)
                    {
                        v.add((*ad)[i]);
                    }

                    r.add(current);
                    isVisit[current] = true;
                }

                v.remove();
            }

            ret = toArray(r);
        }
        else
        {
            THROW_EXCEPTION(InvalidParameterException, "Invalid parameter i ...");
        }

        return ret;
    }

    SharedPointer< Array<int> > DFS(int i)
    {
        SharedPointer< Array<int> > ret;

        if( (0 <= i) && (i < vCount()) )
        {
            LinkStack<int> v;
            LinkQueue<int> r;
            DynamicArray<bool> isVisit(vCount());

            for(int i=0; i<isVisit.length(); i++)
            {
                isVisit[i] = false;
            }

            v.push(i);

            while(v.size() > 0)
            {
                int current = v.top();

                v.pop();

                if(!isVisit[current])
                {
                    SharedPointer< Array<int> > ad = getAdjacent(current);

                    for(int i=0; i<ad->length(); i++)
                    {
                        v.push((*ad)[i]);
                    }

                    isVisit[current] = true;

                    r.add(current);
                }
            }

            ret = toArray(r);
        }
        else
        {
            THROW_EXCEPTION(InvalidParameterException, "Invalid parameter i ...");
        }

        return ret;
    }
};

}

#endif // GRAPH_H
