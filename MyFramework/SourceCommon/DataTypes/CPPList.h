#ifndef _CPPLIST_H_
#define _CPPLIST_H_

/* CPPList.h - (C)2001 Dave 'Redbeard' Eccleston 

   This code is being released into the public domain.  Feel free to use
   and abuse it all you want.  I am constantly amazed at how many people
   have never seen or heard of this type of list, and write their own
   horrid list classes.  This is meant to save everyone from wasting both
   their time, and our CPU's time, by making their own slow list class.


   This is a CPP class version of the Amiga(TM) computer's style of linked
   lists.  In addition to the basic list functionality you expect, I've added
   some extras.  First I'll cover the basics of how this works, then delve
   into the usage basics, then the advanced stuff.  This is one of the
   fastest, most versatile types of linked list I've even laid eyes on, which
   is why I bothered to mimic it.  There are (almost) no safety checks
   provided.  If you believe that you write bad code, add some checks
   yourself, or don't use these classes.  They are geared for performance.
   


   What makes this list different from the usual double-linked list?

     You may have heard of techniques that use "sentinel nodes" bordering
     each edge of a list.  The Amiga used this technique.  As you'll see if
     you delve into the actual code, there are no special cases for adding
     to empty lists, beginnings of lists, and things like that.  The head
     and tail pointers are automagically updated, because the head and tail
     pointers are the sentinel nodes, and update with changes to the list
     like any other node would.  You'll notice that even though you don't
     specify a list when removing a node, the head and tail pointers are
     updated appropriately, if required.  You'll notice there is no special
     code to handle this behaviour.

     This code is fast, and I'm giving it away free.  You'd be stupid
     not to use it.



   Memory layout (skip it if you don't care... go ahead... you may be back)

     A node structure begins with two pointers, Next, and Prev.  You can
     subclass this to add any user defined data you want.  See the test
     code for an example.  It just adds a ULONG, but you'll get the idea.

     CPPListNode:
       CPPListNode *Next;
       CPPListNode *Prev;

     A list structure is two node structures, holding the two sentinel
     nodes that always remain at the edge of the list.  The Amiga took
     advantage of the fact that the head sentinel's previous pointer
     would always be null, and the tail sentinel's next pointer would
     always be null, and thus stored the two node structures overlapping.
     I didn't do this, as I don't trust all CPP compilers to like it, and
     it's only 4 bytes for god's sake.

     CPPListHead:
       CPPListNode HeadNode;
       CPPListNode TailNode;

      - or, expanded -

     CPPListHead:
       CPPListNode *HeadNode_Next;
       CPPListNode *HeadNode_Prev;  - always NULL
       CPPListNode *TailNode_Next;  - always NULL
       CPPListNode *TailNode_Prev;

     Empty lists, and small sample lists:

     As you'll notice, an empty list consists of the two sentinel nodes
     linked together.  Nothing special.

     Empty List:
       HeadNode.Next = &TailNode;
       HeadNode.Prev = 0;
       TailNode.Next = 0;
       TailNode.Prev = &HeadNode;

     In a simple sample list, the contents of the list are between the two
     sentinel nodes.

     Simple List ( [Head]-[Test]-[Tail] ) :
       HeadNode.Next = &TestNode;
       HeadNode.Prev = 0;
       TestNode.Next = &TailNode;
       TestNode.Prev = &HeadNode;
       TailNode.Next = 0;
       TailNode.Prev = &TestNode;

     Simple List 2 ( [Head]-[Test]-[Blah]-[Tail] ) :
       HeadNode.Next = &TestNode;
       HeadNode.Prev = 0;
       TestNode.Next = &BlahNode;
       TestNode.Prev = &HeadNode;
       BlahNode.Next = &TailNode;
       BlahNode.Prev = &TestNode;
       TailNode.Next = 0;
       TailNode.Prev = &BlahNode;



   Basic stuff:

     The basics of how to use a linked list should be obvious.  The test code
     at the end may be useful if you're unsure.  The basic list operations
     are as follows:

     Adding Nodes:

       List.AddHead(&Node);  Adds to the front of the list
       List.AddTail(&Node);  Adds to the end of the list.
       Node.AddBefore(&OtherNode);  Adds Node infront of OtherNode
       Node.AddAfter(&OtherNode);  Adds Node behind of OtherNode

     Removing Nodes:

       List.RemHead();  Removes the first node in the list.
       List.RemTail();  Removes the last node in the list.
       Node.Remove();   Removes the node from it's list.
       List.RemHeadQuick();   Doesn't check if the list is empty.
       List.RemTailQuick();   Doesn't check if the list is empty.

     Checking the status of the list:
     
       List.IsEmpty();  Returns true if the list is empty.
       List.NotEmpty(); Returns true if the list is not empty.
       List.NewList();  Return the list to an empty state.

     Traversing the list:
       
       There are a couple ways to traverse the list.  The first involves
       you actually understanding how the list works in RAM, or some
       memorization skills (take your pick).  This was commonly seen
       throughout all Amiga applications.  Notice that it breaks out of
       the loop when the node's next is null, not when the node is null.
       When the next is null, we are at the sentinel, and we probably
       don't want to do any processing on it.

         for( node = List.HeadNode->Next; node->Next; node = node->Next )
         {
            Do stuff with 'node'.
         }

       Alternately, if you require removal, moving, etc, during traversal,
       you'll have to grab the next pointer before you trash it.

         for( node = List.HeadNode->Next, node2 = node->Next;
           node2;
           node = node2, node2 = node2->Next )
         {
            Do stuff with 'node'.
         }

       If you're more worried about readability than speed, then the
       following is more your style.

         for (node = List.GetHead(); node; node.GetNext())
         {
            Do stuff with 'node'.
         }

       or if moves or deletes are likely to occur, the you'll want the
       following snippet.

         for (node = List.GetHead(), node2 = node.GetNext();
           node;
           node = node2, node2 = node2.GetNext())
         {
            Do stuff with 'node'.
         }

     Sorting

         signed char sortFunc(CPPListNode *a, CPPListNode *b)
         {
           DataNode *aa=(DataNode *)a, *bb=(DataNode *)b;

           if (aa->num <= bb->num)
             return -1;
           return 1;
         }

         myList.Sort(sortFunc);
         myList.SortLong(offsetof(DataNode, num));
         myList.SortFloat(offsetof(FloatDataNode, fnum));


   Advanced Stuff:
   
     I've taken the liberty of adding some new features to the Amiga's lists.
     Among these are bulk operations, and move operations.

     MoveHead(), MoveTail(), MoveBefore(), MoveAfter() work
     exactly as their Add counterparts, except that they perform a Remove()
     first.  That is, Add is meant for nodes that are not currently in a
     list, where as Move is meant for nodes that are already in a list.  Move
     can be used to move a node from one point to another in one list, or to
     move the node from one list to an entirely different list.  It is
     identical to calling Remove(), then AddHead(), AddTail(), etc...

     BulkAddHead(), BulkAddTail(), BulkMoveHead(), BulkMoveTail(),
     BulkAddAfter(), BulkAddBefore(), BulkMoveAfter, BulkMoveBefore(),
     BulkRemove(), all work similar to their non-bulk versions, except that
     if you have a series of nodes, already linked together you can perform
     an operation on them at once, running at the same speed as if you were
     only affecting one node.
     
     If you want to remove items 2 through 4 from a 5 item list, they you'd
     do something like this:
      
       Item2.BulkRemove(&Item4);

     If you wanted to move a bunch of selected items to a different list,
     then you would do something like:

       selectionList.BulkMoveTail(&FirstItem, &LastItem);

     For bulk operations to work, the two nodes must be linked, with any
     number of links between them, and must be in first, last order.  The
     amount of code run to perform a bulk operation is identical to a single
     non-bulk operation, ie:  A bulk add of 50 items takes the same amount of
     code as a normal add of 1 item, which is why I introduced bulk
     operations.

     Prepend(), and Append() are available in the list class to join two
     lists together.  These are among the few calls that check if a list
     is empty and don't die if conditions are wrong.

       ListOne.Append(&ListTwo);    Adds list two onto the end of list one.
       ListOne.Prepend(&ListTwo);   Adds list two onto the front of list one.


     Lists can be converted to and from arrays of node pointers.  When
     converting to an array, the nodes can be optionally removed from the
     list, or left in the list.  When adding an array to a list, you have
     the choice of adding the node to the head or tail of the list.  If
     more nodes are in the list than you have array space, they are left
     in the list if you opted for removal.  The number of nodes actually
     moved into the array is returned.

     CPPListNode *array[maxnodes];

     num = ListOne.ToArray(&array[0], maxnodes, 1); Converts with removal from list
     num = ListOne.ToArray(&array[0], maxnodes);    Leaves nodes in list

     ListOne.FromArrayToHead(&array[0], num)        Places num nodes at head of list
     ListOne.FromArrayToTail(&array[0], num)        Places num nodes at tail of list


   I've attempted to make this as user-friendly as possible.

   All the methods in these classes have comments detailing what they do.
   The comments are in a format that MS Visual C++ 6 likes, and therefore
   will appear as automatic help when attempting to use these classes.  This
   is a wonderful feature of MSVC6, and I just wish it would always show the
   correct comment.  I've never figured out what makes it get screwed up.
   
   Also, the variable names in the prototypes are intended to be meaningful,
   as, again, MSVC6 will display these automagically when attempts to use
   this class are made.

*/


