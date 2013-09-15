#ifndef LIBCOBALT_BYTESTREAM_H
#define LIBCOBALT_BYTESTREAM_H

#include "main.h"
#include "types.h"

class CoString;

// TODO: Make able to handle big-endian too
class CoBytestream {
public:
	enum StringType {
		NullTerminated,
		LengthPrefixed,
	};
	
	CoBytestream (ulong len = 2048);
	CoBytestream (const char* data, ulong len);
	CoBytestream (const initlist<uint8>& bytes);
	
	void init (const char* data, ulong len);
	void rewind();
	void seek (ulong pos);
	void clear();
	void merge (const CoBytestream& other);
	bool tryMerge (const CoBytestream& other) attr (warn_unused_result);
	void resize (ulong len);
	ulong bytesLeft() const;
	ulong spaceLeft() const;
	void growToFit (ulong bytes);
	const uint8* data() const;
	StringType stringType() const;
	void setStringType (StringType type);
	
	inline size_t length() const { return m_len; }
	
	bool readBytes (uint8 numbytes, uint8* val);
	bool readByte (int8& val);
	bool readByte (uint8& val);
	bool readShort (int16& val);
	bool readShort (uint16& val);
	bool readLong (int32& val);
	bool readLong (uint32& val);
	bool readQuad (int64& val);
	bool readQuad (uint64& val);
	bool readString (CoString& val);
	bool readFloat (float& val);
	bool readDouble (double& val);
	
	void writeBytes (uint8 numbytes, const uint8* val);
	void writeByte (uint8 val);
	void writeShort (int16 val);
	void writeLong (int32 val);
	void writeQuad (int64 val);
	void writeFloat (float val);
	void writeDouble (double val);
	void writeString (const str& val);
	
	CoBytestream& operator<< (const CoBytestream& other) {
		merge (other);
		return *this;
	}
	
	uint8& subscript (ulong idx);
	const uint8& const_subscript (ulong idx) const;
	
	uint8& operator[] (ulong idx) {
		return subscript (idx);
	}
	
	const uint8& operator[] (ulong idx) const {
		return const_subscript (idx);
	}
	
private:
	uint8*     m_data;
	uint8*     m_ptr;
	ulong      m_size,
	           m_len;
	StringType m_stringType;
	
	void doWrite (uint8 val);
};

#endif // LIBCOBALT_BYTESTREAM_H
