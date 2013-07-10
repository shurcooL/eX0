template <typename T> class IndexedCircularBuffer
{
public:
	IndexedCircularBuffer(int nBufferSize);
	~IndexedCircularBuffer(void);

	bool push(T & oItem, int nPosition);
	void pop(void);

	T & operator [](int nPosition);

	int begin(void);
	int end(void);

	T & front(void);
	T & back(void);

	bool empty(void);
	//bool full(void);
	int size(void);

	void clear(void);

private:
	T	*m_pBuffer;
	int	m_nBufferSize;
	int	m_nBufferHead;
	int	m_nBufferTail;

	int capacity(void);
};

#include "IndexedCircularBuffer.cpp"