class CPPListNode
{
public:
  CPPListNode *Next;
  CPPListNode *Prev;
//  unsigned long Type;

  CPPListNode() : Next(0), Prev(0) {};
  virtual ~CPPListNode() {};

  // Gets the previous node, or NULL.
  CPPListNode *GetPrev()
  {
    if (Prev->Prev)
      return Prev;
    else
      return 0;
  }

  // Gets the previous node.  This may be the list's head sentinel.
  CPPListNode *GetPrevQuick()
  {
    return Prev;
  }

  // Gets the next node, or NULL.
  CPPListNode *GetNext()
  {
    if (Next->Next)
      return Next;
    else
      return 0;
  }

  // Gets the next node.  This may be the list's tail sentinel.
  CPPListNode *GetNextQuick()
  {
    return Next;
  }

  // Removes this node from whatever list it's in.  It *must* be in
  // a list before attempting this call.  Returns the removed node.
  CPPListNode *Remove()
  {
    Prev->Next = Next;
    Next->Prev = Prev;
    return this;
  }

  // Adds this node before the specified node, in whatever list it's in.
  // Returns the added node.
  CPPListNode *AddBefore(CPPListNode *nodeToPrecede)
  {
    Next = nodeToPrecede;
    Prev = nodeToPrecede->Prev;
    Prev->Next = this;
    Next->Prev = this;
    return this;
  }

