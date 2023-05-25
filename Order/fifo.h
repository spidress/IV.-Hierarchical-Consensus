#ifndef FIFO_H
#define FIFO_H
#include <iostream>
using namespace std;

template<class T> 
class Fifo
{
public:
	Fifo(int size)
	{
		m_pdata = new T[size];
		m_size = size;
		m_ptr_wr = 0;
		m_ptr_rd = 0;
	}
	~Fifo() {delete []m_pdata;}
	bool is_empty() {return (m_ptr_wr == m_ptr_rd);}
	bool is_full() {return (((m_ptr_wr + 1) % m_size) == m_ptr_rd);}
	bool write(T pdata)
	{
		if(is_full())
		{
			return false;
		}
		m_ptr_wr = (m_ptr_wr + 1) % m_size;
		m_pdata[m_ptr_wr] = pdata;
		return true;
	}

	bool read(T *pdata)
	{
		if(is_empty())
		{
			return false;
		}
		m_ptr_rd = (m_ptr_rd + 1) % m_size;
		*pdata = m_pdata[m_ptr_rd];
		return true;
	}
	int get_size() 
	{
		return (m_ptr_wr >= m_ptr_rd ? m_ptr_wr - m_ptr_rd : m_size + m_ptr_wr - m_ptr_rd);
	}
private:
    T *m_pdata;
    int m_ptr_wr;
    int m_ptr_rd;
    int m_size;
};
#endif // FIFO_H