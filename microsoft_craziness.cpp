//
// Author:   Jonathan Blow
// Version:  2
// Date:     7 May, 2019  (update to original version released on 31 August, 2018).
//
// This code is released under the MIT license, which you can find at
//
//          https://opensource.org/licenses/MIT
//
//
//
// See the comments for how to use this library just below the includes.
//

// I added some of my own nonsense to make it easier for myself

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

#include <stdint.h>
#include <io.h>         // For _get_osfhandle

//
//
// HOW TO USE THIS CODE
//
// The purpose of this file is to find the folders that contain libraries
// you may need to link against, on Windows, if you are linking with any
// compiled C or C++ code. This will be necessary for many non-C++ programming
// language environments that want to provide compatibility.
//
// We find the place where the Visual Studio libraries live (for example,
// libvcruntime.lib), where the linker and compiler executables live
// (for example, link.exe), and where the Windows SDK libraries reside
// (kernel32.lib, libucrt.lib).
//
// We all wish you didn't have to worry about so many weird dependencies,
// but we don't really have a choice about this, sadly.
//
// I don't claim that this is the absolute best way to solve this problem,
// and so far we punt on things (if you have multiple versions of Visual Studio
// installed, we return the first one, rather than the newest). But it
// will solve the basic problem for you as simply as I know how to do it,
// and because there isn't too much code here, it's easy to modify and expand.
//
//
// Here is the API you need to know about:
//

struct Find_Result {
    int windows_sdk_version;   // Zero if no Windows SDK found.

    wchar_t *windows_sdk_root              = NULL;
    wchar_t *windows_sdk_um_library_path   = NULL;
    wchar_t *windows_sdk_ucrt_library_path = NULL;
    
    wchar_t *vs_exe_path = NULL;
    wchar_t *vs_library_path = NULL;
};

extern "C" Find_Result find_visual_studio_and_windows_sdk();

void free_resources(Find_Result *result) {
    free(result->windows_sdk_root);
    free(result->windows_sdk_um_library_path);
    free(result->windows_sdk_ucrt_library_path);
    free(result->vs_exe_path);
    free(result->vs_library_path);
}

//
// Call find_visual_studio_and_windows_sdk, look at the resulting
// paths, then call free_resources on the result.
//
// Everything else in this file is implementation details that you
// don't need to care about.
//

//
// This file was about 400 lines before we started adding these comments.
// You might think that's way too much code to do something as simple
// as finding a few library and executable paths. I agree. However,
// Microsoft's own solution to this problem, called "vswhere", is a
// mere EIGHT THOUSAND LINE PROGRAM, spread across 70 files,
// that they posted to github *unironically*.
//
// I am not making this up: https://github.com/Microsoft/vswhere
//
// Several people have therefore found the need to solve this problem
// themselves. We referred to some of these other solutions when 
// figuring out what to do, most prominently ziglang's version,
// by Ryan Saunderson.
// 
// I hate this kind of code. The fact that we have to do this at all
// is stupid, and the actual maneuvers we need to go through
// are just painful. If programming were like this all the time,
// I would quit.
//
// Because this is such an absurd waste of time, I felt it would be
// useful to package the code in an easily-reusable way, in the
// style of the stb libraries. We haven't gone as all-out as some
// of the stb libraries do (which compile in C with no includes, often).
// For this version you need C++ and the headers at the top of the file.
//
// We return the strings as Windows wide character strings. Aesthetically
// I don't like that (I think most sane programs are UTF-8 internally),
// but apparently, not all valid Windows file paths can even be converted
// correctly to UTF-8. So have fun with that. It felt safest and simplest
// to stay with wchar_t since all of this code is fully ensconced in
// Windows crazy-land.
//
// One other shortcut I took is that this is hardcoded to return the
// folders for x64 libraries. If you want x86 or arm, you can make
// slight edits to the code below, or, if enough people want this,
// I can work it in here.
//

// Defer macro/thing.