  // Adds this node after the specified node, in whatever list it's in.
  // Returns the added node.
  CPPListNode *AddAfter(CPPListNode *nodeToFollow)
  {
    Prev = nodeToFollow;
    Next = nodeToFollow->Next;
    Next->Prev = this;
    Prev->Next = this;
    return this;
  }

  // Moves this node from whatever list it's in to precede the specified
  // node in whatever list that node is in.  The node *must* be in a list
  // already.  Returns the added node.
  CPPListNode *MoveBefore(CPPListNode *nodeToPrecede)
  {
    Prev->Next = Next;
    Next->Prev = Prev;

    Next = nodeToPrecede;
    Prev = nodeToPrecede->Prev;
    Prev->Next = this;
    Next->Prev = this;
    return this;
  }

  // Moves this node from whatever list it's in to follow the specified
  // node in whatever list that node is in.  The node *must* be in a list
  // already.  Returns the added node.
  CPPListNode *MoveAfter(CPPListNode *nodeToFollow)
  {
    Prev->Next = Next;
    Next->Prev = Prev;

    Prev = nodeToFollow;
    Next = nodeToFollow->Next;
    Next->Prev = this;
    Prev->Next = this;
    return this;
  }

  // Links two node structures together, joining next and prev's.   This
  // is just here for completeness, as anyone should know how to do it
  // themselves.  Returns the first node.
  CPPListNode *LinkNodes(CPPListNode *nodeToAttach)
  {
    Next = nodeToAttach;
    nodeToAttach->Prev = this;
    return this;
  }

  // Removes all nodes between this node and the specified end node from
  // their current list.  Both nodes must be in the same list and the
  // specified node must come later.  Returns the first removed node.
  CPPListNode *BulkRemove(CPPListNode *endNode)
  {
    Prev->Next = endNode->Next;
    endNode->Next->Prev = Prev;
    return this;
  }

  // Adds the nodes between the current node and the specified endNode before
  // the specified node in a list.  Returns the first added node.
  CPPListNode *BulkAddBefore(CPPListNode *nodeToPrecede, CPPListNode *endNode)
  {
    endNode->Next = nodeToPrecede;
    Prev = nodeToPrecede->Prev;
    nodeToPrecede->Prev->Next = this;
    nodeToPrecede->Prev = endNode;
    return this;
  }

