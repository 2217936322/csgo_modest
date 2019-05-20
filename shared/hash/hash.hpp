#pragma once

namespace shared::hash
{
	constexpr uint64_t BASIS = 0x811c9dc5;
	constexpr uint64_t PRIME = 0x1000193;

	/// <summary>
	/// Creates hash of text during compile-time
	/// </summary>
	/// <param name="txt">The text that is going to be hashed</param>
	/// <param name="value">The current hash value</param>
	/// <returns>Hashed text</returns>
	inline constexpr uint32_t get_const( const char* txt, const uint32_t value = BASIS ) noexcept
	{
		/// Recursive hashing
		return ( txt[ 0 ] == '\0' ) ? value :
			get_const( &txt[ 1 ], ( value ^ uint32_t( txt[ 0 ] ) ) * PRIME );
	}

	/// <summary>
	/// Creates hash of text during run-time
	/// </summary>
	/// <param name="str">The text that is going to be hashed</param>
	/// <returns>Hashed text</returns>
	inline uint32_t get( const char* txt )
	{
		uint32_t ret = BASIS;

		uint32_t length = strlen( txt );
		for ( auto i = 0u; i < length; ++i )
		{
			/// OR character and multiply it with fnv1a prime
			ret ^= txt[ i ];
			ret *= PRIME;
		}

		return ret;
	}
}

/// <summary>
/// Creates hash of text during compile-time
/// </summary>
/// <param name="str">The text that is going to be hashed</param>
/// <returns>Hashed text</returns>
#define CT_HASH( str ) \
       [ ]( ) { \
           constexpr uint32_t ret = shared::hash::get_const( str ); \
           return ret; \
       }( )

/// <summary>
/// Creates hash of text during run-time
/// </summary>
/// <param name="str">The text that is going to be hashed</param>
/// <returns>Hashed text</returns>
#define HASH( str ) shared::hash::get( str )