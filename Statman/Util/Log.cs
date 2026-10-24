using System;
using System.IO;

namespace Statman.Util
{
    static class Log
    {
        private static readonly object m_Lock = new object();

        private static readonly string m_Path = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Statman", "statman.log");

        public static void Exception(string p_Context, Exception p_Exception)
        {
            try
            {
                lock (m_Lock)
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(m_Path));

                    File.AppendAllText(m_Path, string.Format("[{0:yyyy-MM-dd HH:mm:ss}] {1}{2}{3}{2}{2}",
                        DateTime.Now, p_Context, Environment.NewLine, p_Exception));
                }
            }
            catch
            {
            }
        }
    }
}
