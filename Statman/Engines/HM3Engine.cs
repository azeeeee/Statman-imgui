using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using Statman.Engines.HM3;
using Statman.Engines.HM3.Controls;
using Statman.Network;
using Statman.Util;
using Statman.Util.Injection;

namespace Statman.Engines
{
    class HM3Engine : IEngine
    {
        private static readonly Dictionary<string, Tuple<string, string, bool>> m_SceneNames = new Dictionary
            <string, Tuple<string, string, bool>>()
        {
            { "hitmanbloodmoney", new Tuple<string, string, bool>("Main Menu", "Menu", false) },
            { "scenes\\hitmanbloodmoney.gms", new Tuple<string, string, bool>("Main Menu", "Menu", false) },

            { "saveandcontinue", new Tuple<string, string, bool>("Save & Continue", "Menu", false) },

            { "hideout\\hideout_main", new Tuple<string, string, bool>("Hideout", "In-Game", true) },
            { "scenes\\hideout\\hideout_main.gms", new Tuple<string, string, bool>("Hideout", "In-Game", true) },

            { "m00\\m00_intro", new Tuple<string, string, bool>("Death of a Showman", "Intro", false) },
            { "m00\\m00_main", new Tuple<string, string, bool>("Death of a Showman", "In-Game", true) },
            { "scenes\\m00\\m00_main.gms", new Tuple<string, string, bool>("Death of a Showman", "In-Game", true) },
            { "m00\\m00_news", new Tuple<string, string, bool>("Death of a Showman", "News", false) },
            { "m00\\m00_albino", new Tuple<string, string, bool>("Death of a Showman", "Story Intro", false) },

            { "m01\\m01_premission", new Tuple<string, string, bool>("A Vintage Year", "Pre-Mission", false) },
            { "m01\\m01_main", new Tuple<string, string, bool>("A Vintage Year", "In-Game", true) },
            { "scenes\\m01\\m01_main.gms", new Tuple<string, string, bool>("A Vintage Year", "In-Game", true) },
            { "m01\\m01_postmission", new Tuple<string, string, bool>("A Vintage Year", "Post-Mission", false) },
            { "m01\\m01_news", new Tuple<string, string, bool>("A Vintage Year", "News", false) },

            { "m03\\m03_premission", new Tuple<string, string, bool>("Curtains Down", "Pre-Mission", false) },
            { "m03\\m03_main", new Tuple<string, string, bool>("Curtains Down", "In-Game", true) },
            { "scenes\\m03\\m03_main.gms", new Tuple<string, string, bool>("Curtains Down", "In-Game", true) },
            { "m03\\m03_postmission", new Tuple<string, string, bool>("Curtains Down", "Post-Mission", false) },
            { "m03\\m03_news", new Tuple<string, string, bool>("Curtains Down", "News", false) },

            { "m04\\m04_premission", new Tuple<string, string, bool>("Flatline", "Pre-Mission", false) },
            { "m04\\m04_main", new Tuple<string, string, bool>("Flatline", "In-Game", true) },
            { "scenes\\m04\\m04_main.gms", new Tuple<string, string, bool>("Flatline", "In-Game", true) },
            { "m04\\m04_postmission", new Tuple<string, string, bool>("Flatline", "Post-Mission", false) },
            { "m04\\m04_news", new Tuple<string, string, bool>("Flatline", "News", false) },

            { "m05\\m05_premission", new Tuple<string, string, bool>("A New Life", "Pre-Mission", false) },
            { "m05\\m05_main", new Tuple<string, string, bool>("A New Life", "In-Game", true) },
            { "scenes\\m05\\m05_main.gms", new Tuple<string, string, bool>("A New Life", "In-Game", true) },
            { "m05\\m05_postmission", new Tuple<string, string, bool>("A New Life", "Post-Mission", false) },
            { "m05\\m05_news", new Tuple<string, string, bool>("A New Life", "News", false) },

            { "m06\\m06_premission", new Tuple<string, string, bool>("Murder of the Crows", "Pre-Mission", false) },
            { "m06\\m06_main", new Tuple<string, string, bool>("Murder of the Crows", "In-Game", true) },
            { "scenes\\m06\\m06_main.gms", new Tuple<string, string, bool>("Murder of the Crows", "In-Game", true) },
            { "m06\\m06_postmission", new Tuple<string, string, bool>("Murder of the Crows", "Post-Mission", false) },
            { "m06\\m06_news", new Tuple<string, string, bool>("Murder of the Crows", "News", false) },
            
            { "m02\\m02_premission", new Tuple<string, string, bool>("You Better Watch Out", "Pre-Mission", false) },
            { "m02\\m02_main", new Tuple<string, string, bool>("You Better Watch Out", "In-Game", true) },
            { "scenes\\m02\\m02_main.gms", new Tuple<string, string, bool>("You Better Watch Out", "In-Game", true) },
            { "m02\\m02_postmission", new Tuple<string, string, bool>("You Better Watch Out", "Post-Mission", false) },
            { "m02\\m02_news", new Tuple<string, string, bool>("You Better Watch Out", "News", false) },

            { "m08\\m08_premission", new Tuple<string, string, bool>("Death on the Mississippi", "Pre-Mission", false) },
            { "m08\\m08_main", new Tuple<string, string, bool>("Death on the Mississippi", "In-Game", true) },
            { "scenes\\m08\\m08_main.gms", new Tuple<string, string, bool>("Death on the Mississippi", "In-Game", true) },
            { "m08\\m08_postmission", new Tuple<string, string, bool>("Death on the Mississippi", "Post-Mission", false) },
            { "m08\\m08_news", new Tuple<string, string, bool>("Death on the Mississippi", "News", false) },

            { "m09\\m09_premission", new Tuple<string, string, bool>("Till Death Do Us Part", "Pre-Mission", false) },
            { "m09\\m09_main", new Tuple<string, string, bool>("Till Death Do Us Part", "In-Game", true) },
            { "scenes\\m09\\m09_main.gms", new Tuple<string, string, bool>("Till Death Do Us Part", "In-Game", true) },
            { "m09\\m09_postmission", new Tuple<string, string, bool>("Till Death Do Us Part", "Post-Mission", false) },
            { "m09\\m09_news", new Tuple<string, string, bool>("Till Death Do Us Part", "News", false) },

            { "m10\\m10_premission", new Tuple<string, string, bool>("A House of Cards", "Pre-Mission", false) },
            { "m10\\m10_main", new Tuple<string, string, bool>("A House of Cards", "In-Game", true) },
            { "scenes\\m10\\m10_main.gms", new Tuple<string, string, bool>("A House of Cards", "In-Game", true) },
            { "m10\\m10_postmission", new Tuple<string, string, bool>("A House of Cards", "Post-Mission", false) },
            { "m10\\m10_news", new Tuple<string, string, bool>("A House of Cards", "News", false) },

            { "m11\\m11_premission", new Tuple<string, string, bool>("A Dance with The Devil", "Pre-Mission", false) },
            { "m11\\m11_main", new Tuple<string, string, bool>("A Dance with The Devil", "In-Game", true) },
            { "scenes\\m11\\m11_main.gms", new Tuple<string, string, bool>("A Dance with The Devil", "In-Game", true) },
            { "m11\\m11_postmission", new Tuple<string, string, bool>("A Dance with The Devil", "Post-Mission", false) },
            { "m11\\m11_news", new Tuple<string, string, bool>("A Dance with The Devil", "News", false) },

            { "m12\\m12_premission", new Tuple<string, string, bool>("Amendment XXV", "Pre-Mission", false) },
            { "m12\\m12_main", new Tuple<string, string, bool>("Amendment XXV", "In-Game", true) },
            { "scenes\\m12\\m12_main.gms", new Tuple<string, string, bool>("Amendment XXV", "In-Game", true) },
            { "m12\\m12_postmission", new Tuple<string, string, bool>("Amendment XXV", "Post-Mission", false) },
            { "m12\\m12_news", new Tuple<string, string, bool>("Amendment XXV", "News", false) },

            { "m13\\m13_intro", new Tuple<string, string, bool>("Requiem", "Intro", false) },
            { "m13\\m13_main", new Tuple<string, string, bool>("Requiem", "In-Game", true) },
            { "scenes\\m13\\m13_main.gms", new Tuple<string, string, bool>("Requiem", "In-Game", true) },
        };