#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

template<typename T>
struct ExitScope {
    T lambda;
    ExitScope(T lambda):lambda(lambda){}
    ~ExitScope(){lambda();}
    ExitScope(const ExitScope&);
  private:
    ExitScope& operator =(const ExitScope&);
};
 
class ExitScopeHelp {
  public:
    template<typename T>
        ExitScope<T> operator+(T t){ return t;}
};
 
#define defer const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()


// COM objects for the ridiculous Microsoft craziness.

struct DECLSPEC_UUID("B41463C3-8866-43B5-BC33-2B0676F7F42E") DECLSPEC_NOVTABLE ISetupInstance : public IUnknown
{
    STDMETHOD(GetInstanceId)(_Out_ BSTR* pbstrInstanceId) = 0;
    STDMETHOD(GetInstallDate)(_Out_ LPFILETIME pInstallDate) = 0;
    STDMETHOD(GetInstallationName)(_Out_ BSTR* pbstrInstallationName) = 0;
    STDMETHOD(GetInstallationPath)(_Out_ BSTR* pbstrInstallationPath) = 0;
    STDMETHOD(GetInstallationVersion)(_Out_ BSTR* pbstrInstallationVersion) = 0;
    STDMETHOD(GetDisplayName)(_In_ LCID lcid, _Out_ BSTR* pbstrDisplayName) = 0;
    STDMETHOD(GetDescription)(_In_ LCID lcid, _Out_ BSTR* pbstrDescription) = 0;
    STDMETHOD(ResolvePath)(_In_opt_z_ LPCOLESTR pwszRelativePath, _Out_ BSTR* pbstrAbsolutePath) = 0;
};

struct DECLSPEC_UUID("6380BCFF-41D3-4B2E-8B2E-BF8A6810C848") DECLSPEC_NOVTABLE IEnumSetupInstances : public IUnknown
{
    STDMETHOD(Next)(_In_ ULONG celt, _Out_writes_to_(celt, *pceltFetched) ISetupInstance** rgelt, _Out_opt_ _Deref_out_range_(0, celt) ULONG* pceltFetched) = 0;
    STDMETHOD(Skip)(_In_ ULONG celt) = 0;
    STDMETHOD(Reset)(void) = 0;
    STDMETHOD(Clone)(_Deref_out_opt_ IEnumSetupInstances** ppenum) = 0;
};

struct DECLSPEC_UUID("42843719-DB4C-46C2-8E7C-64F1816EFD5B") DECLSPEC_NOVTABLE ISetupConfiguration : public IUnknown
{
    STDMETHOD(EnumInstances)(_Out_ IEnumSetupInstances** ppEnumInstances) = 0;
    STDMETHOD(GetInstanceForCurrentProcess)(_Out_ ISetupInstance** ppInstance) = 0;
    STDMETHOD(GetInstanceForPath)(_In_z_ LPCWSTR wzPath, _Out_ ISetupInstance** ppInstance) = 0;
};


// The beginning of the actual code that does things.

struct Version_Data {
    int32_t best_version[4];  // For Windows 8 versions, only two of these numbers are used.
    wchar_t *best_name;
};

bool os_file_exists(wchar_t *name) {
    // @Robustness: What flags do we really want to check here?
    
    auto attrib = GetFileAttributesW(name);
    if (attrib == INVALID_FILE_ATTRIBUTES) return false;
    if (attrib & FILE_ATTRIBUTE_DIRECTORY) return false;

    return true;
}