  // Adds the nodes between the current node and the specified endNode after
  // the specified node in a list.  Returns the first added node.
  CPPListNode *BulkAddAfter(CPPListNode *nodeToFollow, CPPListNode *endNode)
  {
    endNode->Next = nodeToFollow->Next;
    Prev = nodeToFollow;
    nodeToFollow->Next->Prev = endNode;
    nodeToFollow->Next = this;
    return this;
  }

  // Moves the nodes between the current node and the specified endNode before
  // the specified node in a list.  Returns the first moved node.
  CPPListNode *BulkMoveBefore(CPPListNode *nodeToPrecede, CPPListNode *endNode)
  {
    Prev->Next = endNode->Next;
    endNode->Next->Prev = Prev;

    endNode->Next = nodeToPrecede;
    Prev = nodeToPrecede->Prev;
    nodeToPrecede->Prev->Next = this;
    nodeToPrecede->Prev = endNode;
    return this;
  }

  // Moves the nodes between the current node and the specified endNode after
  // the specified node in a list.  Returns the first moved node.
  CPPListNode *BulkMoveAfter(CPPListNode *nodeToFollow, CPPListNode *endNode)
  {
    Prev->Next = endNode->Next;
    endNode->Next->Prev = Prev;

    endNode->Next = nodeToFollow->Next;
    Prev = nodeToFollow;
    nodeToFollow->Next->Prev = endNode;
    nodeToFollow->Next = this;
    return this;
  }

  virtual bool operator <=(CPPListNode &pNode)
  {
    // Just sort by address for basic functionality.
    // Override this to do what you want (what you really, really want).
    return this <= &pNode;
  }
};

// ----====================--------====================--------====================--------====================--------====================----

class CPPListHead
{
public:
  CPPListNode HeadNode;   // List's sentinel head node
  CPPListNode TailNode;   // List's sentinel tail node

  // Construct new list
  CPPListHead()
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
  ~CPPListHead() {};

  // Get head node of the list.  NULL if empty
  CPPListNode *GetHead() 
  {
    if (HeadNode.Next == &TailNode)
      return 0;
    else
      return HeadNode.Next;
  }

  // Get head node of the list.  This may be the tail sentinel.
  CPPListNode *GetHeadQuick() 
  {
    return HeadNode.Next;
  }

  // Get the tail node of the list.  NULL if empty
  CPPListNode *GetTail()
  {
    if (TailNode.Prev == &HeadNode)
      return 0;
    else
      return TailNode.Prev;
  }

  // Get the tail node of the list.  This may be the head sentinel
  CPPListNode *GetTailQuick()
  {
    return TailNode.Prev;
  }

  // Returns TRUE if the list is empty.
  char IsEmpty()
  {
    return HeadNode.Next == &TailNode;
  }
  
  // Returns TRUE if the list is not empty.
  char NotEmpty()
  {
    return HeadNode.Next != &TailNode;
  }

  // Remove and return the head of the list.  Do not call on an empty list!
  CPPListNode *RemHeadQuick() 
  {
    CPPListNode *retval;

    retval = HeadNode.Next;
    HeadNode.Next->Next->Prev = &HeadNode;
    HeadNode.Next = HeadNode.Next->Next;
    return retval;
  }

  // Remove and return the head of the list.  Returns NULL on empty list.
  CPPListNode *RemHead()
  {
    if (IsEmpty())
      return 0;
    else
      return RemHeadQuick();
  }

  // Remove and retrun the tail of the list.  Do not call on an empty list!
  CPPListNode *RemTailQuick()
  {
    CPPListNode *retval;

    retval = TailNode.Prev;
    TailNode.Prev->Prev->Next = &TailNode;
    TailNode.Prev = TailNode.Prev->Prev;
    return retval;
  }

  // Remove and return the tail of the list.  Returns NULL on empty list.
  CPPListNode *RemTail()
  {
    if (IsEmpty())
      return 0;
    else
      return RemTailQuick();
  }

  // Adds the given node to the head of the list.  Returns the added node.
  // If the node is already in a list, use MoveHead()
  CPPListNode *AddHead(CPPListNode *newNode)
  {
    return newNode->AddAfter(&HeadNode);
  }

  // Adds the given node to the tail of the list.  Returns the added node.
  // If the node is already in a list, use MoveTail()
  CPPListNode *AddTail(CPPListNode *newNode)
  {
    return newNode->AddBefore(&TailNode);
  }

  // Moves the specified node to the head of the list.  The node must already
  // be in a list, though not necessarily this one.  If the node is not in a
  // list, use AddHead() instead.
  // Returns the moved node.
  CPPListNode *MoveHead(CPPListNode *node)
  {
    return node->MoveAfter(&HeadNode);
  }

