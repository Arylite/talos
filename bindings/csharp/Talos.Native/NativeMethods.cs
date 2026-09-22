using System;
using System.Runtime.InteropServices;

namespace Talos.Native;

[StructLayout(LayoutKind.Sequential)]
internal struct TalosConfig
{
    public int DisableChecks;
}

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void TalosLogCallback(int level, IntPtr message, IntPtr userData);

[StructLayout(LayoutKind.Sequential)]
internal struct TalosCheckResult
{
    public IntPtr Name;
    public int Status;
    public int State;
}

[StructLayout(LayoutKind.Sequential)]
internal struct TalosScanReport
{
    internal const int MaxChecks = 16;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaxChecks)]
    public TalosCheckResult[] Checks;

    public uint Count;
}

/// <summary>
/// Raw P/Invoke declarations mirroring the talos.h C ABI. This layer
/// performs no logic of its own and must stay independent of any
/// particular engine or runtime.
/// </summary>
internal static class NativeMethods
{
    private const string LibraryName = "talos";

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_init(IntPtr config, out IntPtr outContext);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void talos_shutdown(IntPtr context);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr talos_version();

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void talos_set_log_callback(IntPtr context, TalosLogCallback? callback, IntPtr userData);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_debugger_present(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_parent_process(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_hardware_breakpoints(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_loaded_modules(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_hidden_threads(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_api_hooks(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_check_self_integrity(IntPtr context, out int outState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int talos_scan(IntPtr context, out TalosScanReport outReport);
}
