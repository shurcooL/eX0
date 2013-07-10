template <typename T, typename Tp> class IndexedCircularBuffer
{
public:
	IndexedCircularBuffer(void);
	~IndexedCircularBuffer(void);

	bool push(T & oItem, Tp nPosition);
	void pop(void);

	T & operator [](Tp nPosition);

	int begin(void);
	int end(void);

	T & front(void);
	T & back(void);

	bool empty(void);
	//bool full(void);
	u_int size(void);

	void clear(void);

private:
	T		*m_pBuffer;
	const u_int		m_knBufferSize;
	Tp		m_nBufferHead;
	Tp		m_nBufferTail;

	u_int capacity(void);
};

#include "IndexedCircularBuffer.cpp"