        public string Name { get { return "H3"; } }
        public bool Active { get; private set; }
        public IntPtr GameModuleBase { get; private set; }

        // Live per-weapon kill counts for the current level, keyed by item ID.
        // They are pushed by the native module's "LW" message and read by StatTracker.
        private readonly object m_LiveWeaponLock = new object();
        private Dictionary<int, int> m_LiveWeaponKills = new Dictionary<int, int>();

        // Returns a copy so callers can use the data without holding the lock.
        public Dictionary<int, int> GetLiveWeaponKills()
        {
            lock (m_LiveWeaponLock)
                return new Dictionary<int, int>(m_LiveWeaponKills);
        }

        // Parses the native "itemId:count,itemId:count" payload. An empty
        // payload clears the snapshot, which happens on every scene load.
        public void SetLiveWeaponKills(string p_Data)
        {
            var s_Kills = new Dictionary<int, int>();

            if (!string.IsNullOrEmpty(p_Data))
            {
                foreach (var s_Entry in p_Data.Split(','))
                {
                    var s_Parts = s_Entry.Split(':');

                    if (s_Parts.Length == 2 &&
                        int.TryParse(s_Parts[0], out var s_ItemId) &&
                        int.TryParse(s_Parts[1], out var s_Count))
                    {
                        s_Kills[s_ItemId] = s_Count;
                    }
                }
            }

            lock (m_LiveWeaponLock)
                m_LiveWeaponKills = s_Kills;
        }

