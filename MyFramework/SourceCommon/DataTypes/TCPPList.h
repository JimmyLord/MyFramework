//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// Templated version of:
// CPPList.h - (C)2001 Dave 'Redbeard' Eccleston 

template <class MyType> class TCPPListNode : public CPPListNode
{
public:
    //MyType Next;
    //MyType Prev;
    ////  unsigned long Type;

    //TCPPListNode() : Next(0), Prev(0) {};
    //virtual ~TCPPListNode() {};

    // Gets the previous node, or NULL.
    MyType GetPrev()
    {
        if( Prev->Prev )
            return (MyType)Prev;
        else
            return 0;
    }

    // Gets the previous node.  This may be the list's head sentinel.
    MyType GetPrevQuick()
    {
        return (MyType)Prev;
    }

    // Gets the next node, or NULL.
    MyType GetNext()
    {
        if( Next->Next )
            return (MyType)Next;
        else
            return 0;
    }

    // Gets the next node.  This may be the list's tail sentinel.
    MyType GetNextQuick()
    {
        return (MyType)Next;
    }

    // Removes this node from whatever list it's in.  It *must* be in
    // a list before attempting this call.  Returns the removed node.
    MyType Remove()
    {
        Prev->Next = Next;
        Next->Prev = Prev;
        return (MyType)this;
    }

    // Adds this node before the specified node, in whatever list it's in.
    // Returns the added node.
    MyType AddBefore(MyType nodeToPrecede)
    {
        Next = nodeToPrecede;
        Prev = nodeToPrecede->Prev;
        Prev->Next = this;
        Next->Prev = this;
        return (MyType)this;
    }

    // Adds this node after the specified node, in whatever list it's in.
    // Returns the added node.
    MyType AddAfter(MyType nodeToFollow)
    {
        Prev = nodeToFollow;
        Next = nodeToFollow->Next;
        Next->Prev = this;
        Prev->Next = this;
        return (MyType)this;
    }

    // Moves this node from whatever list it's in to precede the specified
    // node in whatever list that node is in.  The node *must* be in a list
    // already.  Returns the added node.
    MyType MoveBefore(MyType nodeToPrecede)
    {
        Prev->Next = Next;
        Next->Prev = Prev;

        Next = nodeToPrecede;
        Prev = nodeToPrecede->Prev;
        Prev->Next = this;
        Next->Prev = this;
        return (MyType)this;
    }

    // Moves this node from whatever list it's in to follow the specified
    // node in whatever list that node is in.  The node *must* be in a list
    // already.  Returns the added node.
    MyType MoveAfter(MyType nodeToFollow)
    {
        Prev->Next = Next;
        Next->Prev = Prev;

        Prev = nodeToFollow;
        Next = nodeToFollow->Next;
        Next->Prev = this;
        Prev->Next = this;
        return (MyType)this;
    }

    // Links two node structures together, joining next and prev's.   This
    // is just here for completeness, as anyone should know how to do it
    // themselves.  Returns the first node.
    MyType LinkNodes(MyType nodeToAttach)
    {
        Next = nodeToAttach;
        nodeToAttach->Prev = this;
        return (MyType)this;
    }

    // Removes all nodes between this node and the specified end node from
    // their current list.  Both nodes must be in the same list and the
    // specified node must come later.  Returns the first removed node.
    MyType BulkRemove(MyType endNode)
    {
        Prev->Next = endNode->Next;
        endNode->Next->Prev = Prev;
        return (MyType)this;
    }

    // Adds the nodes between the current node and the specified endNode before
    // the specified node in a list.  Returns the first added node.
    MyType BulkAddBefore(MyType nodeToPrecede, MyType endNode)
    {
        endNode->Next = nodeToPrecede;
        Prev = nodeToPrecede->Prev;
        nodeToPrecede->Prev->Next = this;
        nodeToPrecede->Prev = endNode;
        return (MyType)this;
    }

    // Adds the nodes between the current node and the specified endNode after
    // the specified node in a list.  Returns the first added node.
    MyType BulkAddAfter(MyType nodeToFollow, MyType endNode)
    {
        endNode->Next = nodeToFollow->Next;
        Prev = nodeToFollow;
        nodeToFollow->Next->Prev = endNode;
        nodeToFollow->Next = this;
        return (MyType)this;
    }

    // Moves the nodes between the current node and the specified endNode before
    // the specified node in a list.  Returns the first moved node.
    MyType BulkMoveBefore(MyType nodeToPrecede, MyType endNode)
    {
        Prev->Next = endNode->Next;
        endNode->Next->Prev = Prev;

        endNode->Next = nodeToPrecede;
        Prev = nodeToPrecede->Prev;
        nodeToPrecede->Prev->Next = this;
        nodeToPrecede->Prev = endNode;
        return (MyType)this;
    }

    // Moves the nodes between the current node and the specified endNode after
    // the specified node in a list.  Returns the first moved node.
    MyType BulkMoveAfter(MyType nodeToFollow, MyType endNode)
    {
        Prev->Next = endNode->Next;
        endNode->Next->Prev = Prev;

        endNode->Next = nodeToFollow->Next;
        Prev = nodeToFollow;
        nodeToFollow->Next->Prev = endNode;
        nodeToFollow->Next = this;
        return (MyType)this;
    }

    //virtual bool operator <=(CPPListNode &pNode)
    //{
    //    // Just sort by address for basic functionality.
    //    // Override this to do what you want (what you really, really want).
    //    return this <= &pNode;
    //}
};