  // Moves the specified node to the tail of the list.  The node must already
  // be in a list, though not necessarily this one.  If the node is not in a
  // list, use AddTail() instead.
  // Returns the moved node.
  CPPListNode *MoveTail(CPPListNode *node)
  {
    return node->MoveBefore(&TailNode);
  }

  // Adds all nodes between first and last to the head of the list.  This requires
  // that nodes first and last are attached, and no nodes between them are in
  // a list.  Returns the first added node.
  CPPListNode *BulkAddHead(CPPListNode *firstNode, CPPListNode *lastNode)
  {
    return firstNode->BulkAddAfter(&HeadNode, lastNode);
  }

  // Adds all nodes between first and last to the tail of the list.  This requires
  // that nodes first and last are attached, and no nodes between them are in
  // a list.  Returns the first added node.
  CPPListNode *BulkAddTail(CPPListNode *firstNode, CPPListNode *lastNode)
  {
    return firstNode->BulkAddBefore(&TailNode, lastNode);
  }

  // Moves all nodes between first and last to the head of the list.  This requires
  // that nodes first and last are attached, and no nodes between them are in
  // a list.  Returns the first moved node.
  CPPListNode *BulkMoveHead(CPPListNode *firstNode, CPPListNode *lastNode)
  {
    return firstNode->BulkMoveAfter(&HeadNode, lastNode);
  }

  // Moves all nodes between first and last to the tail of the list.  This requires
  // that nodes first and last are attached, and no nodes between them are in
  // a list.  Returns the first moved node.
  CPPListNode *BulkMoveTail(CPPListNode *firstNode, CPPListNode *lastNode)
  {
    return firstNode->BulkMoveBefore(&TailNode, lastNode);
  }

  // Appends the specified list onto the end of this list.  This call is safe for empty lists.
  // Returns the first added node, or NULL on an empty list.
  CPPListNode *Append(CPPListHead *listToAppend)
  {
    if (listToAppend->NotEmpty())
      return BulkMoveTail(listToAppend->HeadNode.Next, listToAppend->TailNode.Prev);
    return 0;
  }

  // Prepends the specified list onto the beginning of list list.  This call is safe for empty lists.
  // Returns the first added node or NULL on an empty list.
  CPPListNode *Prepend(CPPListHead *listToPrepend)
  {
    if (listToPrepend->NotEmpty())
      return BulkMoveHead(listToPrepend->HeadNode.Next, listToPrepend->TailNode.Prev);
    return 0;
  }

  // Merge sort this list using the specified compare function.
  // The compare function should be of type
  // signed char CmpFunc(CPPListNode *a, CPPListNode *b);
  // Where the return is <0 if p is smaller than q, ==0 is p is equal to q, or
  // >0 if q is smaller than p.
  // If it makes life easier p<q and p==q are the same thing to this call.
  void Sort(signed char (* sortFunc)(CPPListNode *a, CPPListNode *b))
  {
    if (IsEmpty())
      return;
    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    // Apparently O(n log n), which is good.
    // Merge sort has issues on arrays (extra storage requirements) which are
    //  not an issue with list based implementations.  This is supposed to be
    //  a really good list sort algorithm.
    // This could be made faster by only maintaining the next pointers, and
    //  going back and filling in the prev pointers when the sort is complete.
    CPPListHead sortedList;
    long k = 1;
    long numMerges;
    CPPListNode *p, *q, *tmp;
    long psize, qsize;

    do
    {
      p = HeadNode.Next;
      numMerges = 0;
      while (p->Next)
      {
        numMerges++;
        q = p;
        psize = 0;
        while (psize < k)
        {
          psize++;
          q = q->Next;
          if (!q->Next)
            break;
        }
        qsize = k;
        // Merge phase
        if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
        {
          while (qsize && psize)
          {
            if (sortFunc(p, q) <= 0)
            {
              tmp = p->Next;
              sortedList.AddTail(p);
              psize--;
              p = tmp;
            }
            else
            {
              tmp = q->Next;
              sortedList.AddTail(q);
              qsize--;
              q = tmp;
              if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
                break;
            }
          }
        }
        while (psize--)
        {
          tmp = p->Next;
          sortedList.AddTail(p);
          p = tmp;
        }
        while (qsize-- && q->Next)
        {
          tmp = q->Next;
          sortedList.AddTail(q);
          q = tmp;
        }
        p = q;
      }
      k <<= 1;
      HeadNode.Next = sortedList.HeadNode.Next;
      TailNode.Prev = sortedList.TailNode.Prev;
      HeadNode.Next->Prev = &HeadNode;
      TailNode.Prev->Next = &TailNode;
      sortedList.NewList();
    } while (numMerges > 1);
  }