extern "C" wchar_t *concat(wchar_t *a, wchar_t *b, wchar_t *c = nullptr, wchar_t *d = nullptr) {
    // Concatenate up to 4 wide strings together. Allocated with malloc.
    // If you don't like that, use a programming language that actually
    // helps you with using custom allocators. Or just edit the code.
    
    auto len_a = wcslen(a);
    auto len_b = wcslen(b);

    auto len_c = 0;
    if (c) len_c = wcslen(c);
    
    auto len_d = 0;
    if (d) len_d = wcslen(d);
    
    wchar_t *result = (wchar_t *)malloc((len_a + len_b + len_c + len_d + 1) * 2);
    memcpy(result, a, len_a*2);
    memcpy(result + len_a, b, len_b*2);

    if (c) memcpy(result + len_a + len_b, c, len_c * 2);
    if (d) memcpy(result + len_a + len_b + len_c, d, len_d * 2);
        
    result[len_a + len_b + len_c + len_d] = 0;

    return result;
}

typedef void (*Visit_Proc_W)(wchar_t *short_name, wchar_t *full_name, Version_Data *data);
bool visit_files_w(wchar_t *dir_name, Version_Data *data, Visit_Proc_W proc) {

    // Visit everything in one folder (non-recursively). If it's a directory
    // that doesn't start with ".", call the visit proc on it. The visit proc
    // will see if the filename conforms to the expected versioning pattern.

    auto wildcard_name = concat(dir_name, L"\\*");
    defer { free(wildcard_name); };
     
    WIN32_FIND_DATAW find_data;
    auto handle = FindFirstFileW(wildcard_name, &find_data);
    if (handle == INVALID_HANDLE_VALUE) return false;

    while (true) {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (find_data.cFileName[0] != '.')) {
            auto full_name = concat(dir_name, L"\\", find_data.cFileName);
            defer { free(full_name); };
          
            proc(find_data.cFileName, full_name, data);
        }
        
        auto success = FindNextFileW(handle, &find_data);
        if (!success) break;
    }

    FindClose(handle);
    
    return true;
}

wchar_t *read_from_the_registry(HKEY key, wchar_t *value_name) {
    // Returns NULL if read failed.
    // Otherwise returns a wide string allocated via 'malloc'.
    
    //
    // If the registry data changes between the first and second calls to RegQueryValueExW,
    // we may fail to get the entire key, even though it told us initially that our buffer length
    // would be big enough. The only solution is to keep looping until we don't fail.
    //

    DWORD required_length;
    auto rc = RegQueryValueExW(key, value_name, NULL, NULL, NULL, &required_length);
    if (rc != 0)  return NULL;

    wchar_t *value;
    DWORD length;
    while (1) {
        length = required_length + 2;  // The +2 is for the maybe optional zero later on. Probably we are over-allocating.
        value = (wchar_t *)malloc(length + 2);  // This second +2 is for crazy situations where there are race conditions or the API doesn't do what we want!
        if (!value) return NULL;

        DWORD type;
        rc = RegQueryValueExW(key, value_name, NULL, &type, (LPBYTE)value, &length);  // We know that version is zero-terminated...
        if (rc == ERROR_MORE_DATA) {
            free(value);
            required_length = length;
            continue;
        }

        if ((rc != 0) || (type != REG_SZ)) {
            // REG_SZ because we only accept strings here!
            free(value);
            return NULL;
        }
        
        break;
    }
    
    // The documentation says that if the string for some reason was not stored
    // with zero-termination, we need to manually terminate it. Sigh!!

    auto num_wchars = length / 2;
    value[num_wchars] = 0;  // If the string was already zero-terminated, this just puts an extra 0 after (since that 0 was counted in 'length'). If it wasn't, this puts a 0 after the nonzero characters we got.
    
    return value;
}

void win10_best(wchar_t *short_name, wchar_t *full_name, Version_Data *data) {
    // Find the Windows 10 subdirectory with the highest version number.
    
    int i0, i1, i2, i3;
    auto success = swscanf_s(short_name, L"%d.%d.%d.%d", &i0, &i1, &i2, &i3);
    if (success < 4) return;

    if (i0 < data->best_version[0]) return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1]) return;
        else if (i1 == data->best_version[1]) {
            if (i2 < data->best_version[2]) return;
            else if (i2 == data->best_version[2]) {
                if (i3 < data->best_version[3]) return;
            }
        }
    }

    // we have to copy_string and free here because visit_files free's the full_name string
    // after we execute this function, so Win*_Data would contain an invalid pointer.
    if (data->best_name) free(data->best_name);
    data->best_name = _wcsdup(full_name);
            
    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
        data->best_version[2] = i2;
        data->best_version[3] = i3;
    }
}

