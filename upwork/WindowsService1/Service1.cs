﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.Runtime.InteropServices;
using System.ServiceModel.Web;
using ConsoleApplication1;
using System.IO;
using System.Security.Principal;
using System.Web.Script.Serialization;
using System.Management;

namespace WindowsService1
{
    public partial class AService1 : ServiceBase
    {

        public AService1()
        {
            InitializeComponent();
        }

        public ServiceHost serviceHost = null;

        public static string API_PATH = string.Empty;
        public static string EVENTS_SOURCE = "WS_EVENT";
        public static EventLog m_EventLog = new EventLog("AService1Events", System.Environment.MachineName, EVENTS_SOURCE);

        public void ReadApiPath()
        {
            try
            {
                API_PATH = Properties.Settings.Default.ASPDataCenter;
            }
            catch (Exception ex)
            {
                EventLog.WriteEntry("ReadApiPath:" + ex.Message);
            }
            if (string.IsNullOrEmpty(API_PATH))
            {
                
            }
        }
        protected override void OnStart(string[] args)
        {
            EventLog.WriteEntry("Starting");

            #if (DEBUG)
            System.Diagnostics.Debugger.Launch();
            #endif
            try
            {
                m_EventLog.MaximumKilobytes = 64;
                m_EventLog.ModifyOverflowPolicy( OverflowAction.OverwriteOlder, 1);
            }
            catch(Exception ex)
            {
                EventLog.WriteEntry(ex.Message);
            }
            ReadApiPath();

            if (serviceHost != null)
            {
                serviceHost.Close();
            }

            try
            {

                serviceHost = new ServiceHost(typeof(LocalWorkstationState));                

                serviceHost.Open();

                SetState("OnStart");
            }
            catch (Exception ex)
            {
                EventLog.WriteEntry(ex.Message);
            }
        }

        EventInstance myInfoEvent = new EventInstance(0, 0, EventLogEntryType.Information);

        protected void SetState(string state, int sessionId = -1)
        {
            string username = GetUsername(sessionId);
            

            ServiceState pState = new ServiceState(state, username);
            
            if (EventLog.SourceExists(EVENTS_SOURCE))
            {
                string[] insertStrings = { new JavaScriptSerializer().Serialize(pState)};
                byte[] binaryData = {};
                m_EventLog.WriteEvent(myInfoEvent, binaryData, insertStrings);
            }

            ReadApiPath();

            if (!string.IsNullOrEmpty(API_PATH))
            {
                //HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://localhost:52323/api");
                //request.Method = "POST";
                //request.ContentType = "application/x-www-form-urlencoded";
                //request.AllowAutoRedirect = false;

                ////Put the post data into the request
                //byte[] data = (new ASCIIEncoding()).GetBytes("workstation=efjcnwmc&User=voemk&Event=cfhwb&time=1-1-2030");
                //request.ContentLength = data.Length;
                //Stream reqStream = request.GetRequestStream();
                //reqStream.Write(data, 0, data.Length);
                //reqStream.Close();

                using (WebChannelFactory<IWorkstationServiceLog> myChannelFactory =
                                                    new WebChannelFactory<IWorkstationServiceLog>(
                                                        new Uri(Properties.Settings.Default.ASPDataCenter)))
                {
                    IWorkstationServiceLog client = null;

                    try
                    {
                        client = myChannelFactory.CreateChannel();
                        client.WorkstationServiceLog(pState.MachineName, pState.UserName, pState.State, pState.timestamp);

                    }
                    catch (Exception ex)
                    {
                        if (client != null)
                        {
                            ((ICommunicationObject)client).Abort();
                        }
                    }
                }
            }
        }

        protected override void OnStop()
        {
            SetState("OnStop");
            
            if (serviceHost != null)
            {
                try
                {
                    serviceHost.Close();
                    serviceHost = null;
                }
                catch (Exception ex)
                {
                    EventLog.WriteEntry(ex.Message);
                }
            }
        }

