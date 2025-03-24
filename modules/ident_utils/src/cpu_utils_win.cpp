#include <ident_utils/cpu_utils.hpp>

#include <Windows.h>
#include <array>
#include <stdexcept>
#include <thread>


namespace lcxx::experimental::ident_utils::cpu {

    auto get_info() -> cpu_info
    {
        cpu_info ci{};
        // Get the number of cores and threads
        ci.n_cores   = std::thread::hardware_concurrency() / 2;  // Assuming hyper-threading
        ci.n_threads = std::thread::hardware_concurrency();

        // Get CPU vendor
        std::array< int, 4 >     cpui;
        std::array< char, 0x20 > vendor{};
        __cpuid( cpui.data(), 0 );
        std::memcpy( vendor.data(), &cpui[1], 4 );
        std::memcpy( vendor.data() + 4, &cpui[3], 4 );
        std::memcpy( vendor.data() + 8, &cpui[2], 4 );
        ci.vendor = vendor.data();

        // Get CPU model name
        std::array< char, 0x40 > model_name{};
        for ( int i = 0; i < 3; ++i ) {
            __cpuid( reinterpret_cast< int * >( cpui.data() ), 0x80000002 + i );
            std::memcpy( model_name.data() + i * 16, cpui.data(), 16 );
        }
        ci.model_name = model_name.data();

        // Get maximum frequency (requires Windows API)
        HKEY hKey;
        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT( "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0" ), 0,
                           KEY_READ, &hKey ) == ERROR_SUCCESS ) {
            DWORD mhz;
            DWORD size = sizeof( mhz );
            if ( RegQueryValueEx( hKey, TEXT( "~MHz" ), nullptr, nullptr, reinterpret_cast< LPBYTE >( &mhz ), &size ) ==
                 ERROR_SUCCESS ) {
                ci.max_frequency = mhz;
            }
            RegCloseKey( hKey );
        }
        return ci;
    }

}  // namespace lcxx::experimental::ident_utils::cpu
