#ifndef LIBCOBALT_BYTESTREAM_H
#define LIBCOBALT_BYTESTREAM_H

#include "main.h"
#include "types.h"

class String;

// TODO: Make able to handle big-endian too
class Bytestream
{
	public:
		enum StringType
		{
			NullTerminated,
			LengthPrefixed,
		};

		Bytestream( ulong len = 2048 );
		Bytestream( const char* data, ulong len );
		Bytestream( const initlist<uint8>& bytes );

		void init( const char* data, ulong len );
		void rewind();
		void seek( ulong pos );
		void clear();
		void merge( const Bytestream& other );
		bool tryMerge( const Bytestream& other );
		void resize( ulong len );
		ulong bytesLeft() const;
		ulong spaceLeft() const;
		void growToFit( ulong bytes );
		const uint8* data() const;
		StringType stringType() const;
		void setStringType( StringType type );

		inline size_t length() const
		{
			return mLen;
		}

		bool readBytes( uint8 numbytes, uint8* val );
		bool readByte( int8& val );
		bool readByte( uint8& val );
		bool readShort( int16& val );
		bool readShort( uint16& val );
		bool readLong( int32& val );
		bool readLong( uint32& val );
		bool readQuad( int64& val );
		bool readQuad( uint64& val );
		bool readString( String& val );
		bool readFloat( float& val );
		bool readDouble( double& val );

		void writeBytes( uint8 numbytes, const uint8* val );
		void writeByte( uint8 val );
		void writeShort( int16 val );
		void writeLong( int32 val );
		void writeQuad( int64 val );
		void writeFloat( float val );
		void writeDouble( double val );
		void writeString( const str& val );

		Bytestream& operator<< ( const Bytestream& other )
		{
			merge( other );
			return *this;
		}

		uint8& subscript( ulong idx );
		const uint8& const_subscript( ulong idx ) const;

		uint8& operator[]( ulong idx )
		{
			return subscript( idx );
		}

		const uint8& operator[]( ulong idx ) const
		{
			return const_subscript( idx );
		}

	private:
		uint8*     mData;
		uint8*     mPtr;
		ulong      mSize,
				   mLen;
		StringType mStringType;

		void doWrite( uint8 val );
};

#endif // LIBCOBALT_BYTESTREAM_H