        protected override void OnShutdown()
        {
            SetState("OnShutdown");
        }

        protected override bool OnPowerEvent(PowerBroadcastStatus powerStatus)
        {
            SetState("OnPowerEvent");
            return true;
        }

        protected override void OnSessionChange(SessionChangeDescription changeDescription)
        {
            try
            {
                int sessionId = changeDescription.SessionId;

                switch (changeDescription.Reason)
                {
                    case SessionChangeReason.SessionLogon:
                        {
                            SetState("SessionLogon", sessionId);
                            break;
                        }

                    case SessionChangeReason.SessionLogoff:
                        {
                            SetState("SessionLogoff", sessionId);
                            break;
                        }


                    case SessionChangeReason.SessionLock:
                        SetState("SessionLock", sessionId);
                        break;

                    case SessionChangeReason.SessionUnlock:
                        SetState("SessionUnlock", sessionId);
                        break;

                    //case SessionChangeReason.SessionRemoteControl:
                    //    SetState("SessionRemoteControl", sessionId);
                    //    break;
                    
                    //case SessionChangeReason.ConsoleConnect:
                    //    SetState("ConsoleConnect", sessionId);
                    //    break;

                    //case SessionChangeReason.ConsoleDisconnect:
                    //    SetState("ConsoleDisconnect", sessionId);
                    //    break;

                    //case SessionChangeReason.RemoteConnect:
                    //    SetState("RemoteConnect", sessionId);
                    //    break;

                    //case SessionChangeReason.RemoteDisconnect:
                    //    SetState("RemoteDisconnect", sessionId);
                    //    break;

                }

            }
            catch (System.Exception ex)
            {
                EventLog.WriteEntry("Exception: IntellectRunSrv.OnSessionChange. " + ex.Message);
            }
        }

        [DllImport("Wtsapi32.dll")]
        private static extern bool WTSQuerySessionInformation(IntPtr hServer, int sessionId, WtsInfoClass wtsInfoClass, out IntPtr ppBuffer, out int pBytesReturned);
        [DllImport("Wtsapi32.dll")]
        private static extern void WTSFreeMemory(IntPtr pointer);

        private enum WtsInfoClass
        {
            WTSUserName = 5,
            WTSDomainName = 7,
        }

        private static string GetUsername(int sessionId, bool prependDomain = true)
        {
            if (sessionId < 0)
            {
                return getWMIUsername();
            }
            IntPtr buffer;
            int strLen;
            string username = "SYSTEM";
            if (WTSQuerySessionInformation(IntPtr.Zero, sessionId, WtsInfoClass.WTSUserName, out buffer, out strLen) && strLen > 1)
            {
                username = Marshal.PtrToStringAnsi(buffer);
                WTSFreeMemory(buffer);
                if (prependDomain)
                {
                    if (WTSQuerySessionInformation(IntPtr.Zero, sessionId, WtsInfoClass.WTSDomainName, out buffer, out strLen) && strLen > 1)
                    {
                        username = Marshal.PtrToStringAnsi(buffer) + "\\" + username;
                        WTSFreeMemory(buffer);
                    }
                }
            }
            return username;
        }

        private static string getWMIUsername()
        {
            string username = string.Empty;
            try
            {
                // Define WMI scope to look for the Win32_ComputerSystem object
                ManagementScope ms = new ManagementScope("\\\\.\\root\\cimv2");
                ms.Connect();

                ObjectQuery query = new ObjectQuery
                        ("SELECT * FROM Win32_ComputerSystem");
                ManagementObjectSearcher searcher =
                        new ManagementObjectSearcher(ms, query);

                // This loop will only run at most once.
                foreach (ManagementObject mo in searcher.Get())
                {
                    // Extract the username
                    username += mo["UserName"].ToString();
                }
            }
            catch (Exception ex)
            {
                
            }

            return username;
        } // end String getUsername()
    }
}
