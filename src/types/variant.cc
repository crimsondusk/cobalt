#include "variant.h"
#include "format.h"
#include "time.h"
#include "ip.h"
#include "date.h"

#define DEFINE_VARIANT_TYPE(T) \
	variant::variant( T const& val ) \
	{ \
		build_from_##T (val); \
	} \
	\
	T variant::as_##T() const \
	{ \
		assert( value_type() == T##_type ); \
		return *( reinterpret_cast<T*>( m_value )); \
	} \
	\
	void variant::build_from_##T( const T& val )\
	{ \
		T* valptr = new T( val ); \
		\
		/* Init the variant */ \
		m_type = T##_type; \
		m_typestring = #T; \
		m_allocsize = sizeof (T); \
		m_value = valptr; \
		\
		/* Write down the functions we'll clone or deinit this with */ \
		m_deinitfunc = variant::delete_as_##T; \
		m_clonefunc = variant::clone_as_##T; \
	} \
	\
	void* variant::clone_as_##T( const variant& var )\
	{ \
		return new T( var.as_##T() ); \
	} \
	\
	void variant::delete_as_##T( const void* val )\
	{ \
		const T* c_ptr = reinterpret_cast<const T*>( val ); \
		delete const_cast<T*>( c_ptr ); \
	}

namespace cbl
{
	DEFINE_VARIANT_TYPE( int )
	DEFINE_VARIANT_TYPE( short )
	DEFINE_VARIANT_TYPE( long )
	DEFINE_VARIANT_TYPE( uint )
	DEFINE_VARIANT_TYPE( ushort )
	DEFINE_VARIANT_TYPE( ulong )
	DEFINE_VARIANT_TYPE( char )
	DEFINE_VARIANT_TYPE( uchar )
	DEFINE_VARIANT_TYPE( float )
	DEFINE_VARIANT_TYPE( double )
	DEFINE_VARIANT_TYPE( string )
	DEFINE_VARIANT_TYPE( time )
	DEFINE_VARIANT_TYPE( date )
	DEFINE_VARIANT_TYPE( ip_address )
	DEFINE_VARIANT_TYPE( ip_range )

	// -----------------------------------------------------------------------------
	//
	variant::variant( void* val )
	{
		build_as_pointer( val, "void*" );
	}

	// -----------------------------------------------------------------------------
	//
	variant::variant( const char* val )
	{
		build_from_string( val );
	}

	// -----------------------------------------------------------------------------
	//
	variant::variant( std::nullptr_t )
	{
		build_as_pointer( null, "std::nullptr_t" );
	}

	// -----------------------------------------------------------------------------
	//
	variant::variant( const variant& val )
	{
		m_type = val.value_type();
		m_typestring = val.typestring();
		m_deinitfunc = val.deinitFunc();
		m_clonefunc = val.cloneFunc();

		if( val.value_type() != pointer_type )
		{
			m_value = ( *val.cloneFunc() )( val );
		}
		else
		{
			m_value = const_cast<void*>( val.as_pointer() );
			m_allocsize = 0;
		}
	}

	// -----------------------------------------------------------------------------
	//
	void variant::build_as_pointer( void* val, const char* typestring )
	{
		m_type = pointer_type;
		m_value = val;
		m_typestring = typestring;
		m_allocsize = 0;
	}

	// -----------------------------------------------------------------------------
	//
	int variant::allocsize() const
	{
		return m_allocsize;
	}

	// -----------------------------------------------------------------------------
	//
	const void* variant::as_pointer() const
	{
		return m_value;
	}

	// -----------------------------------------------------------------------------
	//
	variant::~variant()
	{
		( *m_deinitfunc )( as_pointer() );
	}

	// -----------------------------------------------------------------------------
	//
	const char* variant::typestring() const
	{
		return m_typestring;
	}

	// -----------------------------------------------------------------------------
	//
	variant::data_type variant::value_type() const
	{
		return m_type;
	}

	// -----------------------------------------------------------------------------
	//
	int variant::size() const
	{
		return sizeof m_value;
	}

	// -----------------------------------------------------------------------------
	//
	string variant::describe() const
	{
		switch( m_type )
		{
#define NUMERIC_STRINGREP( T, C ) \
			case T##_type: \
			{ \
				char valstr[64]; \
				sprintf( valstr, "%" #C, as_##T() ); \
				return valstr; \
			}

			NUMERIC_STRINGREP( char,		d )
			NUMERIC_STRINGREP( int,			d )
			NUMERIC_STRINGREP( short,		d )
			NUMERIC_STRINGREP( long,		ld )
			NUMERIC_STRINGREP( uchar,		u )
			NUMERIC_STRINGREP( uint,		u )
			NUMERIC_STRINGREP( ushort,		u )
			NUMERIC_STRINGREP( ulong,		lu )
			NUMERIC_STRINGREP( float,		f )
			NUMERIC_STRINGREP( double,		g )
			NUMERIC_STRINGREP( pointer,		p )

			case string_type:
				return as_string();

			case time_type:
				return as_time().describe();

			case date_type:
				return as_date().describe();

			case ip_address_type:
				return as_ip_address().to_string( true );

			case ip_range_type:
				return as_ip_range().to_string();

			default:
				return string( "<" ) + typestring() + ">";
		}

		return "";
	}
}
