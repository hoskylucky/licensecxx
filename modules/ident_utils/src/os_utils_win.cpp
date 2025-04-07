#include <ident_utils/os_utils.hpp>

#include <stdexcept>

#include <Windows.h>

typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS ( 0x00000000 )

typedef NTSTATUS( WINAPI * RtlGetVersionPtr )( PRTL_OSVERSIONINFOW );

RTL_OSVERSIONINFOW GetRealOSVersion()
{
    RTL_OSVERSIONINFOW rovi = {};
    HMODULE            hMod = ::GetModuleHandleW( L"ntdll.dll" );
    if ( hMod ) {
        RtlGetVersionPtr fxPtr = ( RtlGetVersionPtr )::GetProcAddress( hMod, "RtlGetVersion" );
        if ( fxPtr != nullptr ) {
            rovi.dwOSVersionInfoSize = sizeof( rovi );
            if ( STATUS_SUCCESS == fxPtr( &rovi ) ) {
                return rovi;
            }
        }
    }
    return rovi;
}

namespace lcxx::experimental::ident_utils::os {

    auto get_info() -> os_info
    {
        os_info oi{};

        char  user_name[256];
        DWORD user_name_size = sizeof( user_name );
        if ( GetUserNameA( user_name, &user_name_size ) ) {
            oi.user_name = user_name;
        }
        else {
            throw std::runtime_error( "Failed to retrieve user name" );
        }

        SYSTEM_INFO si;
        GetSystemInfo( &si );

        switch ( si.wProcessorArchitecture ) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            oi.os_architecture = "x64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            oi.os_architecture = "x86";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            oi.os_architecture = "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            oi.os_architecture = "ARM64";
            break;
        default:
            oi.os_architecture = "Unknown";
            break;
        }

        RTL_OSVERSIONINFOEXW osvi;
        ZeroMemory( &osvi, sizeof( RTL_OSVERSIONINFOEXW ) );
        osvi.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOEXW );

        auto version = GetRealOSVersion();
        oi.os_name   = std::to_string( version.dwMajorVersion ) + "." + std::to_string( version.dwMinorVersion ) + "." +
                     std::to_string( version.dwBuildNumber );

        char  computer_name[256];
        DWORD computer_name_size = sizeof( computer_name );
        if ( GetComputerNameA( computer_name, &computer_name_size ) ) {
            oi.os_pc_name = computer_name;
        }
        else {
            throw std::runtime_error( "Failed to retrieve computer name" );
        }

        // Retrieve user group IDs (GIDs)
        // HANDLE token_handle = nullptr;
        // if ( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token_handle ) ) {
        //     DWORD group_info_size = 0;
        //     GetTokenInformation( token_handle, TokenGroups, nullptr, 0, &group_info_size );

        //     if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
        //         auto group_info = reinterpret_cast< PTOKEN_GROUPS >( new BYTE[group_info_size] );
        //         if ( GetTokenInformation( token_handle, TokenGroups, group_info, group_info_size, &group_info_size )
        //         ) {
        //             for ( DWORD i = 0; i < group_info->GroupCount; ++i ) {
        //                 SID_IDENTIFIER_AUTHORITY * sid_authority =
        //                     GetSidIdentifierAuthority( group_info->Groups[i].Sid );
        //                 if ( sid_authority ) {
        //                     DWORD sub_authority_count = *GetSidSubAuthorityCount( group_info->Groups[i].Sid );
        //                     if ( sub_authority_count > 0 ) {
        //                         DWORD gid = *GetSidSubAuthority( group_info->Groups[i].Sid, sub_authority_count - 1
        //                         ); oi.user_gids.emplace_back( gid );
        //                     }
        //                 }

        //                 SID_NAME_USE sid_type;
        //                 WCHAR        group_name[256];
        //                 WCHAR        domain_name[256];
        //                 DWORD        group_name_size  = 256;
        //                 DWORD        domain_name_size = 256;

        //                 if ( LookupAccountSidW( nullptr, group_info->Groups[i].Sid, group_name, &group_name_size,
        //                                         domain_name, &domain_name_size, &sid_type ) ) {
        //                     int size_needed = WideCharToMultiByte( CP_UTF8, 0, group_name, group_name_size, nullptr,
        //                     0,
        //                                                            nullptr, nullptr );
        //                     if ( size_needed > 0 ) {
        //                         std::string group_name_utf8( size_needed, 0 );
        //                         WideCharToMultiByte( CP_UTF8, 0, group_name, group_name_size, &group_name_utf8[0],
        //                                              size_needed, nullptr, nullptr );
        //                         oi.user_groups.emplace_back( group_name_utf8 );
        //                     }
        //                 }
        //             }
        //         }
        //         delete[] reinterpret_cast< BYTE * >( group_info );
        //     }

        //     DWORD token_user_size = 0;
        //     GetTokenInformation( token_handle, TokenUser, nullptr, 0, &token_user_size );
        //     if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
        //         auto token_user = reinterpret_cast< TOKEN_USER * >( new BYTE[token_user_size] );

        //         // Second call to retrieve the token information
        //         if ( GetTokenInformation( token_handle, TokenUser, token_user, token_user_size, &token_user_size ) )
        //         {
        //             SID_IDENTIFIER_AUTHORITY * sid_authority = GetSidIdentifierAuthority( token_user->User.Sid );
        //             if ( sid_authority ) {
        //                 DWORD sub_authority_count = *GetSidSubAuthorityCount( token_user->User.Sid );
        //                 if ( sub_authority_count > 0 ) {
        //                     DWORD uid   = *GetSidSubAuthority( token_user->User.Sid, sub_authority_count - 1 );
        //                     oi.user_uid = uid;
        //                 }
        //             }
        //         }
        //         delete[] reinterpret_cast< BYTE * >( token_user );
        //     }
        //     CloseHandle( token_handle );
        // }
        // else {
        //     throw std::runtime_error( "Failed to retrieve user group IDs" );
        // }

        return oi;
    }

}  // namespace lcxx::experimental::ident_utils::os
