using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading.Tasks;

namespace Statman.Util
{
    [DataContract]
    internal class HM3Options
    {
        [DataMember]
        public bool Cheats { get; set; } = false;

        [DataMember]
        public bool UnlimitedSaves { get; set; } = false;

        [DataMember]
        public bool Hitman2016Mode { get; set; } = false;

        [DataMember]
        public bool Hitman2016DebugMode { get; set; } = false;

        [DataMember]
        public bool Overlay { get; set; } = true;
    }

    [DataContract]
    internal class StatmanOptions
    {
        [DataMember]
        public bool DarkMode { get; set; } = false;

        [DataMember]
        public HM3Options HM3 { get; set; } = new HM3Options();
    }

    class Options
    {
        private static string m_OptionsPath;
        private static StatmanOptions m_Options;

        static Options()
        {
            m_OptionsPath = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "statman.json");
            m_Options = ParseOptions();
        }

        private static StatmanOptions ParseOptions() 
        {
            try
            {
                if (File.Exists(m_OptionsPath))
                {
                    var s_Json = File.ReadAllText(m_OptionsPath);

                    using (var s_Stream = new MemoryStream(Encoding.UTF8.GetBytes(s_Json)))
                    {
                        var s_Serializer = new DataContractJsonSerializer(typeof (StatmanOptions));
                        var s_Options = s_Serializer.ReadObject(s_Stream) as StatmanOptions;

                        if (s_Options == null)
                            return new StatmanOptions();

                        if (s_Options.HM3 == null)
                            s_Options.HM3 = new HM3Options();

                        return s_Options;
                    }
                }
            }
            catch (Exception) 
            {
                return new StatmanOptions();
            }

            return new StatmanOptions();
        }

        private static void WriteOptions()
        {
            try
            {
                using (var s_Stream = File.Create(m_OptionsPath))
                {
                    var s_Serializer = new DataContractJsonSerializer(typeof (StatmanOptions));
                    s_Serializer.WriteObject(s_Stream, m_Options);
                }
            }
            catch (Exception)
            {
                // No-op.
            }
        }

        public static StatmanOptions Get() { return m_Options; }

        public static void Update(Action<StatmanOptions> updater)
        {
            lock (m_Options)
            {
                updater(m_Options);
                WriteOptions();
            }
        }
    }
}
