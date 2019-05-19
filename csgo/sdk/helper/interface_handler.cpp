#include "../../csgo.hpp"

namespace interface_handler
{
	/// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/tier1/interface.h#L72
	using InstantiateInterfaceFn = void* ( __cdecl* )( );
	struct InterfaceReg
	{
		InstantiateInterfaceFn	m_CreateFn;
		const char* m_pName;

		InterfaceReg* m_pNext; // For the global list.
		static InterfaceReg* s_pInterfaceRegs;
	};

	std::unordered_map< uint32_t,
		std::vector< std::pair< const char*, shared::address_t >>> m_interfaces{};

	uintptr_t* get( const char* mod, const char* interface_name )
	{
		auto& entry = m_interfaces[ HASH( mod ) ];

		if ( entry.empty() )
		{
			auto module_address = GetModuleHandleA( mod );

			/// Find internal CreateInterface function and walk the context
			auto create_interface_fn = shared::address_t( GetProcAddress( module_address, "CreateInterface" ) );
			if ( !create_interface_fn )
				return nullptr;

			/// Is this is the right function the 5th byte should be jmp opcode
			/// .text:108C25F0 55                                push    ebp
			/// .text:108C25F1 8B EC                             mov     ebp, esp
			/// .text:108C25F3 5D                                pop     ebp
			/// .text:108C25F4 E9 87 FF FF FF                    jmp     sub_108C2580    ; Jump
			///                ^ Check for this byte
			if ( create_interface_fn.offset( 0x4 ).get<byte>() != 0xE9 )
				return nullptr;

			/// Now that we know that there is a jump we will follow that jump
			/// After that get the interface_reg_list
			/// .text:108C2580 55                                push    ebp
			///	.text : 108C2581 8B EC                           mov     ebp, esp
			///	.text : 108C2583 56                              push    esi
			///	.text : 108C2584 8B 35 64 CA 14 13               mov     esi, interface_reg_list
			///                        ^_________^ Get this address
			auto list = create_interface_fn.rel( 0x5 ).offset( 0x6 ).get< InterfaceReg* >( 2 );
			if ( !list )
				return nullptr;

			while ( list )
			{
				entry.push_back( std::make_pair( list->m_pName, uintptr_t( list->m_CreateFn() ) ) );

				list = list->m_pNext;
			}
		}

		/// Sanity check, shouldn't really be empty unless you pass
		/// a wrong module name
		if ( entry.empty() )
			return nullptr;

		for ( auto& cur : entry )
		{
			if ( std::string( cur.first ).find( interface_name ) != std::string::npos )
				return cur.second.as<uintptr_t*>();
		}

		/// No valid interface found
		return nullptr;
	}
}