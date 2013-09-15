#include "bytestream.h"
#include "format.h"
#include "misc.h"
#include <string.h>

CoBytestream::CoBytestream (ulong len) :
	m_data (null),
	m_stringType (NullTerminated)
{
	resize (len);
	clear();
}

CoBytestream::CoBytestream (const char* data, ulong len) :
	m_data (null),
	m_stringType (NullTerminated)
{
	m_data = null;
	init (data, len);
}

CoBytestream::CoBytestream (const initlist<uint8>& bytes) :
	m_data (null),
	m_stringType (NullTerminated)
{
	char* data = new char[bytes.size()];
	char* p = &data[0];
	
	for (uint8 byte : bytes)
		*p++ = byte;
	
	init (data, bytes.size());
	delete[] data;
}

void CoBytestream::resize (ulong newsize) {
	char* olddata = null;
	ulong oldsize;
	
	if (m_data) {
		oldsize = m_size;
		olddata = new char[oldsize];
		memcpy (olddata, m_data, oldsize);
	}
	
	delete[] m_data;
	m_data = new uint8[newsize];
	m_size = newsize;
	
	if (olddata)
		memcpy (m_data, olddata, min<ulong> (oldsize, newsize));
}

void CoBytestream::init (const char* data, ulong len) {
	resize (len);
	memcpy (m_data, data, len);
	m_ptr = &m_data[0];
	m_len = len;
}

void CoBytestream::clear() {
	m_ptr = &m_data[0];
	m_len = 0;
}

uint8& CoBytestream::subscript (ulong idx) {
	return m_data[idx];
}

const uint8& CoBytestream::const_subscript (ulong idx) const {
	return m_data[idx];
}

void CoBytestream::seek (ulong pos) {
	m_ptr = m_data + pos;
}

// =============================================================================
void CoBytestream::rewind() {
	m_ptr = m_data;
}

ulong CoBytestream::bytesLeft() const {
	return (m_len - (m_ptr - &m_data[0]));
}

ulong CoBytestream::spaceLeft() const {
	return (m_size - m_len);
}

// =============================================================================
bool CoBytestream::readByte (uint8& val) {
	if (bytesLeft() < 1)
		return false;

	val = *m_ptr++;
	return true;
}

// =============================================================================
bool CoBytestream::readByte (int8& val) {
	return readByte (*(reinterpret_cast<uint8*> (&val)));
}

// =============================================================================
bool CoBytestream::readShort (int16& val) {
	if (bytesLeft() < 2)
		return false;
	
	val = 0;
	
	for (int i = 0; i < 2; ++i)
		val |= *m_ptr++ << (i * 8);
	
	return true;
}

// =============================================================================
bool CoBytestream::readShort (uint16& val) {
	return readShort (*(reinterpret_cast<int16*> (&val)));
}

// =============================================================================
bool CoBytestream::readLong (int32& val) {
	if (bytesLeft() < 4)
		return false;
	
	val = 0;
	
	for (int i = 0; i < 4; ++i)
		val |= *m_ptr++ << (i * 8);
	
	return true;
}

// =============================================================================
bool CoBytestream::readLong (uint32& val) {
	return readLong (*(reinterpret_cast<int32*> (&val)));
}

// =============================================================================
bool CoBytestream::readQuad (int64& val) {
	if (bytesLeft() < 8)
		return false;
	
	val = 0;
	
	for (int i = 0; i < 8; ++i)
		val |= *m_ptr++ << (i * 8);
	
	return true;
}

// =============================================================================
bool CoBytestream::readQuad (uint64& val) {
	return readQuad (*(reinterpret_cast<int64*> (&val)));
}

// =============================================================================
bool CoBytestream::readFloat (float& val) {
	int32 ival;

	if (!readLong (ival))
		return false;

	val = *(reinterpret_cast<float*> (&ival));
	return true;
}

// =============================================================================
bool CoBytestream::readDouble (double& val) {
	int64 ival;
	
	if (!readQuad (ival))
		return false;
	
	val = *(reinterpret_cast<double*> (&ival));
	return true;
}

// =============================================================================
bool CoBytestream::readString (CoString& val) {
	if (bytesLeft() < 1)
		return false;
	
	uint8_t c, len;
	
	switch (stringType()) {
	case NullTerminated:
		while (readByte (c) && c != '\0')
			val += (char) c;
		break;
	
	case LengthPrefixed:
		readByte (len);
		
		while (len--) {
			if (!readByte (c))
				return false;
			
			val += (char) c;
		}
		break;
	}
	
	return true;
}

// =============================================================================
void CoBytestream::doWrite (uint8 val) {
	*m_ptr++ = val;
	m_len++;
}

void CoBytestream::growToFit (ulong bytes) {
	if (spaceLeft() < bytes)
		resize (m_size + bytes + 128);
}

bool CoBytestream::readBytes (uint8 numbytes, uint8* val) {
	while (numbytes--)
		if (!readByte (*val++))
			return false;
	
	return true;
}

void CoBytestream::writeBytes (uint8 numbytes, const uint8* val) {
	growToFit (numbytes);
	
	while (numbytes--)
		writeByte (*val++);
}

// =============================================================================
void CoBytestream::writeByte (uint8 val) {
	growToFit (1);
	doWrite (val);
}

// =============================================================================
void CoBytestream::writeShort (int16 val) {
	growToFit (2);

	for (int i = 0; i < 2; ++i)
		doWrite ( (val >> (i * 8)) & 0xFF);
}

// =============================================================================
void CoBytestream::writeLong (int32 val) {
	growToFit (4);
	
	for (int i = 0; i < 4; ++i)
		doWrite ((val >> (i * 8)) & 0xFF);
}

// =============================================================================
void CoBytestream::writeQuad (int64 val) {
	growToFit (8);
	
	for (int i = 0; i < 8; ++i)
		doWrite ((val >> (i * 8)) & 0xFF);
}

// =============================================================================
void CoBytestream::writeFloat (float val) {
	writeLong (*(reinterpret_cast<int32*> (&val)));
}

void CoBytestream::writeDouble (double val) {
	writeLong (*(reinterpret_cast<int64*> (&val)));
}

// =============================================================================
void CoBytestream::writeString (const str& val) {
	growToFit (val.length() + 1);
	
	switch (stringType()) {
	case NullTerminated:
		for (char c : val)
			doWrite (c);
		
		doWrite ('\0');
		break;
	
	case LengthPrefixed:
		doWrite (val.length());
		
		for (char c : val)
			doWrite (c);
		break;
	}
}

// =============================================================================
bool CoBytestream::tryMerge (const CoBytestream& other) {
	if (spaceLeft() < other.length())
		return false;

	for (ulong i = 0; i < other.length(); ++i)
		writeByte (other[i]);

	return true;
}

void CoBytestream::merge (const CoBytestream& other) {
	growToFit (other.length());

	if (!tryMerge (other)) {
		// Shouldn't happen
		fprint (stderr, "ByteStream: Not enough space for merge (%1 bytes left, need %2)",
				spaceLeft(), other.length());
		abort();
	}
}

const uint8* CoBytestream::data() const {
	return m_data;
}

// =============================================================================
CoBytestream::StringType CoBytestream::stringType() const {
	return m_stringType;
}

void CoBytestream::setStringType (StringType type) {
	m_stringType = type;
}