void win8_best(wchar_t *short_name, wchar_t *full_name, Version_Data *data) {
    // Find the Windows 8 subdirectory with the highest version number.

    int i0, i1;
    auto success = swscanf_s(short_name, L"winv%d.%d", &i0, &i1);
    if (success < 2) return;

    if (i0 < data->best_version[0]) return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1]) return;
    }

    // we have to copy_string and free here because visit_files free's the full_name string
    // after we execute this function, so Win*_Data would contain an invalid pointer.
    if (data->best_name) free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
    }
}

void find_windows_kit_root(Find_Result *result) {
    // Information about the Windows 10 and Windows 8 development kits
    // is stored in the same place in the registry. We open a key
    // to that place, first checking preferntially for a Windows 10 kit,
    // then, if that's not found, a Windows 8 kit.
    
    HKEY main_key;

    auto rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
                            0, KEY_QUERY_VALUE | KEY_WOW64_32KEY | KEY_ENUMERATE_SUB_KEYS, &main_key);
    if (rc != S_OK) return;
    defer { RegCloseKey(main_key); };

    // Look for a Windows 10 entry.
    auto windows10_root = read_from_the_registry(main_key, L"KitsRoot10");
    if (windows10_root) {
        defer { free(windows10_root); };
        Version_Data data = {0};
        auto windows10_lib = concat(windows10_root, L"Lib");
        defer { free(windows10_lib); };
        
        visit_files_w(windows10_lib, &data, win10_best);
        if (data.best_name) {
            result->windows_sdk_version = 10;
            result->windows_sdk_root = data.best_name;
            return;
        }
    }

    // Look for a Windows 8 entry.
    auto windows8_root = read_from_the_registry(main_key, L"KitsRoot81");

    if (windows8_root) {
        defer { free(windows8_root); };
        
        auto windows8_lib = concat(windows8_root, L"Lib");
        defer { free(windows8_lib); };

        Version_Data data = {0};
        visit_files_w(windows8_lib, &data, win8_best);
        if (data.best_name) {
            result->windows_sdk_version = 8;
            result->windows_sdk_root = data.best_name;
            return;
        }
    }

    // If we get here, we failed to find anything.
}


