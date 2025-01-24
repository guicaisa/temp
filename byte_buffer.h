#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include <vector>
#include "define.h"

class ByteBuffer
{
public: 
	//constructor
	ByteBuffer() : wpos_(0), rpos_(0), bytes_()
	{
		bytes_.resize(4096);
	}

	explicit ByteBuffer(size_t size) : wpos_(0), rpos_(0), bytes_()
	{
		bytes_.resize(size);
	}

public:
	uint8* BasePtr() { return bytes_.data(); }

	uint8* ReadPtr() { return BasePtr() + rpos_; }
	
	uint8* WritePtr() { return BasePtr() + wpos_; }
	
	void AddReadNum(size_t num) { rpos_ += num; }
	
	void AddWriteNum(size_t num) { wpos_ += num; }
	
	size_t ReadableSize() { return wpos_ - rpos_; }
	
	size_t RemainAvailableSize() { return bytes_.size() - wpos_; }
	
	size_t BufferSize() { return bytes_.size(); }

	void Reset()
	{
		wpos_ = 0;
		rpos_ = 0;
	}

	void Resize(size_t size)
	{
		bytes_.resize(size);
	}

	//重新指向读写位置，丢弃已读的数据
	void Redirect()
	{
		if (rpos_ == 0)
		{
			return;
		}
		if (rpos_ != wpos_)
		{
			memmove(BasePtr(), ReadPtr(), ReadableSize());
		}
		wpos_ -= rpos_;
		rpos_ = 0;
	}

private:
	size_t wpos_;
	size_t rpos_;
	std::vector<uint8> bytes_;
};


#endif // BYTE_BUFFER_H