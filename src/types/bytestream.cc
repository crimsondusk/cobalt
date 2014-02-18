#include "bytestream.h"
#include "format.h"
#include "misc.h"
#include <string.h>

Bytestream::Bytestream (ulong len) :
	mData (null),
	mStringType (NullTerminated)
{
	resize (len);
	clear();
}

Bytestream::Bytestream (const char* data, ulong len) :
	mData (null),
	mStringType (NullTerminated)
{
	mData = null;
	init (data, len);
}

Bytestream::Bytestream (const initlist<uint8>& bytes) :
	mData (null),
	mStringType (NullTerminated)
{
	char* data = new char[bytes.size()];
	char* p = &data[0];

	for (uint8 byte : bytes)
		*p++ = byte;

	init (data, bytes.size());
	delete[] data;
}

void Bytestream::resize (ulong newsize)
{
	char* olddata = null;
	ulong oldsize;

	if (mData)
	{
		oldsize = mSize;
		olddata = new char[oldsize];
		memcpy (olddata, mData, oldsize);
	}

	delete[] mData;
	mData = new uint8[newsize];
	mSize = newsize;

	if (olddata)
		memcpy (mData, olddata, min<ulong> (oldsize, newsize));
}

void Bytestream::init (const char* data, ulong len)
{
	resize (len);
	memcpy (mData, data, len);
	mPtr = &mData[0];
	mLen = len;
}

void Bytestream::clear()
{
	mPtr = &mData[0];
	mLen = 0;
}

uint8& Bytestream::subscript (ulong idx)
{
	return mData[idx];
}

const uint8& Bytestream::const_subscript (ulong idx) const
{
	return mData[idx];
}

void Bytestream::seek (ulong pos)
{
	mPtr = mData + pos;
}

// =============================================================================
void Bytestream::rewind()
{
	mPtr = mData;
}

ulong Bytestream::bytesLeft() const
{
	return (mLen - (mPtr - &mData[0]));
}

ulong Bytestream::spaceLeft() const
{
	return (mSize - mLen);
}

// =============================================================================
bool Bytestream::readByte (uint8& val)
{
	if (bytesLeft() < 1)
		return false;

	val = *mPtr++;
	return true;
}

// =============================================================================
bool Bytestream::readByte (int8& val)
{
	return readByte (* (reinterpret_cast<uint8*> (&val)));
}

// =============================================================================
bool Bytestream::readShort (int16& val)
{
	if (bytesLeft() < 2)
		return false;

	val = 0;

	for (int i = 0; i < 2; ++i)
		val |= *mPtr++ << (i * 8);

	return true;
}

// =============================================================================
bool Bytestream::readShort (uint16& val)
{
	return readShort (* (reinterpret_cast<int16*> (&val)));
}

// =============================================================================
bool Bytestream::readLong (int32& val)
{
	if (bytesLeft() < 4)
		return false;

	val = 0;

	for (int i = 0; i < 4; ++i)
		val |= *mPtr++ << (i * 8);

	return true;
}

// =============================================================================
bool Bytestream::readLong (uint32& val)
{
	return readLong (* (reinterpret_cast<int32*> (&val)));
}

// =============================================================================
bool Bytestream::readQuad (int64& val)
{
	if (bytesLeft() < 8)
		return false;

	val = 0;

	for (int i = 0; i < 8; ++i)
		val |= *mPtr++ << (i * 8);

	return true;
}

// =============================================================================
bool Bytestream::readQuad (uint64& val)
{
	return readQuad (* (reinterpret_cast<int64*> (&val)));
}

// =============================================================================
bool Bytestream::readFloat (float& val)
{
	int32 ival;

	if (!readLong (ival))
		return false;

	val = * (reinterpret_cast<float*> (&ival));
	return true;
}

// =============================================================================
bool Bytestream::readDouble (double& val)
{
	int64 ival;

	if (!readQuad (ival))
		return false;

	val = * (reinterpret_cast<double*> (&ival));
	return true;
}

// =============================================================================
bool Bytestream::readString (String& val)
{
	if (bytesLeft() < 1)
		return false;

	uint8_t c, len;

	switch (stringType())
	{
	case NullTerminated:
	{
		while (readByte (c) && c != '\0')
			val += (char) c;
	} break;

	case LengthPrefixed:
	{
		readByte (len);

		while (len--)
		{
			if (!readByte (c))
				return false;

			val += (char) c;
		}
	} break;
	}

	return true;
}

// =============================================================================
void Bytestream::doWrite (uint8 val)
{
	*mPtr++ = val;
	mLen++;
}

void Bytestream::growToFit (ulong bytes)
{
	if (spaceLeft() < bytes)
		resize (mSize + bytes + 128);
}

bool Bytestream::readBytes (uint8 numbytes, uint8* val)
{
	while (numbytes--)
		if (!readByte (*val++))
			return false;

	return true;
}

void Bytestream::writeBytes (uint8 numbytes, const uint8* val)
{
	growToFit (numbytes);

	while (numbytes--)
		writeByte (*val++);
}

// =============================================================================
void Bytestream::writeByte (uint8 val)
{
	growToFit (1);
	doWrite (val);
}

// =============================================================================
void Bytestream::writeShort (int16 val)
{
	growToFit (2);

	for (int i = 0; i < 2; ++i)
		doWrite ( (val >> (i * 8)) & 0xFF);
}

// =============================================================================
void Bytestream::writeLong (int32 val)
{
	growToFit (4);

	for (int i = 0; i < 4; ++i)
		doWrite ( (val >> (i * 8)) & 0xFF);
}

// =============================================================================
void Bytestream::writeQuad (int64 val)
{
	growToFit (8);

	for (int i = 0; i < 8; ++i)
		doWrite ( (val >> (i * 8)) & 0xFF);
}

// =============================================================================
void Bytestream::writeFloat (float val)
{
	writeLong (* (reinterpret_cast<int32*> (&val)));
}

void Bytestream::writeDouble (double val)
{
	writeLong (* (reinterpret_cast<int64*> (&val)));
}

// =============================================================================
void Bytestream::writeString (const str& val)
{
	growToFit (val.length() + 1);

	switch (stringType())
	{
	case NullTerminated:
	{
		for (char c : val)
			doWrite (c);

		doWrite ('\0');
	} break;

	case LengthPrefixed:
	{
		doWrite (val.length());

		for (char c : val)
			doWrite (c);
	} break;
	}
}

// =============================================================================
bool Bytestream::tryMerge (const Bytestream& other)
{
	if (spaceLeft() < other.length())
		return false;

	for (ulong i = 0; i < other.length(); ++i)
		writeByte (other[i]);

	return true;
}

void Bytestream::merge (const Bytestream& other)
{
	growToFit (other.length());

	if (!tryMerge (other))
	{
		// Shouldn't happen
		PrintTo (stderr, "ByteStream: Not enough space for merge (%1 bytes left, need %2)",
				spaceLeft(), other.length());
		abort();
	}
}

const uint8* Bytestream::data() const
{
	return mData;
}

// =============================================================================
Bytestream::StringType Bytestream::stringType() const
{
	return mStringType;
}

void Bytestream::setStringType (StringType type)
{
	mStringType = type;
}
