#pragma once
#include <vector>
#include <thread>
#include <random>
#include <mutex>

constexpr int g_ChangeOffsetDelay = 1000;
constexpr int g_PossibleOffsetsForVar = 1000;

template<class T>
class pvar
{
private:
	bool m_ThreadCheck = true;

	T m_Value;
	std::vector<T> m_pValue = std::vector<T>( g_PossibleOffsetsForVar );

	int m_Pos = 0; // stores 'pseudo-position' in RAM for protected variable
	pvar * m_object;

	std::random_device m_Device;
	std::mt19937 m_rng { m_Device( ) };
	std::uniform_int_distribution<int> m_Dist { 0 , g_PossibleOffsetsForVar - 1 };

	std::thread m_Thread[ 2 ];
	std::mutex m_Mutex;

	auto fnCheckThread( ) -> void
	{
		while( m_ThreadCheck )
		{
			std::unique_lock<std::mutex> uLock( m_Mutex );

			if( m_pValue[ m_Pos ] != m_object->m_Value )
			{
				// behaviour after detection

				char Buff[ 1024 ];
				sprintf_s( Buff , "oldValue: %d\nnewValue: %d" , m_pValue[ m_Pos ] , m_object->m_Value );
				MessageBox( NULL , Buff , "Detected!" , MB_ICONWARNING | MB_OK );
				m_object->m_Value = m_pValue[ m_Pos ];
			}

			uLock.unlock( );

			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	auto fnMoveThread( ) -> void
	{
		while( m_ThreadCheck )
		{
			std::unique_lock<std::mutex> uLock( m_Mutex );

			m_Pos = m_Dist( m_rng );
			m_pValue[ m_Pos ] = m_object->m_Value;

			uLock.unlock( );

			std::this_thread::sleep_for( std::chrono::milliseconds( g_ChangeOffsetDelay ) );
		}
	}

	auto fnInit( pvar * object ) -> void
	{
		m_object = object;

		m_Thread[ 0 ] = std::thread( [ & ] { fnMoveThread( ); } );
		m_Thread[ 1 ] = std::thread( [ & ] { fnCheckThread( ); } );
	}

	auto fnSetValue( T t ) -> void
	{
		m_pValue[ m_Pos ] = t;
		m_Value = t;
	}

public:
	pvar( ) { fnInit( this ); }

	~pvar( )
	{
		m_ThreadCheck = false;

		this->m_Thread[ 0 ].join( );
		this->m_Thread[ 1 ].join( );
	}

	pvar( T t )
	{
		this->m_Value = t;
		fnInit( this );
	}

	pvar( T & t )
	{
		this->m_Value = t;
		fnInit( this );
	}

	pvar( T * t )
	{
		this->m_Value = t;
		fnInit( this );
	}

	auto operator++( ) -> pvar &
	{
		std::unique_lock<std::mutex> uLock( m_Mutex );
		fnSetValue( this->m_Value + 1 );
		return *this;
	}

	auto operator=( const T * t ) -> pvar &
	{
		std::unique_lock<std::mutex> uLock( m_Mutex );
		fnSetValue( *t );
		return *this;
	}

	auto operator=( const T & t ) -> pvar &
	{
		std::unique_lock<std::mutex> uLock( m_Mutex );
		fnSetValue( t );
		return *this;
	}

	operator T&( ) { return this->m_Value; }
};
