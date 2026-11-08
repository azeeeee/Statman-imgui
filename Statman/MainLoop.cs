using System;
using System.Diagnostics;
using System.Threading;
using System.Windows;
using Statman.Util;
using Statman.Windows;

namespace Statman
{
    class MainLoop : IDisposable
    {
        private readonly int m_Ticks;
        private TickTimer m_Timer;
        private Thread m_Thread;
        private volatile bool m_Running;

        public event EventHandler Update;

        public MainLoop(int p_TickRate)
        {
            // Calculate required ticks for each update.
            m_Ticks = 1000 / p_TickRate;

            m_Timer = new TickTimer();
            m_Running = false;
        }

        public void Dispose()
        {
            m_Running = false;

            // Cooperative stop: the loop finishes its current tick and exits. The thread is
            // a background thread, so a slow tick (e.g. the update check HTTP request) can
            // never keep the process alive.
            m_Thread?.Join(2000);
            m_Thread = null;
        }

        public void Start()
        {
            if (m_Running)
                return;

            m_Running = true;

            m_Thread = new Thread(DoLoopWork)
            {
                IsBackground = true,
            };
            m_Thread.SetApartmentState(ApartmentState.STA);
            m_Thread.Start();
        }

        public void Stop()
        {
            if (!m_Running || m_Thread == null)
            {
                m_Thread = null;
                return;
            }

            m_Running = false;

            m_Thread.Join();
            m_Thread = null;
        }

        protected void DoLoopWork()
        {
            m_Timer = new TickTimer();

            while (m_Running)
            {
                if (MainApp.CheckForUpdates)
                {
                    MainApp.CheckForUpdates = false;

                    // Check for updates.
                    MainApp.MainWindow.SetStatusLabel("Checking for updates...");

                    var s_UpdateCheck = Stopwatch.StartNew();

                    var s_HasUpdate = UpdateChecker.HasNewerVersion(out var s_CurrentVersion, out var s_LatestVersion, out var s_ReleaseURL, out var s_PreRelease);

                    s_UpdateCheck.Stop();

                    Log.Info($"Update check finished in {s_UpdateCheck.ElapsedMilliseconds} ms (update available: {s_HasUpdate}).");

                    if (s_HasUpdate)
                    {
                        MainApp.MainWindow.Dispatcher.Invoke(() =>
                        {
                            new UpdateWindow(s_CurrentVersion, s_LatestVersion, s_ReleaseURL, s_PreRelease).Show();
                        });
                    }

                    MainApp.MainWindow.SetStatusLabel("Waiting for game...");
                }

                m_Timer.Reset();

                Update?.Invoke(this, null);

                var s_Ticks = m_Timer.Ticks;

                if (s_Ticks >= m_Ticks)
                    continue;

                Thread.Sleep((int) (m_Ticks - s_Ticks));
            }
        }
    }
}
