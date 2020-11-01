#ifndef _NETLIB_STRAIGHTBUFFER_H
#define _NETLIB_STRAIGHTBUFFER_H

/*********************************************
* Flat buffer to store data that comes/goes from a socket
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
class StraightBuffer
{
	char * m_buffer;
	size_t space;
	size_t written;

public:
	StraightBuffer()
	{
        space = written = 0;
		m_buffer = 0;
	}
	~StraightBuffer()
	{
		if( m_buffer )
		{
			free(m_buffer);
			m_buffer = NULL;
		}
	}

	/** Read bytes from the buffer
	 * @param destination pointer to destination where bytes will be written
	 * @param bytes number of bytes to read
	 * @return true if there was enough data, false otherwise
	 */
	bool Read(void * destination, size_t bytes)
	{
//printf("!!!!! reading %u from buffer, have %u  max(%u)\n",bytes,written,space);
		if(written <= bytes)
		{
			// copy what we have 
//			ASSERT( written >= bytes );	//maybe the packet header was corrupted and we try to read more then what we have ?
			memcpy(destination, m_buffer, written);
			written = 0;
			return false;
		}
		else
		{
			/* read everything */
			memcpy(destination, m_buffer, bytes);
			written -= bytes;

			/* move the buffer "backwards" */
			memcpy(&m_buffer[0], &m_buffer[bytes], written);
			return true;
		}
	}

	/** Write bytes to the buffer
	 * @param data pointer to the data to be written
	 * @param bytes number of bytes to be written
	 * @return true if was successful, otherwise false
	 */
	bool Write(const void * data, size_t bytes)
	{
//printf("!!!!! writing %u into buffer have %u max(%u)\n",bytes,written,space);
		if((written + bytes) >= space)
		{
			// write what we can 
/*			bytes = space - written;
			if(bytes)
				memcpy(&m_buffer[written], data, bytes);
			written = space; */
			// could not write packet. Notify owner that buffer is too small
			// this will happen a lot for players with lag
//			ASSERT( false );
			return false;
		}
		else
		{
			// write everything 
			memcpy(&m_buffer[written], data, bytes);
			written += bytes;
			return true;
		}
	}

	/** Returns the number of available bytes left.
	 */
	size_t GetSpace()
	{
		return (space - written);
	}

	/** Returns the number of bytes currently stored in the buffer.
	 */
	size_t GetSize()
	{
		return written;
	}

	/** Removes len bytes from the front of the buffer
	 * @param len the number of bytes to "cut"
	 */
	void Remove(size_t len)
	{
//printf("!!!! removing %u from %u max %u\n",len,written,space);
//		ASSERT( len == 0 || written >= len );
		//how on earth ?
		if( len >= written )
		{
			written = 0;
		}
		else
		{
			written -= len;
			if(written)
				memcpy(&m_buffer[0], &m_buffer[len], written);
		}
	} 

	/** Returns a pointer at the "end" of the buffer, where new data can be written
	 */
	void * GetBuffer()
	{
		return &m_buffer[written];
	}

	/**we can always start reading from start of the buffer. Maybe later implement read /write indexes to avoid so many copy ?
	*/
	void * GetBufferStart()
	{
		return m_buffer;
	}

	/** Allocate the buffer with room for size bytes
	 * @param size the number of bytes to allocate
	 */
	void Allocate(size_t size)
	{
		m_buffer = (char*)malloc(size);
		space = size;
	}

	/** Increments the "writen" pointer forward len bytes
	 * @param len number of bytes to step
	 */
	void IncrementWritten(size_t len)
	{
		//ignore acknowleging that we recived anything in buffer if it would not fit inside of it
		//this is practically a memory corruption somewhere
		if( written + len < space )
			written += len;
		//this will overwrite the packet sent before us. Should never happen
		else if( len < space )
		{
			written = space - len;
//			ASSERT( 0 );
		}
		else
		{
			written = 0;	//corrupt as less memory as possible
//			ASSERT( 0 );
		}
	}
};

#endif		// _NETLIB_STRAIGHTBUFFER_H
#endif