bool find_visual_studio_2017_by_fighting_through_microsoft_craziness(Find_Result *result) {
    // The name of this procedure is kind of cryptic. Its purpose is (no it's not, it's glorious)
    // to fight through Microsoft craziness. The things that the fine
    // Visual Studio team want you to do, JUST TO FIND A SINGLE FOLDER
    // THAT EVERYONE NEEDS TO FIND, are ridiculous garbage.

    // For earlier versions of Visual Studio, you'd find this information in the registry,
    // similarly to the Windows Kits above. But no, now it's the future, so to ask the
    // question "Where is the Visual Studio folder?" you have to do a bunch of COM object
    // instantiation, enumeration, and querying. (For extra bonus points, try doing this in
    // a new, underdeveloped programming language where you don't have COM routines up
    // and running yet. So fun.)
    //
    // If all this COM object instantiation, enumeration, and querying doesn't give us
    // a useful result, we drop back to the registry-checking method.
    
    auto rc = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    // "Subsequent valid calls return false." So ignore false.
    // if rc != S_OK  return false;

    GUID my_uid                   = {0x42843719, 0xDB4C, 0x46C2, {0x8E, 0x7C, 0x64, 0xF1, 0x81, 0x6E, 0xFD, 0x5B}};
    GUID CLSID_SetupConfiguration = {0x177F0C4A, 0x1CD3, 0x4DE7, {0xA3, 0x2C, 0x71, 0xDB, 0xBB, 0x9F, 0xA3, 0x6D}};

    ISetupConfiguration *config = NULL;
    auto hr = CoCreateInstance(CLSID_SetupConfiguration, NULL, CLSCTX_INPROC_SERVER, my_uid, (void **)&config);
    if (hr != 0)  return false;
    defer { config->Release(); };

    IEnumSetupInstances *instances = NULL;
    hr = config->EnumInstances(&instances);
    if (hr != 0)     return false;
    if (!instances)  return false;
    defer { instances->Release(); };

    while (1) {
        ULONG found = 0;
        ISetupInstance *instance = NULL;
        auto hr = instances->Next(1, &instance, &found);
        if (hr != S_OK) break;

        defer { instance->Release(); };
        
        BSTR bstr_inst_path;
        hr = instance->GetInstallationPath(&bstr_inst_path);
        if (hr != S_OK)  continue;
        defer { SysFreeString(bstr_inst_path); };
        
        auto tools_filename = concat(bstr_inst_path, L"\\VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt");
        defer { free(tools_filename); };

        FILE *f = nullptr;
        auto open_result = _wfopen_s(&f, tools_filename, L"rt");
        if (open_result != 0) continue;
        if (!f) continue;
        defer { fclose(f); };

        LARGE_INTEGER tools_file_size;
        auto file_handle = (HANDLE)_get_osfhandle(_fileno(f));
        BOOL success = GetFileSizeEx(file_handle, &tools_file_size);
        if (!success) continue;

        auto version_bytes = (tools_file_size.QuadPart + 1) * 2;  // Warning: This multiplication by 2 presumes there is no variable-length encoding in the wchars (wacky characters in the file could betray this expectation).
        wchar_t *version = (wchar_t *)malloc(version_bytes);
        defer { free(version); };

        auto read_result = fgetws(version, version_bytes, f);
        if (!read_result) continue;

        auto version_tail = wcschr(version, '\n');
        if (version_tail)  *version_tail = 0;  // Stomp the data, because nobody cares about it.

        auto library_path = concat(bstr_inst_path, L"\\VC\\Tools\\MSVC\\", version, L"\\lib\\x64");
        auto library_file = concat(library_path, L"\\vcruntime.lib");  // @Speed: Could have library_path point to this string, with a smaller count, to save on memory flailing!

        if (os_file_exists(library_file)) {
            auto link_exe_path = concat(bstr_inst_path, L"\\VC\\Tools\\MSVC\\", version, L"\\bin\\Hostx64\\x64");
            result->vs_exe_path     = link_exe_path;
            result->vs_library_path = library_path;
            return true;
        }

        /*
           Ryan Saunderson said:
           "Clang uses the 'SetupInstance->GetInstallationVersion' / ISetupHelper->ParseVersion to find the newest version 
           and then reads the tools file to define the tools path - which is definitely better than what i did."
 
           So... @Incomplete: Should probably pick the newest version...
        */
    }

    // If we get here, we didn't find Visual Studio 2017. Try earlier versions.
    return false;
}

void find_visual_studio_by_fighting_through_microsoft_craziness(Find_Result *result) {
    bool found_visual_studio_2017 = find_visual_studio_2017_by_fighting_through_microsoft_craziness(result);
    if (found_visual_studio_2017) return;

    HKEY vs7_key;
    auto rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &vs7_key);

    if (rc != S_OK)  return;
    defer { RegCloseKey(vs7_key); };

    // Hardcoded search for 4 prior Visual Studio versions. Is there something better to do here?
    wchar_t *versions[] = { L"14.0", L"12.0", L"11.0", L"10.0" };
    const int NUM_VERSIONS = sizeof(versions) / sizeof(versions[0]);

    for (int i = 0; i < NUM_VERSIONS; i++) {
        auto v = versions[i];

        auto buffer = read_from_the_registry(vs7_key, v);
        if (!buffer) continue;

        defer { free(buffer); };
        
        auto lib_path = concat(buffer, L"VC\\Lib\\amd64");

        // Check to see whether a vcruntime.lib actually exists here.
        auto vcruntime_filename = concat(lib_path, L"\\vcruntime.lib");
        defer { free(vcruntime_filename); };

        if (os_file_exists(vcruntime_filename)) {
            result->vs_exe_path     = concat(buffer, L"VC\\bin\\amd64");
            result->vs_library_path = lib_path;
            return;
        }
        
        free(lib_path);
    }

    // If we get here, we failed to find anything.
}

