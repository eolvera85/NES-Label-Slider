#ifndef ListDynamic_h
#define ListDynamic_h

template <typename T>
struct node
{
    T data;
    node *next;
};

template <class T>
class List
{
  private:
    node<T> *head, *tail;
  public:
    List();
    ~List();

    void Add(T value);
    T Value(int index);
    int Count();
};

template <class T>
List<T>::List()
{
  head = NULL;
  tail = NULL;
}

template <class T>
List<T>::~List()
{
    node<T> *current;
    node<T> *previous;
    current = head;
    
    while (current != NULL)
    {
        previous = current;
        current = current->next;
        delete previous;
    }
    
    current = NULL;
    previous = NULL;
    head = NULL;
    tail = NULL;
}

template <class T>
void List<T>::Add(T value)
{
    node<T> *temp = new node<T>;
    temp->data = value;
    temp->next = NULL;
    
    if(head == NULL)
    {
        head = temp;
        tail = temp;
        temp = NULL;
    }
    else
    {
        tail->next = temp;
        tail = temp;
    }
}

template <class T>
T List<T>::Value(int index)
{
    node<T> *temp = new node<T>;
    temp = head;
    
    for(int i = 0; i < index; i++)
        temp = temp->next;
    
    return temp->data;
}

template <class T>
int List<T>::Count()
{
    node<T> *current;
    current = head;
    int count = 0;
    
    while(current != NULL)
    {
        count++;
        current = current->next;
    }
    
    current = NULL;
    return count;
}


#endif /* ListDynamic_h */