  // Merge sort this list using the <= operator or the nodes.
  void Sort()
  {
    if (IsEmpty())
      return;
    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    // Apparently O(n log n), which is good.
    // Merge sort has issues on arrays (extra storage requirements) which are
    //  not an issue with list based implementations.  This is supposed to be
    //  a really good list sort algorithm.
    // This could be made faster by only maintaining the next pointers, and
    //  going back and filling in the prev pointers when the sort is complete.
    CPPListHead sortedList;
    long k = 1;
    long numMerges;
    CPPListNode *p, *q, *tmp;
    long psize, qsize;

    do
    {
      p = HeadNode.Next;
      numMerges = 0;
      while (p->Next)
      {
        numMerges++;
        q = p;
        psize = 0;
        while (psize < k)
        {
          psize++;
          q = q->Next;
          if (!q->Next)
            break;
        }
        qsize = k;
        // Merge phase
        if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
        {
          while (qsize && psize)
          {
            if ((*p) <= (*q))
            {
              tmp = p->Next;
              sortedList.AddTail(p);
              psize--;
              p = tmp;
            }
            else
            {
              tmp = q->Next;
              sortedList.AddTail(q);
              qsize--;
              q = tmp;
              if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
                break;
            }
          }
        }
        while (psize--)
        {
          tmp = p->Next;
          sortedList.AddTail(p);
          p = tmp;
        }
        while (qsize-- && q->Next)
        {
          tmp = q->Next;
          sortedList.AddTail(q);
          q = tmp;
        }
        p = q;
      }
      k <<= 1;
      HeadNode.Next = sortedList.HeadNode.Next;
      TailNode.Prev = sortedList.TailNode.Prev;
      HeadNode.Next->Prev = &HeadNode;
      TailNode.Prev->Next = &TailNode;
      sortedList.NewList();
    } while (numMerges > 1);
  }

  // Merge sort this list using the long at the given offset as the sort field
  void SortLong(long offset)
  {
    if (IsEmpty())
      return;
    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    // Apparently O(n log n), which is good.
    // Merge sort has issues on arrays (extra storage requirements) which are
    //  not an issue with list based implementations.  This is supposed to be
    //  a really good list sort algorithm.
    // This could be made faster by only maintaining the next pointers, and
    //  going back and filling in the prev pointers when the sort is complete.
    CPPListHead sortedList;
    long k = 1;
    long numMerges;
    CPPListNode *p, *q, *tmp;
    long psize, qsize;

    do
    {
      p = HeadNode.Next;
      numMerges = 0;
      while (p->Next)
      {
        numMerges++;
        q = p;
        psize = 0;
        while (psize < k)
        {
          psize++;
          q = q->Next;
          if (!q->Next)
            break;
        }
        qsize = k;
        // Merge phase
        if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
        {
          while (qsize && psize)
          {
            if ( *((long *)(&((char *)p)[offset])) <= *((long *)(&((char *)q)[offset])) )
            {
              tmp = p->Next;
              sortedList.AddTail(p);
              psize--;
              p = tmp;
            }
            else
            {
              tmp = q->Next;
              sortedList.AddTail(q);
              qsize--;
              q = tmp;
              if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
                break;
            }
          }
        }
        while (psize--)
        {
          tmp = p->Next;
          sortedList.AddTail(p);
          p = tmp;
        }
        while (qsize-- && q->Next)
        {
          tmp = q->Next;
          sortedList.AddTail(q);
          q = tmp;
        }
        p = q;
      }
      k <<= 1;
      HeadNode.Next = sortedList.HeadNode.Next;
      TailNode.Prev = sortedList.TailNode.Prev;
      HeadNode.Next->Prev = &HeadNode;
      TailNode.Prev->Next = &TailNode;
      sortedList.NewList();
    } while (numMerges > 1);
  }