        // Returns true when the game itself has any built-in trainer flag active.
        // This stays true even if the main "Enable cheats" flag has been turned off
        // while individual cheats are still enabled.
        public bool CheatsActive()
        {
            // The injected module resolves all cheat flags by pattern scanning,
            // so it is the only reliable source across game builds. Before the
            // module reports, assume no cheats.
            return m_HasCheatsActiveFromModule && m_CheatsActiveFromModule;
        }

        /// <summary>
        /// Total NPC count of the current level, reported by the injected module.
        /// </summary>
        public int TotalLevelNpcs
        {
            get { return m_TotalLevelNpcs; }
        }

        public ProcessMemoryReader Reader { get; private set; }

        public StatTracker StatTracker { get; private set; }
        public TimeTracker TimeTracker { get; private set; }

        public MainControl Control { get; private set; }

        public string CurrentScene { get; private set; }
        public string CurrentLevel { get; private set; }
        public string CurrentLevelScene { get; private set; }
        public bool InGame { get; private set; }

        private Process m_GameProcess;
        private Injector m_Injector;

        private uint m_SkipUpdates;

        // Authoritative cheat state reported by the injected module. The module
        // knows which time-scale changes came from the game itself (the dying
        // slow motion), which this side cannot detect on its own.
        private volatile bool m_CheatsActiveFromModule;
        private volatile bool m_HasCheatsActiveFromModule;

        // Total NPC count of the current level, reported by the injected module
        // so this side never has to read build-specific addresses.
        private volatile int m_TotalLevelNpcs;

        // The game is a Steam DRM packed executable that decrypts itself on startup.
        // Injecting before that finishes can corrupt the process, so wait until the
        // executable has demonstrably unpacked itself before touching it.
        private const int ReadyTimeoutMs = 3000;

        private DateTime m_AttachStarted;
        private DateTime m_GameStartedAt;
        private DateTime m_LastProcessProbe;
        private string m_LastReadinessLog;
        private bool m_EntryPointChecked;
        private bool m_EntryPointLooksPacked;
        private bool m_ModuleListLogged;
        private bool m_HasD3D9;
        private bool m_HasBink;
        private bool m_HasWindow;
        private string m_ModuleError;

        private readonly List<Control> m_MenuItems; 

        public HM3Engine()
        {
            m_MenuItems = new List<Control>();
            Active = false;
        }

