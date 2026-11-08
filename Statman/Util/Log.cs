using System;
using System.IO;

namespace Statman.Util
{
    /// <summary>
    /// Append-only diagnostic log named statman.log next to Statman.exe.
    /// INFO/ERROR are always written; VERBOSE lines are written when verbose
    /// logging is enabled, either through the "Verbose logging" menu option or
    /// the STATMAN_VERBOSE=1 environment variable.
    /// </summary>
    static class Log
    {
        private const long MaxLogSize = 1024 * 1024;

        private static readonly object m_Lock = new object();

        private static readonly string m_Path = Path.Combine(
            AppDomain.CurrentDomain.BaseDirectory, "statman.log");

        private static readonly bool m_EnvironmentVerbose = ReadVerboseFlag();
        private static volatile bool m_RuntimeVerbose;

        // Folder that contains statman.log and hm3.log.
        public static string DirectoryPath
        {
            get { return Path.GetDirectoryName(m_Path); }
        }

        // Runtime switch controlled by the "Verbose logging" menu option.
        public static bool VerboseEnabled
        {
            get { return m_EnvironmentVerbose || m_RuntimeVerbose; }
            set { m_RuntimeVerbose = value; }
        }

        public static void Info(string p_Message)
        {
            Write("INFO", p_Message);
        }

        public static void Verbose(string p_Message)
        {
            if (VerboseEnabled)
                Write("VERBOSE", p_Message);
        }

        public static void Exception(string p_Context, Exception p_Exception)
        {
            Write("ERROR", string.Format("{0}{1}{1}{2}", p_Context, Environment.NewLine, p_Exception));
        }

        private static bool ReadVerboseFlag()
        {
            try
            {
                return Environment.GetEnvironmentVariable("STATMAN_VERBOSE") == "1";
            }
            catch
            {
                return false;
            }
        }

        private static void Write(string p_Level, string p_Message)
        {
            try
            {
                lock (m_Lock)
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(m_Path));

                    RotateIfNeeded();

                    File.AppendAllText(m_Path, string.Format("[{0:yyyy-MM-dd HH:mm:ss.fff}] [{1}] {2}{3}",
                        DateTime.Now, p_Level, p_Message, Environment.NewLine));
                }
            }
            catch
            {
                // Logging must never take the application down.
            }
        }

        private static void RotateIfNeeded()
        {
            try
            {
                var s_Info = new FileInfo(m_Path);

                if (!s_Info.Exists || s_Info.Length < MaxLogSize)
                    return;

                var s_Backup = m_Path + ".old";

                if (File.Exists(s_Backup))
                    File.Delete(s_Backup);

                File.Move(m_Path, s_Backup);
            }
            catch
            {
                // If rotation fails we keep appending to the existing file.
            }
        }
    }
}