  // Merge sort this list using the long at the given offset as the sort field
  void SortFloat(long offset)
  {
    if (IsEmpty())
      return;
    // based on http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
    // Apparently O(n log n), which is good.
    // Merge sort has issues on arrays (extra storage requirements) which are
    //  not an issue with list based implementations.  This is supposed to be
    //  a really good list sort algorithm.
    // This could be made faster by only maintaining the next pointers, and
    //  going back and filling in the prev pointers when the sort is complete.
    CPPListHead sortedList;
    long k = 1;
    long numMerges;
    CPPListNode *p, *q, *tmp;
    long psize, qsize;

    do
    {
      p = HeadNode.Next;
      numMerges = 0;
      while (p->Next)
      {
        numMerges++;
        q = p;
        psize = 0;
        while (psize < k)
        {
          psize++;
          q = q->Next;
          if (!q->Next)
            break;
        }
        qsize = k;
        // Merge phase
        if (q->Next)  // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
        {
          while (qsize && psize)
          {
            if ( *((float *)(&((char *)p)[offset])) <= *((float *)(&((char *)q)[offset])) )
            {
              tmp = p->Next;
              sortedList.AddTail(p);
              psize--;
              p = tmp;
            }
            else
            {
              tmp = q->Next;
              sortedList.AddTail(q);
              qsize--;
              q = tmp;
              if (!q->Next) // Since q doesn't change every loop iteration, I've broken this out of the main loop condition
                break;
            }
          }
        }
        while (psize--)
        {
          tmp = p->Next;
          sortedList.AddTail(p);
          p = tmp;
        }
        while (qsize-- && q->Next)
        {
          tmp = q->Next;
          sortedList.AddTail(q);
          q = tmp;
        }
        p = q;
      }
      k <<= 1;
      HeadNode.Next = sortedList.HeadNode.Next;
      TailNode.Prev = sortedList.TailNode.Prev;
      HeadNode.Next->Prev = &HeadNode;
      TailNode.Prev->Next = &TailNode;
      sortedList.NewList();
    } while (numMerges > 1);
  }

  // Convert the list to an array of node pointers.
  // Returns the number of used entries from 0 to maxNodes.
  // The entries are optionally removed from the list.  If you choose to
  //  removed entries, and more entries are in the list than the array, the
  //  remaining entries will be left in the list.
  long ToArray(CPPListNode **storage, long maxNodes, char removeNodes = 0)
  {
    CPPListNode *ptr;
    long numNodes = 0;

    for (ptr = HeadNode.Next; ptr->Next && numNodes != maxNodes; ptr = ptr->Next)
    {
      *storage++ = ptr;
      numNodes++;
    }
    if (removeNodes)
    {
      HeadNode.Next = ptr;
      ptr->Prev = &HeadNode;
    }
    return numNodes;
  }

  // Converts an array of node pointers to a set of linked nodes at the head
  // of the list.
  void FromArrayToHead(CPPListNode **storage, long numNodes)
  {
    CPPListNode *firstNode = HeadNode.Next;
    CPPListNode *prevNode = &HeadNode;

    while(numNodes--)
    {
      prevNode->Next = *storage;
      (*storage)->Prev = prevNode;
      prevNode = *storage++;
    }
    prevNode->Next = firstNode;
    firstNode->Prev = prevNode;
  }

  // Converts an array of node pointers to a set of linked nodes at the tail
  // of the list.
  void FromArrayToTail(CPPListNode **storage, long numNodes)
  {
    CPPListNode *lastNode = TailNode.Prev;
    CPPListNode *nextNode = &TailNode;

    storage += numNodes-1;
    while(numNodes--)
    {
      nextNode->Prev = *storage;
      (*storage)->Next = nextNode;
      nextNode = *storage--;
    }
    nextNode->Prev = lastNode;
    lastNode->Next = nextNode;
  }

  // Specialized alist v2.0 features that we don't require.
  // These would require a subclassed type that had some field to enqueue on
  //  or would require an enqueueing function call (slow!)
    //AddEnqueue
    //MoveEnqueue
};

// ----====================--------====================--------====================--------====================--------====================----

#if 0
// TestCode

#include <stdlib.h>
#include <stdio.h>

#include "utilcode\cpplist.h"

CPPListHead ListOne;
CPPListHead ListTwo;

class DataNode : public CPPListNode
{
public:
  unsigned long num;
};

DataNode NodeOne;
DataNode NodeTwo;
DataNode NodeThree;
DataNode NodeFour;
DataNode NodeFive;

void DumpList(CPPListHead *l, char *title)
{
	DataNode *dn;

	printf ("%s\r\n< ",title);

	if (l->IsEmpty())
	{
		printf ("EMPTY >\r\n\n");
		return;
	}

	for (dn=(DataNode *)l->HeadNode.Next; dn->Next; dn=(DataNode *)dn->Next)
	{
		printf ("%d ",dn->num);
	}
	printf (">\r\n\n");
}