        private void InitMenuItems()
        {
            MainApp.MainWindow.Dispatcher.Invoke(() =>
            {
                var s_EnableCheats = new MenuItem()
                {
                    Header = "Enable Cheats",
                    IsCheckable = true,
                    IsChecked = Options.Get().HM3.Cheats,
                };

                s_EnableCheats.Click += OnEnableCheats;

                var s_UnlimitedSaves = new MenuItem()
                {
                    Header = "Unlimited Saves",
                    IsCheckable = true,
                    IsChecked = Options.Get().HM3.UnlimitedSaves,
                };

                s_UnlimitedSaves.Click += OnUnlimitedSaves;

                var s_Hitman2016 = new MenuItem()
                {
                    Header = "Hitman 2016 Mode",
                    IsCheckable = true,
                    IsChecked = Options.Get().HM3.Hitman2016Mode,
                };

                s_Hitman2016.Click += OnHitman2016Mode;
                
                var s_ReloadHitman2016 = new MenuItem()
                {
                    Header = "Reload locations",
                };
                
                s_ReloadHitman2016.Click += OnReloadHitman2016;

                var s_Hitman2016Debug = new MenuItem()
                {
                    Header = "Hitman 2016 Mode Debug",
                    IsCheckable = true,
                    IsChecked = Options.Get().HM3.Hitman2016DebugMode,
                };

                s_Hitman2016Debug.Click += OnHitman2016DebugMode;

                var s_PredictSpecialRatings = new MenuItem()
                {
                    Header = "Predict Special Ratings [Experimental]",
                    IsCheckable = true,
                    IsChecked = Options.Get().HM3.PredictSpecialRatings,
                };

                s_PredictSpecialRatings.Click += OnPredictSpecialRatings;

                var s_VerboseLogging = new MenuItem()
                {
                    Header = "Verbose logging (diagnostics)",
                    IsCheckable = true,
                    IsChecked = Options.Get().VerboseLogging,
                    ToolTip = "Write detailed diagnostic lines to statman.log and hm3.log. Useful when reporting a problem.",
                };

                s_VerboseLogging.Click += OnVerboseLogging;

                var s_OpenLogFolder = new MenuItem()
                {
                    Header = "Open log folder",
                    ToolTip = "Open the folder containing Statman.exe, statman.log and hm3.log.",
                };

                s_OpenLogFolder.Click += OnOpenLogFolder;

                var s_IngameOverlay = new MenuItem()
                {
                    Header = "In-game Overlay",
                    IsCheckable = true,
                    IsChecked = Options.Get().HM3.Overlay,
                };

                s_IngameOverlay.Click += OnIngameOverlay;

                m_MenuItems.Clear();
                m_MenuItems.Add(s_EnableCheats);
                m_MenuItems.Add(s_UnlimitedSaves);
                m_MenuItems.Add(s_IngameOverlay);
                m_MenuItems.Add(s_Hitman2016);
                m_MenuItems.Add(s_ReloadHitman2016);
                m_MenuItems.Add(s_Hitman2016Debug);
                m_MenuItems.Add(s_PredictSpecialRatings);
                m_MenuItems.Add(new Separator());
                m_MenuItems.Add(s_VerboseLogging);
                m_MenuItems.Add(s_OpenLogFolder);
            });
        }

        private void OnEnableCheats(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null) 
                return;

            SendMessage("EC", s_MenuItem.IsChecked ? "true" : "false");
            Options.Update(opt => opt.HM3.Cheats = s_MenuItem.IsChecked);
        }

        private void OnUnlimitedSaves(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null)
                return;

