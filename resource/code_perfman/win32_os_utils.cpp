#include <Windows.h>
#include <Psapi.h>
#include "win32_os_utils.h"


#pragma comment ( lib , "Psapi.lib" )


namespace win32 {
namespace os_utils {

    const char* ProcessNameID_2_name(DWORD process_id, char* out_process_name, unsigned int buf_size)
    {
        char* psz = out_process_name;
        *psz = 0;
        HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                    PROCESS_VM_READ,
                    FALSE, process_id );

        // Get the process name.
        if ( hProcess )
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
            {
                GetModuleBaseName( hProcess, hMod, out_process_name, buf_size);                    
            }
        }
        
        CloseHandle( hProcess );
        return psz;
    }

int query_process_cnt(const char* process_name)
{
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 0;

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.
    char tmp[MAX_PATH] = {0};
    int cnt = 0;
    char qry_exe_path[MAX_PATH];
    strcpy(qry_exe_path, process_name);
    if (strstr(qry_exe_path, ".exe") == NULL)
    {
        strcat(qry_exe_path, ".exe");
    }
    
    for ( i = 0; i < cProcesses; i++ )
    {
        ProcessNameID_2_name(aProcesses[i], tmp, sizeof(tmp) );
        if (strcmp(tmp, qry_exe_path) == 0 )
        {
            ++cnt;
        }
    }

    return cnt;
}


} // namespace os_utils

} // namespace win32