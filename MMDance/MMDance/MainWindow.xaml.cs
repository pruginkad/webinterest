﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using System.Threading;

using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Collections.ObjectModel;
using System.Windows.Media.Media3D;
using System.Diagnostics;
using System.IO;


namespace MMDance
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        
        public ControlWrapper m_ControlWrapper = new ControlWrapper();

        public MainWindow()
        {
            InitializeComponent();

            for (int motor = 0; motor < ControlWrapper.MOTORS_COUNT; motor++)
            {
                m_step_mult.m_uMult[motor] = 1;
            }

            m_BezierProfileUserControl.SetCurve(Properties.Settings.Default.BezierCurve);
            m_BezierProfileLimitUserControl.SetCurve(Properties.Settings.Default.BezierCurveLimit);

            WorkingThread = new Thread(new ThreadStart(ProcessCommand));
            WorkingThread.SetApartmentState(ApartmentState.STA);
            WorkingThread.Start();
        }


        Thread WorkingThread = null;
        System.Windows.Threading.DispatcherTimer dispatcherTimer = new System.Windows.Threading.DispatcherTimer();

        bool StopThread = false;
        private delegate void UpdateCurrentPositionDelegate(double x1, double y1);

        private Queue<Byte[]> m_out_list = new Queue<Byte[]>();

        public int GetQueueLen()
        {
            lock (m_out_list)
            {
                return m_out_list.Count;
            }
        }
        public void AddCommand(Byte[] command)
        {
            ushort crc16 = CRCHelper.crc16_ccitt(command, command.Length - 2);
            byte[] byteArray = BitConverter.GetBytes(crc16);
            command[command.Length - 2] = byteArray[0];
            command[command.Length - 1] = byteArray[1];
            lock (m_out_list)
            {
                m_out_list.Enqueue(command);
            }
        }

        private void ProcessCommand()
        {
            int check_counter = 100;
            int SleepVal = 1;
            while (!StopThread)
            {
                check_counter++;
                if (check_counter > 100 && ControlUserControl != null && m_ControlWrapper.IsOpen())
                {
                    check_counter = 0;
                    ControlUserControl.textBlockInfo.Dispatcher.BeginInvoke(new Action(delegate()
                    {
                        ControlUserControl.textBlockInfo.Text = m_ControlWrapper.GetCurText(); //where item is the item to be added and listbox is the control being updated.
                    }));
                }
                if (m_ControlWrapper.IsControllerAvailable())
                {
                    lock (m_out_list)
                    {
                        if (m_out_list.Count <= 0)
                        {
                            SleepVal = 100;
                        }
                        else
                        {
                            Byte[] buf = m_out_list.Dequeue();
                            if (!m_ControlWrapper.WriteCommandToController(buf))
                            {
                                MessageBox.Show("Unable To write");
                            }
                        }
                    }
                    SleepVal = 1;
                }
                else
                {
                    SleepVal = m_ControlWrapper.GetEstimatedSleep();
                }
                
                Thread.Sleep(SleepVal);
            }
        }

        internal class stanok_coord
        {
            public int z = 0;//между бабок
            public int x = 0;//ход шпинделя
            public int b = 0;//угол
            public int w = 0;
        }

        [StructLayout(LayoutKind.Sequential, Size = 4 * ControlWrapper.MOTORS_COUNT), Serializable]
        public class do_steps
        {
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = ControlWrapper.MOTORS_COUNT)]
            public Int32[] m_uSteps = new Int32[ControlWrapper.MOTORS_COUNT];
        }

        [StructLayout(LayoutKind.Sequential, Size = 4 * ControlWrapper.MOTORS_COUNT), Serializable]
        public class do_steps_multiplier
        {
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = ControlWrapper.MOTORS_COUNT)]
            public Int32[] m_uMult = new Int32[ControlWrapper.MOTORS_COUNT];
        }

        [StructLayout(LayoutKind.Sequential, Size = 4 + ControlWrapper.MOTORS_COUNT), Serializable]
        public class do_timer_set
        {
            [MarshalAs(UnmanagedType.U2)]
            public UInt16 m_timer_res;
            [MarshalAs(UnmanagedType.U2)]
            public UInt16 m_strike_impuls;
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = ControlWrapper.MOTORS_COUNT)]
            public byte[] m_multiplier = new byte[ControlWrapper.MOTORS_COUNT];
        }

        [StructLayout(LayoutKind.Sequential, Size = 4), Serializable]
        internal class  do_control_signals
        {
            [MarshalAs(UnmanagedType.U1)]
            public byte enable;
        }
        
        public do_steps_multiplier m_step_mult = new do_steps_multiplier();
        stanok_coord m_cur_pos = new stanok_coord();
        stanok_coord m_cur_task = new stanok_coord();//Последняя отправленная на обработку координата
                                    //steps max x8 x16
        public const int Z_POS = 2; //2225 //1150 - x16
        public const int X_POS = 1; //1900 //950  -x16
        public const int B_POS = 3; //12000 
        public const int W_POS = 0; //12000 
        //~0.38 мм на шаг x16 0.76 mm

        public void SetControlSettings(bool enable)
        {
            do_control_signals control_sign = new do_control_signals();
            control_sign.enable = Convert.ToByte(enable);
            
            byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];
            OutputPacketBuffer[0] = 0;
            OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_CONTROL_SIGNALS;
            ControlWrapper.StructureToByteArray(control_sign, OutputPacketBuffer, 2);
            AddCommand(OutputPacketBuffer);
        }

        public static do_timer_set m_curtimerset = new do_timer_set();
        
        public void SetTimerSettings(UInt16 timer_res, UInt16 strike_impuls, byte[] multiplier)
        {
            m_curtimerset.m_timer_res = (UInt16)(UInt16.MaxValue - timer_res);
            m_curtimerset.m_strike_impuls = strike_impuls;

            for (int i = 0; i < ControlWrapper.MOTORS_COUNT && i < multiplier.Length; i++)
            {
                m_curtimerset.m_multiplier[i] = multiplier[i];
            }
            
            byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];
            OutputPacketBuffer[0] = 0;
            OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_TIME;
            ControlWrapper.StructureToByteArray(m_curtimerset, OutputPacketBuffer, 2);
            AddCommand(OutputPacketBuffer);
        }
        bool m_bPauseSoft = false;
        public void SetPauseSoft(bool pause)
        {
            m_bPauseSoft = pause;
        }
        public void SetPause(bool pause)
        {
            byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];
            OutputPacketBuffer[0] = 0;
            OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_PAUSE;
            OutputPacketBuffer[2] = Convert.ToByte(pause);
            AddCommand(OutputPacketBuffer);
        }
        
        public void SetInk(bool ink)
        {
            byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];
            OutputPacketBuffer[0] = 0;
            OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_INK;
            OutputPacketBuffer[2] = Convert.ToByte(ink);
            AddCommand(OutputPacketBuffer);
        }

        public void InitCurBPos()
        {
            m_cur_pos.b = 0;
        }
        public void SetStepsToController(do_steps steps, bool update_pos = true)
        {
	        if(
		        steps.m_uSteps[Z_POS] == 0 &&
		        steps.m_uSteps[X_POS] == 0 &&
                steps.m_uSteps[B_POS] == 0 &&
		        steps.m_uSteps[W_POS] == 0)
	        {
		        return;
	        }
            if (update_pos)
            {
                m_cur_pos.x += steps.m_uSteps[X_POS];
                m_cur_pos.z += steps.m_uSteps[Z_POS];
                m_cur_pos.b += steps.m_uSteps[B_POS];
                m_cur_pos.w += steps.m_uSteps[W_POS];
            }

	       	for(int motor = 0; motor < ControlWrapper.MOTORS_COUNT; motor++)
	        {
		        steps.m_uSteps[motor] = steps.m_uSteps[motor]*m_step_mult.m_uMult[motor];
	        }

	        byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];	//Allocate a memory buffer equal to our endpoint size + 1
	        OutputPacketBuffer[0] = 0;				//The first byte is the "Report ID" and does not get transmitted over the USB bus.  Always set = 0.
	        OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_STEPS;
            ControlWrapper.StructureToByteArray(steps, OutputPacketBuffer, 2);
	        AddCommand(OutputPacketBuffer);
        }

        public void GoToZX(int z = -1, int x = -1, int b = -1, int w = -1)
        {
            MainWindow.do_steps var_do_steps = new MainWindow.do_steps();
            
            if(z >= 0)
                var_do_steps.m_uSteps[Z_POS] = z - m_cur_pos.z;
            if(x >= 0)
                var_do_steps.m_uSteps[X_POS] = x - m_cur_pos.x;
            if (b >= 0)
                var_do_steps.m_uSteps[B_POS] = b - m_cur_pos.b;
            if (w >= 0)
                var_do_steps.m_uSteps[W_POS] = w - m_cur_pos.w;

            string s;
            s = String.Format("Z:{0},X:{1},B:{2}",
                var_do_steps.m_uSteps[Z_POS],
                var_do_steps.m_uSteps[X_POS],
                var_do_steps.m_uSteps[B_POS]);
            Debug.WriteLine(s);

            SetStepsToController(var_do_steps);
        }

        public void GoToBW(int b, int w)
        {
            MainWindow.do_steps var_do_steps = new MainWindow.do_steps();
            var_do_steps.m_uSteps[B_POS] = b - m_cur_pos.b;
            var_do_steps.m_uSteps[W_POS] = w - m_cur_pos.w;
            SetStepsToController(var_do_steps);
        }

        int m_counter = 0;
        

        public void Start()
        {
            m_counter = 0;

            PictureUserControl.UpdateProfileResult();

            InitStartPos();
            //RawProg();
            

            dispatcherTimer.Tick += new EventHandler(dispatcherTimer_Tick);
            dispatcherTimer.Interval = TimeSpan.FromMilliseconds(1);
            dispatcherTimer.Start();
            GoToZX(m_CurTask.z, -1, -1);
        }

        void RawProg()
        {
            int z1 = GetStepFromZ(10);
            int z2 = GetStepFromZ(700);

            
            int yDown = GetStepFromY(40);
            int yUp = GetStepFromY(45);
            GoToZX(-1, yUp, -1);
            for (double alfa = 0; alfa < 360; alfa += 45)
            {
                int b = GetStepFromAngle(alfa);
                GoToZX(z1, yUp, b);
                GoToZX(z1, yDown, b);
                GoToZX(z2, yDown, b);
                GoToZX(z2, yUp, b);
            }

            GoToZeroes();
        }
        void InitStartPos()
        {
            m_cur_pos.x = GetStepFromY(Properties.Settings.Default.YStart);
            m_CurTask.z =  GetStepFromZ((double)Properties.Settings.Default.ZStart);
        }

        public double GetDepthStep()
        {
            return UserControlFor3D.m_dFreza.X + UserControlFor3D.m_dFreza.Z;
        }

        private int DoEngraving(int BSteps, int ZSteps)
        {
            Point3D intersection;
            double angle = GetAngleFromStep(BSteps);
            double Z = GetZFromStep(ZSteps);
            int YSteps = GetStepFromY(Properties.Settings.Default.YStart);

            UserControlFor3D.IntersectionType ret = PictureUserControl.
                m_UserControlFor3D.GetIntersection(angle, Z, out intersection);

            Vector3D vec = (Vector3D)intersection;
            vec.Z = 0;
            if (UserControlFor3D.IntersectionType.E_INTERSECTION == ret)
            {
                if (!Properties.Settings.Default.Roughing || BSteps == 0)
                {
                    PictureUserControl.m_UserControlFor3D.UpdatePosition(intersection, angle);
                }

                YSteps = GetStepFromY(vec.Length);
            }
            else if (UserControlFor3D.IntersectionType.E_OUT != ret)
            {
                YSteps = GetStepFromY(UserControlFor3D.max_x);
            }

            return YSteps;
        }

        stanok_coord m_CurTask = new stanok_coord();
        
        int Get360GradSteps()
        {
            return (int)(360/ Properties.Settings.Default.StepBgrad);
        }
        int GetStepFromAngle(double angle)
        {
            double temp = angle / Properties.Settings.Default.StepBgrad;
            return (int)temp;
        }
        double GetAngleFromStep(int step)
        {
            double angle = step;
            angle *= Properties.Settings.Default.StepBgrad;
            return angle;
        }
        double GetZFromStep(int step)
        {
            double Z = step;
            Z *= Properties.Settings.Default.StepZmm;
            return Z;
        }
        int GetStepFromZ(double Z)
        {
            double ret = Z / Properties.Settings.Default.StepZmm;
            return (int)ret;
        }
        double GetYFromStep(int step)
        {
            double Y = step;
            Y *= Properties.Settings.Default.StepYmm;
            return Y;
        }

        int GetStepFromY(double Y)
        {
            double ret = Y/Properties.Settings.Default.StepYmm;
            return (int)ret;
        }

        int GetMaxOnCircle()
        {
            int max_y = 0;
            for (m_CurTask.b = 0; m_CurTask.b < Get360GradSteps(); m_CurTask.b += 1)
            {
                m_CurTask.x = DoEngraving(m_CurTask.b, m_CurTask.z);
                if (m_CurTask.x > max_y)
                {
                    max_y = m_CurTask.x;
                }
            }
            return max_y;
        }

        void DoLongitudinal()
        {
            m_CurTask.x = DoEngraving(m_CurTask.b, m_CurTask.z);
            GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);


            m_CurTask.z += GetStepFromZ(UserControlFor3D.m_dFreza.Z - 1);

            if (GetZFromStep(m_CurTask.z) > PictureUserControl.m_UserControlFor3D.GetMaxZ())
            {
                m_CurTask.x = GetStepFromY(Properties.Settings.Default.YStart);
                GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);
                m_CurTask.z = GetStepFromZ((double)Properties.Settings.Default.ZStart); ;
                GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);
                m_CurTask.b += GetStepFromAngle(0.9);
                
            }
            m_counter++;

            if (m_CurTask.b > Get360GradSteps())
            {
                StopMachine();
            }
        }
        void DoTransversal()
        {
            if (Properties.Settings.Default.Roughing)
            {
                m_CurTask.x = GetMaxOnCircle();
                for (m_CurTask.b = 0; m_CurTask.b < Get360GradSteps(); m_CurTask.b += 1)
                {
                    GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);
                }
            }
            else
            {
                m_CurTask.x = DoEngraving(m_CurTask.b, m_CurTask.z);
                GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);
            }

            m_CurTask.b += 1;

            if (m_CurTask.b > Get360GradSteps())
            {
                m_CurTask.b = 0;
                InitCurBPos();
                m_CurTask.z += GetStepFromZ(UserControlFor3D.m_dFreza.Z - 1);
            }
            m_counter++;

            if (GetZFromStep(m_CurTask.z) > PictureUserControl.m_UserControlFor3D.GetMaxZ())
            {
                StopMachine();
            }
        }
        private void dispatcherTimer_Tick(object sender, EventArgs e)
        {
            if (m_bPauseSoft)
            {
                return;
            }
            if (GetQueueLen() < 10)
            {

                if (Properties.Settings.Default.Longitudinal)
                {
                    DoLongitudinal();
                }
                else
                {
                    DoTransversal();
                }
                
                ControlUserControl.textBlockCounter.Text = m_counter.ToString();

            }
        }

        void StopMachine()
        {
            Thread.Sleep(1000);
            GoToZeroes();
            m_CurTask = new stanok_coord();
            //ControlUserControl.checkBoxPauseSoft.IsChecked = true;
            dispatcherTimer.Stop();
        }
        void GoToZeroes()
        {
            m_CurTask.x = GetStepFromY(Properties.Settings.Default.YStart);
            GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);
            m_CurTask.z = 0;
            m_CurTask.b = 0;
            GoToZX(m_CurTask.z, m_CurTask.x, m_CurTask.b);
        }
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Properties.Settings.Default.BezierCurve = m_BezierProfileUserControl.GetCurve();
            Properties.Settings.Default.BezierCurveLimit = m_BezierProfileLimitUserControl.GetCurve();

            Properties.Settings.Default.Save();
            StopThread = true;
            if (WorkingThread != null)
            {
                WorkingThread.Join();
            }
        }
        static public MainWindow GetMainWnd()
        {
            return (MainWindow)System.Windows.Application.Current.Windows[0];
        }
        public List<Point> GetLongProfileCurvePoints()
        {
            return m_BezierProfileUserControl.GetCurvePoints();
        }
        public List<Point> GetLimitProfileCurvePoints()
        {
            return m_BezierProfileLimitUserControl.GetCurvePoints();
        }

        public void LoadAll(string FileName)
        {
            ProfileElementList profile = new ProfileElementList();
            string dir = System.IO.Path.GetDirectoryName(FileName);
            using (StreamReader outfile = new StreamReader(FileName, true))
            {
                profile = ProfileElementSerializer.DeserializeObject(outfile.ReadToEnd());
            }

            m_BezierProfileUserControl.SetCurve(profile.BezierCurve);
            m_BezierProfileLimitUserControl.SetCurve(profile.BezierCurveLimit);

            for (int i = 0; i < profile.ProfileElements.Count; i++)
            {
                ProfileElement element = profile.ProfileElements[i];
                
                string filename = System.IO.Path.GetFileName(element.FileName);
                element.FileName = dir + filename;
                ListPoint.SerializeObject(element.Points, element.FileName);
                element.Points = null;
            }
            PictureUserControl.SetFullProfile(profile);
        }

        public void SaveAll(string FileName)
        {
            ProfileElementList profile = PictureUserControl.GetFullProfile();
            if (profile == null)
            {
                return;
            }
            profile.BezierCurve = m_BezierProfileUserControl.m_Segments;
            profile.BezierCurveLimit = m_BezierProfileLimitUserControl.m_Segments;

            using (StreamWriter outfile = new StreamWriter(FileName, false))
            {
                outfile.Write(ProfileElementSerializer.SerializeObject(profile));
            }
            profile.BezierCurve = null;
            profile.BezierCurveLimit = null;
            for (int i = 0; i < profile.ProfileElements.Count; i++)
            {
                ProfileElement element = profile.ProfileElements[i];
                element.Points = null;
            }
        }
    }
}