            SendMessage("US", s_MenuItem.IsChecked ? "true" : "false");
            Options.Update(opt => opt.HM3.UnlimitedSaves = s_MenuItem.IsChecked);
        }

        private void OnHitman2016Mode(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null)
                return;

            SendMessage("H2", s_MenuItem.IsChecked ? "true" : "false");
            Options.Update(opt => opt.HM3.Hitman2016Mode = s_MenuItem.IsChecked);
        }

        private void ReloadHitman2016Data()
        {
            try
            {
                Paths.MigrateLegacyConfig("HM3_locations.json");

                var s_Hitman2016Data = System.IO.File.ReadAllText(Paths.ConfigFile("HM3_locations.json"));
                SendMessage("2J", s_Hitman2016Data);
            }
            catch (Exception s_Exception)
            {
                Log.Exception("Failed to reload HM3_locations.json", s_Exception);
            }
        }

        private void OnReloadHitman2016(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            ReloadHitman2016Data();
        }

        private void OnHitman2016DebugMode(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null)
                return;

            SendMessage("2D", s_MenuItem.IsChecked ? "true" : "false");
            Options.Update(opt => opt.HM3.Hitman2016DebugMode = s_MenuItem.IsChecked);
        }

        private void OnPredictSpecialRatings(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null)
                return;

            Options.Update(opt => opt.HM3.PredictSpecialRatings = s_MenuItem.IsChecked);
        }

        private void OnVerboseLogging(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null)
                return;

            var s_Enabled = s_MenuItem.IsChecked;

            Log.VerboseEnabled = s_Enabled;
            Options.Update(opt => opt.VerboseLogging = s_Enabled);
            SendMessage("VL", s_Enabled ? "true" : "false");

            Log.Info($"Verbose logging {(s_Enabled ? "enabled" : "disabled")}.");
        }

        private void OnOpenLogFolder(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            try
            {
                Directory.CreateDirectory(Log.DirectoryPath);

                Process.Start(new ProcessStartInfo()
                {
                    FileName = Log.DirectoryPath,
                    UseShellExecute = true,
                });
            }
            catch (Exception s_Exception)
            {
                Log.Exception("Could not open the log folder", s_Exception);
            }
        }

        private void OnIngameOverlay(object p_Sender, RoutedEventArgs p_RoutedEventArgs)
        {
            var s_MenuItem = p_Sender as MenuItem;

            if (s_MenuItem == null)
                return;

            SendMessage("OV", s_MenuItem.IsChecked ? "true" : "false");
            Options.Update(opt => opt.HM3.Overlay = s_MenuItem.IsChecked);
        }

        private void SyncMenuItem(string p_Header, bool p_Checked)
        {
            if (MainApp.MainWindow == null)
                return;

            MainApp.MainWindow.Dispatcher.BeginInvoke(new Action(() =>
            {
                foreach (var s_Item in m_MenuItems)
                {
                    if (s_Item is MenuItem s_MenuItem && s_MenuItem.Header as string == p_Header)
                        s_MenuItem.IsChecked = p_Checked;
                }
            }));
        }

        public void Update()
        {
            if (m_GameProcess == null || m_GameProcess.HasExited)
            {
                if (m_SkipUpdates > 0)
                {
                    --m_SkipUpdates;
                    return;
                }

                m_SkipUpdates = 4;

                Detach();

                var s_Processes = Process.GetProcessesByName("HitmanBloodMoney");

                if (s_Processes.Length == 0)
                    return;

                // We always select the first process.
                m_GameProcess = s_Processes[0];
                m_AttachStarted = DateTime.UtcNow;
                m_LastProcessProbe = DateTime.MinValue;

                try
                {
                    GameModuleBase = m_GameProcess.MainModule.BaseAddress;
                }
                catch
                {
                    GameModuleBase = IntPtr.Zero;
                }

                try
                {
                    m_GameStartedAt = m_GameProcess.StartTime;
                }
                catch
                {
                    m_GameStartedAt = DateTime.MinValue;
                }

                var s_Age = m_GameStartedAt == DateTime.MinValue
                    ? "age unknown"
                    : $"{(int) (DateTime.Now - m_GameStartedAt).TotalMilliseconds} ms old";

                Log.Info($"HitmanBloodMoney.exe detected (pid {m_GameProcess.Id}, {s_Age}). " +
                         "Waiting for the game to finish initializing before injecting.");
            }

            if (!Active)
            {
                // Injection is deferred until the game has unpacked itself.
                TryAttach();
                return;
            }

            // Update our trackers.
            TimeTracker.Update();
            StatTracker.Update();

            // Set game time.
            if (StatTracker.CurrentStats.m_Time > 0 || !InGame)
                Control.SetCurrentTime(StatTracker.CurrentStats.m_Time);
            else
                Control.SetCurrentTime(TimeTracker.CurrentTime);
        }

        private void TryAttach()
        {
            if (m_GameProcess == null)
                return;

            try
            {
                if (Reader == null)
                {
                    Reader = new ProcessMemoryReader(m_GameProcess);

                    if (!Reader.OpenProcess())
                    {
                        Reader = null;
                        m_GameProcess = null;
                        m_SkipUpdates = 4;

                        Log.Info("Could not open HitmanBloodMoney.exe for reading; will retry.");
                        return;
                    }
                }

                var s_Ready = IsGameReady(out var s_Readiness, out var s_ReadinessKey);

                if (s_ReadinessKey != m_LastReadinessLog)
                {
                    m_LastReadinessLog = s_ReadinessKey;

                    Log.Info(s_Ready
                        ? $"Game looks ready for injection ({s_Readiness})."
                        : $"Waiting to inject ({s_Readiness}).");
                }

                if (!s_Ready)
                    return;

                // The game can exit between the readiness poll and the injection attempt.
                if (m_GameProcess.HasExited)
                {
                    Log.Info("The game exited before HM3.dll could be injected.");

                    Detach();

                    m_SkipUpdates = 4;
                    return;
                }

                var s_ModulePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "HM3.dll");

                Log.Info($"Injecting '{s_ModulePath}' into pid {m_GameProcess.Id}.");

                // Create our injector and inject our stat module.
                // HM3.dll restores its hooks and unloads itself after the exit request.
                m_Injector = new Injector(m_GameProcess, false);
                m_Injector.InjectLibrary(s_ModulePath);

                Log.Info("HM3.dll injected.");

                // Setup our main control.
                MainApp.MainWindow.Dispatcher.Invoke(() =>
                {
                    Control = new MainControl();
                });

                // Setup our engine-specific classes.
                StatTracker = new StatTracker(this);
                TimeTracker = new TimeTracker(this);

                // Update level labels.
                CurrentLevel = "No Level";
                CurrentLevelScene = "No Level";
                InGame = false;

                Control.SetCurrentLevel(CurrentLevel);
                Control.SetCurrentLevelScene(CurrentLevelScene);

                // Set our control in the main window.
                InitMenuItems();
                MainApp.MainWindow.SetEngineControl(Control, m_MenuItems);

                m_SkipUpdates = 0;
                Active = true;

                Log.Info("HM3 engine active.");
            }
            catch (Exception s_Exception)
            {
                Log.Exception("Failed to initialize the HM3 engine.", s_Exception);

                Detach();

                m_SkipUpdates = 4;
            }
        }

        private bool IsGameReady(out string p_Readiness, out string p_ReadinessKey)
        {
            // Enumerating modules and querying MainWindowHandle are comparatively expensive,
            // so they are refreshed at most every 250 ms. The entry point probe is a 6 byte
            // read and runs on every tick.
            var s_Now = DateTime.UtcNow;

            if ((s_Now - m_LastProcessProbe).TotalMilliseconds >= 250)
            {
                m_LastProcessProbe = s_Now;
                ProbeProcessState();
            }

            // Primary signal: the Steam DRM stub replaces the entry point once it has
            // decrypted the image, so this flips to "unpacked" the moment it is safe to
            // inject. It does not depend on which DLLs the game happens to load when.
            var s_EntryPointUnpacked = IsEntryPointUnpacked();

            var s_Elapsed = (int) (s_Now - m_AttachStarted).TotalMilliseconds;

            var s_ProcessAge = m_GameStartedAt == DateTime.MinValue
                ? "unknown"
                : $"{(int) (DateTime.Now - m_GameStartedAt).TotalMilliseconds} ms";

            string s_Reason = null;

            if (s_EntryPointUnpacked)
                s_Reason = "entry point no longer looks like the DRM stub";
            else if (m_HasD3D9)
                s_Reason = "d3d9 loaded";
            else if (m_HasBink)
                s_Reason = "binkw32 loaded";
            else if (m_HasWindow)
                s_Reason = "game window created";
            else if (s_Elapsed >= ReadyTimeoutMs)
                s_Reason = $"readiness timeout reached after {ReadyTimeoutMs} ms, injecting anyway";

            // Stable key (no elapsed timing) so callers only react to real state changes.
            p_ReadinessKey = string.Format("ep={0};d3d9={1};bink32={2};window={3};error={4};state={5}",
                s_EntryPointUnpacked, m_HasD3D9, m_HasBink, m_HasWindow, m_ModuleError ?? "none",
                s_Reason == null ? "waiting" : "ready");

            p_Readiness = string.Format(
                "{0} ms after detection, process age {1}: entry-point={2}, d3d9={3}, binkw32={4}, window={5}{6}",
                s_Elapsed,
                s_ProcessAge,
                m_EntryPointChecked
                    ? (m_EntryPointLooksPacked ? "still the DRM stub" : "unpacked")
                    : "unknown",
                m_HasD3D9 ? "loaded" : "not loaded",
                m_HasBink ? "loaded" : "not loaded",
                m_HasWindow ? "created" : "not created",
                m_ModuleError != null ? $", module enumeration failed ({m_ModuleError})" : string.Empty);

            if (s_Reason == null)
                return false;

            p_Readiness += $" [{s_Reason}]";

            return true;
        }

        /// <summary>
        /// Refreshes the cached process snapshot used by the readiness gate.
        /// The module list is recorded once on the first successful probe.
        /// </summary>
        private void ProbeProcessState()
        {
            var s_HasD3D9 = false;
            var s_HasBink = false;
            var s_HasWindow = false;
            string s_ModuleError = null;
            List<string> s_ModuleNames = null;

            try
            {
                s_ModuleNames = new List<string>();

                foreach (ProcessModule s_Module in m_GameProcess.Modules)
                {
                    var s_Name = s_Module.ModuleName ?? string.Empty;
                    s_ModuleNames.Add(s_Name);

                    if (s_Name.Equals("d3d9.dll", StringComparison.OrdinalIgnoreCase))
                        s_HasD3D9 = true;
                    else if (s_Name.Equals("binkw32.dll", StringComparison.OrdinalIgnoreCase))
                        s_HasBink = true;
                }

                s_HasWindow = m_GameProcess.MainWindowHandle != IntPtr.Zero;
            }
            catch (Exception s_Exception)
            {
                s_ModuleError = s_Exception.GetType().Name;
            }

            if (s_ModuleError == null)
            {
                m_HasD3D9 = s_HasD3D9;
                m_HasBink = s_HasBink;
                m_HasWindow = s_HasWindow;
            }

            m_ModuleError = s_ModuleError;

            if (m_ModuleListLogged)
                return;

            m_ModuleListLogged = true;

            if (s_ModuleNames == null)
            {
                Log.Info($"Could not enumerate game modules: {m_ModuleError}");
                return;
            }

            var s_List = s_ModuleNames.Count > 30
                ? string.Join(", ", s_ModuleNames.GetRange(0, 30)) + ", ..."
                : string.Join(", ", s_ModuleNames);

            Log.Verbose($"Game modules at first readiness check ({s_ModuleNames.Count}): {s_List}");
        }

        private bool IsEntryPointUnpacked()
        {
            try
            {
                if (Reader == null)
                    return false;

                var s_EntryPoint = m_GameProcess.MainModule.EntryPointAddress;

                if (s_EntryPoint == IntPtr.Zero)
                    return false;

                // The Steam DRM stub is 7 bytes: B8 <imm32> FF E0  (mov eax, imm32 ; jmp eax).
                // The whole sequence must be read; the trailing FF E0 is what identifies the stub.
                var s_Bytes = Reader.Read(s_EntryPoint, 7);

                if (s_Bytes == null || s_Bytes.Length < 7)
                    return false;

                var s_LooksPacked = s_Bytes[0] == 0xB8 && s_Bytes[5] == 0xFF && s_Bytes[6] == 0xE0;

                if (!m_EntryPointChecked || s_LooksPacked != m_EntryPointLooksPacked)
                {
                    m_EntryPointChecked = true;
                    m_EntryPointLooksPacked = s_LooksPacked;

                    Log.Info(string.Format("Game entry point 0x{0:X8} bytes: {1} ({2}).",
                        s_EntryPoint.ToInt64(),
                        BitConverter.ToString(s_Bytes).Replace("-", " "),
                        s_LooksPacked
                            ? "still looks like the Steam DRM stub, the game has not unpacked yet"
                            : "unpacked"));
                }

                return !s_LooksPacked;
            }
            catch
            {
                // A failed probe must not prevent attaching.
                return false;
            }
        }

        private void Detach()
        {
            Active = false;
            m_GameProcess = null;
            m_LastReadinessLog = null;
            m_EntryPointChecked = false;
            m_EntryPointLooksPacked = false;
            m_ModuleListLogged = false;
            m_GameStartedAt = DateTime.MinValue;
            m_LastProcessProbe = DateTime.MinValue;
            m_HasD3D9 = false;
            m_HasBink = false;
            m_HasWindow = false;
            m_ModuleError = null;

            if (Reader != null)
            {
                Reader.CloseHandle();
                Reader = null;
            }

            if (m_Injector != null)
            {
                m_Injector.Dispose();
                m_Injector = null;
            }
        }

        public void OnMessage(string p_Type, string p_Data)
        {
            if (!Active)
                return;

            switch (p_Type)
            {
                case "HI":
                {
                    // Received hello. Send relevant configuration messages.

                    // A new injection means the previous module state is gone.
                    m_HasCheatsActiveFromModule = false;
                    m_TotalLevelNpcs = 0;

                    // Send Hitman 2016 data.
                    ReloadHitman2016Data();

                    // Send configuration values.
                    SendMessage("EC", Options.Get().HM3.Cheats ? "true" : "false");
                    SendMessage("US", Options.Get().HM3.UnlimitedSaves ? "true" : "false");
                    SendMessage("H2", Options.Get().HM3.Hitman2016Mode ? "true" : "false");
                    SendMessage("2D", Options.Get().HM3.Hitman2016DebugMode ? "true" : "false");
                    SendMessage("OV", Options.Get().HM3.Overlay ? "true" : "false");
                    SendMessage("VL", Log.VerboseEnabled ? "true" : "false");
                    
                    break;
                }

                case "CS":
                {
                    CurrentScene = p_Data.ToLowerInvariant().Replace("/", "\\");

                    Tuple<string, string, bool> s_LevelData;
                    if (!m_SceneNames.TryGetValue(CurrentScene, out s_LevelData))
                    {
                        CurrentLevel = "No Level";
                        CurrentLevelScene = "No Level";
                        InGame = false;
                    }
                    else
                    {
                        CurrentLevel = s_LevelData.Item1;
                        CurrentLevelScene = s_LevelData.Item2;
                        InGame = s_LevelData.Item3;
                    }

                    Control?.SetCurrentLevel(CurrentLevel);
                    Control?.SetCurrentLevelScene(CurrentLevelScene);

                    break;
                }

                case "SA":
                {
                    int statAddr = 0;
                    if (int.TryParse(p_Data, out statAddr))
                    {
                        StatTracker.StatAddress = new IntPtr(statAddr);
                    }
                    break;
                }

                case "DA":
                {
                    int difficultyAddr = 0;
                    if (int.TryParse(p_Data, out difficultyAddr))
                    {
                        StatTracker.DifficultyAddress = new IntPtr(difficultyAddr);
                    }
                    break;
                }

                case "LW":
                {
                    SetLiveWeaponKills(p_Data);
                    break;
                }

                case "TA":
                {
                    int timeAddr = 0;
                    if (TimeTracker != null && int.TryParse(p_Data, out timeAddr))
                    {
                        TimeTracker.TimeAddress = new IntPtr(timeAddr);
                    }
                    break;
                }

                case "EC":
                {
                    var s_Enabled = p_Data == "true";
                    Options.Update(opt => opt.HM3.Cheats = s_Enabled);
                    SyncMenuItem("Enable Cheats", s_Enabled);
                    break;
                }

                case "US":
                {
                    var s_Enabled = p_Data == "true";
                    Options.Update(opt => opt.HM3.UnlimitedSaves = s_Enabled);
                    SyncMenuItem("Unlimited Saves", s_Enabled);
                    break;
                }

                case "H2":
                {
                    var s_Enabled = p_Data == "true";
                    Options.Update(opt => opt.HM3.Hitman2016Mode = s_Enabled);
                    SyncMenuItem("Hitman 2016 Mode", s_Enabled);
                    break;
                }

                case "2D":
                {
                    var s_Enabled = p_Data == "true";
                    Options.Update(opt => opt.HM3.Hitman2016DebugMode = s_Enabled);
                    SyncMenuItem("Hitman 2016 Mode Debug", s_Enabled);
                    break;
                }

                case "RL":
                {
                    ReloadHitman2016Data();
                    break;
                }

                case "CA":
                {
                    m_CheatsActiveFromModule = p_Data == "true";
                    m_HasCheatsActiveFromModule = true;
                    break;
                }

                case "NP":
                {
                    int s_Count;
                    if (int.TryParse(p_Data, out s_Count))
                        m_TotalLevelNpcs = s_Count;
                    break;
                }
            }
        }

        public void SendMessage(string p_Type, string p_Contents = "")
        {
            MainApp.Pipeman.PushMessage(new PipeMessage()
            {
                Module = Name, 
                Type = p_Type, 
                Content = p_Contents
            });
        }

        public void Dispose()
        {
            SendMessage("XX");

            if (Reader != null)
            {
                Reader.CloseHandle();
                Reader = null;
            }

            if (m_Injector != null)
            {
                m_Injector.Dispose();
                m_Injector = null;
            }
        }
    }
}
