#pragma once

#define PROPERTY( ACCESS, TYPE, READ, WRITE, WRITETYPE )			\
	private:														\
		TYPE m_##READ;												\
																	\
	public:															\
		inline TYPE const& READ() const								\
		{															\
			return m_##READ; 										\
		}															\
																	\
	ACCESS:															\
		void WRITE( TYPE const& a ) PROPERTY_##WRITETYPE( READ )	\

#define PROPERTY_STOCK_WRITE( READ )	\
		{								\
			m_##READ = a;				\
		}

#define PROPERTY_CUSTOM_WRITE( READ )	\
		;