signed char sortFunc(CPPListNode *a, CPPListNode *b)
{
  DataNode *aa=(DataNode *)a, *bb=(DataNode *)b;

  return (signed char)(aa->num - bb->num);
}

void main(void)
{
  long x;

  /* Setup our test data.  The two lists are initiailized by constructor already. */
  NodeOne.num = 1;
  NodeTwo.num = 2;
  NodeThree.num = 3;
  NodeFour.num = 4;
  NodeFive.num = 5;

  ListOne.AddHead(&NodeTwo);
  ListOne.AddHead(&NodeOne);
  NodeThree.AddAfter(&NodeOne);
  NodeFour.LinkNodes(&NodeFive);
  ListOne.BulkAddTail(&NodeFour, &NodeFive);
  DumpList(&ListOne, "Swapped two and three");
  DumpList(&ListTwo, "Empty list");

  NodeThree.MoveAfter(&NodeTwo);
	DumpList(&ListOne, "In order");

  ListOne.RemHead();
  DumpList(&ListOne, "Missing one");

  ListOne.RemTail();
	DumpList(&ListOne, "Missing five too");

  NodeOne.LinkNodes(&NodeFive);
  ListTwo.BulkAddHead(&NodeOne, &NodeFive);
	DumpList(&ListTwo, "Just one and five");

  ListOne.Append(&ListTwo);
	DumpList(&ListTwo, "Empty list");
	DumpList(&ListOne, "Two through Four, One, and Five");

  ListOne.BulkMoveHead(&NodeOne, &NodeFive);
	DumpList(&ListOne, "One, and Five, Two through Four");

  ListOne.BulkMoveTail(&NodeOne, &NodeFive);
	DumpList(&ListOne, "Two through Four, One, and Five, again");

  NodeOne.BulkMoveAfter(&NodeTwo, &NodeFive);
	DumpList(&ListOne, "Two, one, five, three, four");

  NodeTwo.BulkMoveBefore(&NodeThree, &NodeOne);
	DumpList(&ListOne, "five, Two, one, three, four");

  NodeTwo.BulkRemove(&NodeThree);
	DumpList(&ListOne, "five, four");

  NodeFour.Remove();
	DumpList(&ListOne, "just five");

  ListOne.AddTail(&NodeThree);
	DumpList(&ListOne, "five, three");

  NodeFour.AddBefore(&NodeThree);
	DumpList(&ListOne, "five, four, three");

  NodeFour.MoveBefore(&NodeFive);
	DumpList(&ListOne, "four, five, three");

  ListOne.MoveHead(&NodeThree);
	DumpList(&ListOne, "three, four, five");

  ListOne.MoveTail(&NodeFour);
	DumpList(&ListOne, "three, five, four");

  NodeOne.LinkNodes(&NodeTwo);
  NodeOne.BulkAddBefore(&NodeFive, &NodeTwo);
	DumpList(&ListOne, "three, one, two, five, four");

  NodeThree.Remove();
  NodeTwo.Remove();
  NodeFour.Remove();
  NodeTwo.LinkNodes(&NodeThree);
  NodeThree.LinkNodes(&NodeFour);
  NodeTwo.BulkAddAfter(&NodeOne, &NodeFour);
	DumpList(&ListOne, "In order");

  NodeOne.Remove();
  NodeThree.Remove();
  ListTwo.AddHead(&NodeThree);
  ListTwo.AddHead(&NodeOne);
  ListOne.Prepend(&ListTwo);
	DumpList(&ListOne, "one, three, two, four, five");

  ListOne.Sort(sortFunc);
  DumpList(&ListOne, "sorted");

  x = ListOne.ToArray((CPPListNode **)&nodes[0], 3, 1);
  ListTwo.FromArrayToHead((CPPListNode **)&nodes[0], x);
  DumpList(&ListOne, "Four Five");
  DumpList(&ListTwo, "One Two Three");
  x = ListOne.ToArray((CPPListNode **)&nodes[0], 5, 1);
  ListTwo.FromArrayToHead((CPPListNode **)&nodes[0], x);
  DumpList(&ListOne, "Empty");
  DumpList(&ListTwo, "Four Five One Two Three");
  x = ListTwo.ToArray((CPPListNode **)&nodes[0], 2, 1);
  ListTwo.FromArrayToTail((CPPListNode **)&nodes[0], x);
  DumpList(&ListTwo, "One Two Three Four Five");
}
#endif

#endif