extern "C" char* wchar_to_char (wchar_t* wide_string) {
    // probably could have just use wcslen() but whatever
    wchar_t* i_am_very_annoyed_with_this = wide_string;
    int len = 1;
    while (*i_am_very_annoyed_with_this != L'\0') { i_am_very_annoyed_with_this++; len++; }

    char *str = (char*)calloc(len, sizeof(char));
    wcstombs(str, wide_string, len);
    return str;
}

// try to deduce the path for the headers on windows with serious pointer abuse
// this is insanely hacky. I have no idea what the hell COM objects are, so I just try to deduce things
wchar_t* get_include_path_root (Find_Result result) {
    int sdk_root_len = wcslen(result.windows_sdk_root) + 1; // account for null terminator

    wchar_t* sdk_root = (wchar_t*)malloc(sdk_root_len * sizeof(wchar_t));
    memcpy(sdk_root, result.windows_sdk_root, sdk_root_len * sizeof(wchar_t));

    wchar_t* weird_number = sdk_root + sdk_root_len;
    while (*weird_number != L'\\') weird_number--; 

    wchar_t* real_sdk_root = weird_number;
    do { real_sdk_root--; } while (*real_sdk_root != L'\\');
    *real_sdk_root = L'\0';

    wchar_t* include_path_root = concat(sdk_root, L"\\Include", weird_number);
    return include_path_root;
}

wchar_t* get_vs_include_path (Find_Result result) {
    int vs_lib_len = wcslen(result.vs_library_path); 

    wchar_t* vs_include = (wchar_t*)malloc(vs_lib_len * sizeof(wchar_t));
    memcpy(vs_include, result.vs_library_path, vs_lib_len * sizeof(wchar_t));

    vs_include[vs_lib_len - strlen("\\lib\\x64")] = '\0';
    return concat(vs_include, L"\\include");
}

extern "C" wchar_t* get_include_paths (Find_Result result) {
    wchar_t* ucrt_include = concat(get_include_path_root(result), L"\\ucrt", L";");
    wchar_t* um_include = concat(get_include_path_root(result), L"\\um", L";");
    wchar_t* shared_include = concat(get_include_path_root(result), L"\\shared", L";");
    wchar_t* vs_include = get_vs_include_path(result);
    return concat(ucrt_include, um_include, shared_include, vs_include);
}

extern "C" wchar_t* get_lib_paths (Find_Result result) {
    wchar_t* ucrt_libs = concat(result.windows_sdk_ucrt_library_path, L";");
    wchar_t* um_libs = concat(result.windows_sdk_um_library_path, L";");
    wchar_t* vs_libs = result.vs_library_path;
    return concat(ucrt_libs, um_libs, vs_libs);
}

extern "C" wchar_t* get_cl_exe_path (Find_Result result) {
    return concat(result.vs_exe_path, L"\\cl.exe");
}

extern "C" Find_Result find_visual_studio_and_windows_sdk() {
    Find_Result result;

    find_windows_kit_root(&result);

    if (result.windows_sdk_root) {
        result.windows_sdk_um_library_path   = concat(result.windows_sdk_root, L"\\um\\x64");
        result.windows_sdk_ucrt_library_path = concat(result.windows_sdk_root, L"\\ucrt\\x64");
    }

    find_visual_studio_by_fighting_through_microsoft_craziness(&result);

    return result;
}