template <class MyType> class TCPPListHead
{
public:
    CPPListNode HeadNode;   // List's sentinel head node
    CPPListNode TailNode;   // List's sentinel tail node

    // Construct new list
    TCPPListHead()
    {
        HeadNode.Prev = 0;
        HeadNode.Next = &TailNode;
        TailNode.Prev = &HeadNode;
        TailNode.Next = 0;
    }

    // Empties the list
    void NewList()
    {
        HeadNode.Prev = 0;
        HeadNode.Next = &TailNode;
        TailNode.Prev = &HeadNode;
        TailNode.Next = 0;
    }

    // Destruct list
    ~TCPPListHead() {};

    // Get head node of the list.  NULL if empty
    MyType GetHead()
    {
        if( HeadNode.Next == &TailNode )
            return 0;
        else
            return (MyType)HeadNode.Next;
    }

    // Get head node of the list.  This may be the tail sentinel.
    MyType GetHeadQuick()
    {
        return (MyType)HeadNode.Next;
    }

    // Get the tail node of the list.  NULL if empty
    MyType GetTail()
    {
        if (TailNode.Prev == &HeadNode)
            return 0;
        else
            return (MyType)TailNode.Prev;
    }

    // Get the tail node of the list.  This may be the head sentinel
    MyType GetTailQuick()
    {
        return (MyType)TailNode.Prev;
    }

    // Returns TRUE if the list is empty.
    bool IsEmpty()
    {
        return HeadNode.Next == &TailNode;
    }

    // Returns TRUE if the list is not empty.
    bool NotEmpty()
    {
        return HeadNode.Next != &TailNode;
    }

    // Remove and return the head of the list.  Do not call on an empty list!
    MyType RemHeadQuick() 
    {
        MyType retval;

        retval = (MyType)HeadNode.Next;
        HeadNode.Next->Next->Prev = &HeadNode;
        HeadNode.Next = HeadNode.Next->Next;
        return retval;
    }

    // Remove and return the head of the list.  Returns NULL on empty list.
    MyType RemHead()
    {
        if( IsEmpty() )
            return 0;
        else
            return (MyType)RemHeadQuick();
    }

    // Remove and retrun the tail of the list.  Do not call on an empty list!
    MyType RemTailQuick()
    {
        MyType retval;

        retval = (MyType)TailNode.Prev;
        TailNode.Prev->Prev->Next = &TailNode;
        TailNode.Prev = TailNode.Prev->Prev;
        return retval;
    }

    // Remove and return the tail of the list.  Returns NULL on empty list.
    MyType RemTail()
    {
        if( IsEmpty() )
            return 0;
        else
            return RemTailQuick();
    }

    // Adds the given node to the head of the list.  Returns the added node.
    // If the node is already in a list, use MoveHead()
    MyType AddHead(MyType newNode)
    {
        return (MyType)newNode->AddAfter( (MyType)&HeadNode );
    }

    // Adds the given node to the tail of the list.  Returns the added node.
    // If the node is already in a list, use MoveTail()
    MyType AddTail(MyType newNode)
    {
        return (MyType)newNode->AddBefore( (MyType)&TailNode );
    }

    // Moves the specified node to the head of the list.  The node must already
    // be in a list, though not necessarily this one.  If the node is not in a
    // list, use AddHead() instead.
    // Returns the moved node.
    MyType MoveHead(MyType node)
    {
        return node->MoveAfter( (MyType)&HeadNode );
    }

    // Moves the specified node to the tail of the list.  The node must already
    // be in a list, though not necessarily this one.  If the node is not in a
    // list, use AddTail() instead.
    // Returns the moved node.
    MyType MoveTail(MyType node)
    {
        return node->MoveBefore( (MyType)&TailNode );
    }

    //// Adds all nodes between first and last to the head of the list.  This requires
    //// that nodes first and last are attached, and no nodes between them are in
    //// a list.  Returns the first added node.
    //MyType BulkAddHead(MyType firstNode, MyType lastNode)
    //{
    //    return firstNode->BulkAddAfter( &HeadNode, lastNode );
    //}

    //// Adds all nodes between first and last to the tail of the list.  This requires
    //// that nodes first and last are attached, and no nodes between them are in
    //// a list.  Returns the first added node.
    //MyType BulkAddTail(MyType firstNode, MyType lastNode)
    //{
    //    return firstNode->BulkAddBefore( &TailNode, lastNode );
    //}

    //// Moves all nodes between first and last to the head of the list.  This requires
    //// that nodes first and last are attached, and no nodes between them are in
    //// a list.  Returns the first moved node.
    //MyType BulkMoveHead(MyType firstNode, MyType lastNode)
    //{
    //    return firstNode->BulkMoveAfter( &HeadNode, lastNode );
    //}

    //// Moves all nodes between first and last to the tail of the list.  This requires
    //// that nodes first and last are attached, and no nodes between them are in
    //// a list.  Returns the first moved node.
    //MyType BulkMoveTail(MyType firstNode, MyType lastNode)
    //{
    //    return firstNode->BulkMoveBefore( &TailNode, lastNode );
    //}

    //// Appends the specified list onto the end of this list.  This call is safe for empty lists.
    //// Returns the first added node, or NULL on an empty list.
    //MyType Append(TCPPListHead<MyType>* listToAppend)
    //{
    //    if( listToAppend->NotEmpty() )
    //        return BulkMoveTail( listToAppend->HeadNode.Next, listToAppend->TailNode.Prev );
    //    return 0;
    //}

    //// Prepends the specified list onto the beginning of list list.  This call is safe for empty lists.
    //// Returns the first added node or NULL on an empty list.
    //MyType Prepend(TCPPListHead<MyType>* listToPrepend)
    //{
    //    if( listToPrepend->NotEmpty() )
    //        return BulkMoveHead( listToPrepend->HeadNode.Next, listToPrepend->TailNode.Prev );
    //    return 0;
    //}

    //// Merge sort this list using the specified compare function.
    //// The compare function should be of type
    //// signed char CmpFunc(CPPListNode *a, CPPListNode *b);
    //// Where the return is <0 if p is smaller than q, ==0 is p is equal to q, or
    //// >0 if q is smaller than p.
    //// If it makes life easier p<q and p==q are the same thing to this call.
    //void Sort(signed char (* sortFunc)(CPPListNode *a, CPPListNode *b))
    //{
    //    if (IsEmpty())
    //        return;
    //    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    //    // Apparently O(n log n), which is good.
    //    // Merge sort has issues on arrays (extra storage requirements) which are
    //    //  not an issue with list based implementations.  This is supposed to be
    //    //  a really good list sort algorithm.
    //    // This could be made faster by only maintaining the next pointers, and
    //    //  going back and filling in the prev pointers when the sort is complete.
    //    CPPListHead sortedList;
    //    long k = 1;
    //    long numMerges;
    //    CPPListNode *p, *q, *tmp;
    //    long psize, qsize;

    //    do
    //    {
    //        p = HeadNode.Next;
    //        numMerges = 0;
    //        while (p->Next)
    //        {
    //            numMerges++;
    //            q = p;
    //            psize = 0;
    //            while (psize < k)
    //            {
    //                psize++;
    //                q = q->Next;
    //                if (!q->Next)
    //                    break;
    //            }
    //            qsize = k;
    //            // Merge phase
    //            if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //            {
    //                while (qsize && psize)
    //                {
    //                    if (sortFunc(p, q) <= 0)
    //                    {
    //                        tmp = p->Next;
    //                        sortedList.AddTail(p);
    //                        psize--;
    //                        p = tmp;
    //                    }
    //                    else
    //                    {
    //                        tmp = q->Next;
    //                        sortedList.AddTail(q);
    //                        qsize--;
    //                        q = tmp;
    //                        if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //                            break;
    //                    }
    //                }
    //            }
    //            while (psize--)
    //            {
    //                tmp = p->Next;
    //                sortedList.AddTail(p);
    //                p = tmp;
    //            }
    //            while (qsize-- && q->Next)
    //            {
    //                tmp = q->Next;
    //                sortedList.AddTail(q);
    //                q = tmp;
    //            }
    //            p = q;
    //        }
    //        k <<= 1;
    //        HeadNode.Next = sortedList.HeadNode.Next;
    //        TailNode.Prev = sortedList.TailNode.Prev;
    //        HeadNode.Next->Prev = &HeadNode;
    //        TailNode.Prev->Next = &TailNode;
    //        sortedList.NewList();
    //    } while (numMerges > 1);
    //}

    //// Merge sort this list using the <= operator or the nodes.
    //void Sort()
    //{
    //    if (IsEmpty())
    //        return;
    //    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    //    // Apparently O(n log n), which is good.
    //    // Merge sort has issues on arrays (extra storage requirements) which are
    //    //  not an issue with list based implementations.  This is supposed to be
    //    //  a really good list sort algorithm.
    //    // This could be made faster by only maintaining the next pointers, and
    //    //  going back and filling in the prev pointers when the sort is complete.
    //    CPPListHead sortedList;
    //    long k = 1;
    //    long numMerges;
    //    CPPListNode *p, *q, *tmp;
    //    long psize, qsize;

    //    do
    //    {
    //        p = HeadNode.Next;
    //        numMerges = 0;
    //        while (p->Next)
    //        {
    //            numMerges++;
    //            q = p;
    //            psize = 0;
    //            while (psize < k)
    //            {
    //                psize++;
    //                q = q->Next;
    //                if (!q->Next)
    //                    break;
    //            }
    //            qsize = k;
    //            // Merge phase
    //            if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //            {
    //                while (qsize && psize)
    //                {
    //                    if ((*p) <= (*q))
    //                    {
    //                        tmp = p->Next;
    //                        sortedList.AddTail(p);
    //                        psize--;
    //                        p = tmp;
    //                    }
    //                    else
    //                    {
    //                        tmp = q->Next;
    //                        sortedList.AddTail(q);
    //                        qsize--;
    //                        q = tmp;
    //                        if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //                            break;
    //                    }
    //                }
    //            }
    //            while (psize--)
    //            {
    //                tmp = p->Next;
    //                sortedList.AddTail(p);
    //                p = tmp;
    //            }
    //            while (qsize-- && q->Next)
    //            {
    //                tmp = q->Next;
    //                sortedList.AddTail(q);
    //                q = tmp;
    //            }
    //            p = q;
    //        }
    //        k <<= 1;
    //        HeadNode.Next = sortedList.HeadNode.Next;
    //        TailNode.Prev = sortedList.TailNode.Prev;
    //        HeadNode.Next->Prev = &HeadNode;
    //        TailNode.Prev->Next = &TailNode;
    //        sortedList.NewList();
    //    } while (numMerges > 1);
    //}

    //// Merge sort this list using the long at the given offset as the sort field
    //void SortLong(long offset)
    //{
    //    if (IsEmpty())
    //        return;
    //    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    //    // Apparently O(n log n), which is good.
    //    // Merge sort has issues on arrays (extra storage requirements) which are
    //    //  not an issue with list based implementations.  This is supposed to be
    //    //  a really good list sort algorithm.
    //    // This could be made faster by only maintaining the next pointers, and
    //    //  going back and filling in the prev pointers when the sort is complete.
    //    CPPListHead sortedList;
    //    long k = 1;
    //    long numMerges;
    //    CPPListNode *p, *q, *tmp;
    //    long psize, qsize;

    //    do
    //    {
    //        p = HeadNode.Next;
    //        numMerges = 0;
    //        while (p->Next)
    //        {
    //            numMerges++;
    //            q = p;
    //            psize = 0;
    //            while (psize < k)
    //            {
    //                psize++;
    //                q = q->Next;
    //                if (!q->Next)
    //                    break;
    //            }
    //            qsize = k;
    //            // Merge phase
    //            if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //            {
    //                while (qsize && psize)
    //                {
    //                    if ( *((long *)(&((char *)p)[offset])) <= *((long *)(&((char *)q)[offset])) )
    //                    {
    //                        tmp = p->Next;
    //                        sortedList.AddTail(p);
    //                        psize--;
    //                        p = tmp;
    //                    }
    //                    else
    //                    {
    //                        tmp = q->Next;
    //                        sortedList.AddTail(q);
    //                        qsize--;
    //                        q = tmp;
    //                        if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //                            break;
    //                    }
    //                }
    //            }
    //            while (psize--)
    //            {
    //                tmp = p->Next;
    //                sortedList.AddTail(p);
    //                p = tmp;
    //            }
    //            while (qsize-- && q->Next)
    //            {
    //                tmp = q->Next;
    //                sortedList.AddTail(q);
    //                q = tmp;
    //            }
    //            p = q;
    //        }
    //        k <<= 1;
    //        HeadNode.Next = sortedList.HeadNode.Next;
    //        TailNode.Prev = sortedList.TailNode.Prev;
    //        HeadNode.Next->Prev = &HeadNode;
    //        TailNode.Prev->Next = &TailNode;
    //        sortedList.NewList();
    //    } while (numMerges > 1);
    //}

    //// Merge sort this list using the long at the given offset as the sort field
    //void SortFloat(long offset)
    //{
    //    if (IsEmpty())
    //        return;
    //    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    //    // Apparently O(n log n), which is good.
    //    // Merge sort has issues on arrays (extra storage requirements) which are
    //    //  not an issue with list based implementations.  This is supposed to be
    //    //  a really good list sort algorithm.
    //    // This could be made faster by only maintaining the next pointers, and
    //    //  going back and filling in the prev pointers when the sort is complete.
    //    CPPListHead sortedList;
    //    long k = 1;
    //    long numMerges;
    //    CPPListNode *p, *q, *tmp;
    //    long psize, qsize;

    //    do
    //    {
    //        p = HeadNode.Next;
    //        numMerges = 0;
    //        while (p->Next)
    //        {
    //            numMerges++;
    //            q = p;
    //            psize = 0;
    //            while (psize < k)
    //            {
    //                psize++;
    //                q = q->Next;
    //                if (!q->Next)
    //                    break;
    //            }
    //            qsize = k;
    //            // Merge phase
    //            if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //            {
    //                while (qsize && psize)
    //                {
    //                    if ( *((float *)(&((char *)p)[offset])) <= *((float *)(&((char *)q)[offset])) )
    //                    {
    //                        tmp = p->Next;
    //                        sortedList.AddTail(p);
    //                        psize--;
    //                        p = tmp;
    //                    }
    //                    else
    //                    {
    //                        tmp = q->Next;
    //                        sortedList.AddTail(q);
    //                        qsize--;
    //                        q = tmp;
    //                        if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
    //                            break;
    //                    }
    //                }
    //            }
    //            while (psize--)
    //            {
    //                tmp = p->Next;
    //                sortedList.AddTail(p);
    //                p = tmp;
    //            }
    //            while (qsize-- && q->Next)
    //            {
    //                tmp = q->Next;
    //                sortedList.AddTail(q);
    //                q = tmp;
    //            }
    //            p = q;
    //        }
    //        k <<= 1;
    //        HeadNode.Next = sortedList.HeadNode.Next;
    //        TailNode.Prev = sortedList.TailNode.Prev;
    //        HeadNode.Next->Prev = &HeadNode;
    //        TailNode.Prev->Next = &TailNode;
    //        sortedList.NewList();
    //    } while (numMerges > 1);
    //}

    //// Convert the list to an array of node pointers.
    //// Returns the number of used entries from 0 to maxNodes.
    //// The entries are optionally removed from the list.  If you choose to
    ////  removed entries, and more entries are in the list than the array, the
    ////  remaining entries will be left in the list.
    //long ToArray(CPPListNode **storage, long maxNodes, char removeNodes = 0)
    //{
    //    CPPListNode *ptr;
    //    long numNodes = 0;

    //    for (ptr = HeadNode.Next; ptr->Next && numNodes != maxNodes; ptr = ptr->Next)
    //    {
    //        *storage++ = ptr;
    //        numNodes++;
    //    }
    //    if (removeNodes)
    //    {
    //        HeadNode.Next = ptr;
    //        ptr->Prev = &HeadNode;
    //    }
    //    return numNodes;
    //}

    //// Converts an array of node pointers to a set of linked nodes at the head
    //// of the list.
    //void FromArrayToHead(CPPListNode **storage, long numNodes)
    //{
    //    CPPListNode *firstNode = HeadNode.Next;
    //    CPPListNode *prevNode = &HeadNode;

    //    while(numNodes--)
    //    {
    //        prevNode->Next = *storage;
    //        (*storage)->Prev = prevNode;
    //        prevNode = *storage++;
    //    }
    //    prevNode->Next = firstNode;
    //    firstNode->Prev = prevNode;
    //}

    //// Converts an array of node pointers to a set of linked nodes at the tail
    //// of the list.
    //void FromArrayToTail(CPPListNode **storage, long numNodes)
    //{
    //    CPPListNode *lastNode = TailNode.Prev;
    //    CPPListNode *nextNode = &TailNode;

    //    storage += numNodes-1;
    //    while(numNodes--)
    //    {
    //        nextNode->Prev = *storage;
    //        (*storage)->Next = nextNode;
    //        nextNode = *storage--;
    //    }
    //    nextNode->Prev = lastNode;
    //    lastNode->Next = nextNode;
    //}

    //// Specialized alist v2.0 features that we don't require.
    //// These would require a subclassed type that had some field to enqueue on
    ////  or would require an enqueueing function call (slow!)
    ////AddEnqueue
    ////MoveEnqueue
};
