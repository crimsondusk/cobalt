#pragma once
#include <string.h>
#include "types.h"
#include "string.h"

#define DECLARE_VARIANT_TYPE( T ) \
public: \
	variant (const T& val); \
	T as_##T() const; \
private: \
	void build_from_##T( const T& val ); \
	static void* clone_as_##T( variant const& var ); \
	static void delete_as_##T( const void* val );

namespace cbl
{
	class time;
	class date;
	class ip_range;
	class ip_address;

	class variant
	{
		public:
			using deinit_func = void (*)( const void* );
			using clone_func = void* (*)( const variant& );

			enum data_type
			{
				int_type,
				short_type,
				long_type,
				uint_type,
				ushort_type,
				ulong_type,
				char_type,
				uchar_type,
				float_type,
				double_type,
				longdouble_type,
				ip_address_type,
				ip_range_type,
				string_type,
				time_type,
				date_type,
				pointer_type,
			};

			DECLARE_VARIANT_TYPE( int )
			DECLARE_VARIANT_TYPE( short )
			DECLARE_VARIANT_TYPE( long )
			DECLARE_VARIANT_TYPE( uint )
			DECLARE_VARIANT_TYPE( ushort )
			DECLARE_VARIANT_TYPE( ulong )
			DECLARE_VARIANT_TYPE( char )
			DECLARE_VARIANT_TYPE( uchar )
			DECLARE_VARIANT_TYPE( float )
			DECLARE_VARIANT_TYPE( double )
			DECLARE_VARIANT_TYPE( ldouble )
			DECLARE_VARIANT_TYPE( ip_address )
			DECLARE_VARIANT_TYPE( ip_range )
			DECLARE_VARIANT_TYPE( string )
			DECLARE_VARIANT_TYPE( time )
			DECLARE_VARIANT_TYPE( date )

		public:
			variant( void* val );
			variant( const char* val );
			variant( std::nullptr_t );
			variant( const variant& val );
			~variant();

			void			build_as_pointer( void* val, const char* typestring );
			int				allocsize() const;
			const void*		as_pointer() const;
			string			describe() const;
			const char*		typestring() const;
			data_type		value_type() const;
			int				size() const;

			inline clone_func cloneFunc() const
			{
				return m_clonefunc;
			}

			inline deinit_func deinitFunc() const
			{
				return m_deinitfunc;
			}

		private:
			int				m_allocsize;
			void*			m_value;
			const char*		m_typestring;
			data_type		m_type;
			deinit_func		m_deinitfunc;
			clone_func		m_clonefunc;
	};
}
