using System;
using System.IO;

namespace Statman.Util
{
    /// <summary>
    /// Resolves the folders used next to Statman.exe. User configuration lives
    /// in a "config" subfolder so the application folder only contains the
    /// executable, its dependencies and the in-game module.
    /// </summary>
    static class Paths
    {
        private const string c_ConfigFolder = "config";

        // Folder that contains Statman.exe and HM3.dll.
        public static string BaseDirectory
        {
            get { return AppDomain.CurrentDomain.BaseDirectory; }
        }

        // Folder that contains statman.json, HM3Overlay.json and HM3_locations.json.
        public static string ConfigDirectory
        {
            get { return Path.Combine(BaseDirectory, c_ConfigFolder); }
        }

        public static string ConfigFile(string p_FileName)
        {
            return Path.Combine(ConfigDirectory, p_FileName);
        }

        /// <summary>
        /// Moves a configuration file that older builds kept next to
        /// Statman.exe into the config folder. A file that already exists in the
        /// config folder is replaced so saved settings survive an upgrade.
        /// </summary>
        public static void MigrateLegacyConfig(string p_FileName)
        {
            try
            {
                var s_LegacyPath = Path.Combine(BaseDirectory, p_FileName);

                if (!File.Exists(s_LegacyPath))
                    return;

                Directory.CreateDirectory(ConfigDirectory);

                var s_TargetPath = ConfigFile(p_FileName);

                if (File.Exists(s_TargetPath))
                    File.Delete(s_TargetPath);

                File.Move(s_LegacyPath, s_TargetPath);

                Log.Info(string.Format("Moved {0} into the config folder.", p_FileName));
            }
            catch (Exception s_Exception)
            {
                // A failed migration must not prevent startup; the old file is
                // left in place and the defaults are used instead.
                Log.Exception("Failed to migrate " + p_FileName, s_Exception);
            }
        }
    }
}
