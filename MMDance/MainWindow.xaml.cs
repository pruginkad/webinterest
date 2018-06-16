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
using System.Numerics;

using GCode;
using MacGen;

namespace MMDance
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const double MMPerBStep = 0.00505;
        const double MMPerXYStep = 0.0467*4.0;
        const double MillLen = 52;
        public ControlWrapper m_ControlWrapper = new ControlWrapper();

        public MainWindow()
        {
            Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
            InitializeComponent();
            for (int motor = 0; motor < ControlWrapper.MOTORS_COUNT; motor++)
            {
                m_step_mult.m_uMult[motor] = 1;
            }


            //OnFileOpen(Properties.Settings.Default.PicFile);
            WorkingThread = new Thread(new ThreadStart(ProcessCommand));
            WorkingThread.SetApartmentState(ApartmentState.STA);
            WorkingThread.Start();
        }

        public ObservableCollection<PictureColors> m_colors = new ObservableCollection<PictureColors>();
        FormatConvertedBitmap newFormatedBitmapSource = null;
        Size m_image_size = new Size(0, 0);
        public Size GetImageSize()
        {
            if (newFormatedBitmapSource != null)
            {
                m_image_size.Width = newFormatedBitmapSource.PixelWidth;
                m_image_size.Height = newFormatedBitmapSource.PixelHeight;
            }
            return m_image_size;
        }
        public bool OnFileOpen(string filename)
        {
            try
            {
                BitmapImage bitmapImage = new BitmapImage(new Uri(filename));
                
                newFormatedBitmapSource = new FormatConvertedBitmap();
                newFormatedBitmapSource.BeginInit();
                newFormatedBitmapSource.Source = bitmapImage;
                newFormatedBitmapSource.DestinationFormat = PixelFormats.Rgb24;
                newFormatedBitmapSource.EndInit();

                PictureUserControl.loaded_image.Source = newFormatedBitmapSource;
                m_colors.Clear();

               Dictionary<Color, int> ColorMap = new Dictionary<Color,int>();

               int width = newFormatedBitmapSource.PixelWidth;
               int stride = width * 3;
               int size = newFormatedBitmapSource.PixelHeight * stride;
               byte[] pixels = new byte[size];
               Array.Clear(pixels, 0, size);
               newFormatedBitmapSource.CopyPixels(pixels, stride, 0);
                for (int y = 0; y < newFormatedBitmapSource.PixelHeight; y++)
                {
                    
                    for (int x = 0; x < newFormatedBitmapSource.PixelWidth; x++)
                    {
                        int index = y * stride + 3 * x;
                        byte red = pixels[index];
                        byte green = pixels[index + 1];
                        byte blue = pixels[index + 2];
                        
                        Color cur_col = Color.FromRgb(red, green, blue);
                        int Count = 0;
                        ColorMap.TryGetValue(cur_col, out Count);
                        Count++;

                        ColorMap[cur_col] = Count;
                    }
                }
                foreach(KeyValuePair<Color,int> pair in ColorMap)
                {
                    Color color = pair.Key;
                    
                    m_colors.Add(new PictureColors(color, pair.Value));
                }
            }
            catch (System.Exception ex)
            {
                return false;
            }
            
            ControlUserControl.listViewColors.DataContext = m_colors;

            return true;
        }

        Color m_SelectedColor = Color.FromRgb(0, 0, 0);
        Color GetPixelColor(byte[] pixels, int x, int y, int stride)
        {
            int index = y * stride + 3 * x;
            byte red = pixels[index];
            byte green = pixels[index + 1];
            byte blue = pixels[index + 2];
            return Color.FromRgb(red, green, blue);
        }

        List<KeyValuePair<int, int>> m_selected_points = new List<KeyValuePair<int, int>>();

        public void SelectionChanged(Color color, bool bContour)
        {
            m_selected_points.Clear();
            m_SelectedColor = color;
            this.Background = new SolidColorBrush(color);
            int width = newFormatedBitmapSource.PixelWidth;
            int stride = width * 3;
            int size = newFormatedBitmapSource.PixelHeight * stride;
            byte[] pixels = new byte[size];
            Array.Clear(pixels, 0, size);
            newFormatedBitmapSource.CopyPixels(pixels, stride, 0);

            for (int y = 0; y < newFormatedBitmapSource.PixelHeight; y++)
            {

                for (int x = 0; x < newFormatedBitmapSource.PixelWidth; x++)
                {
                    int index = y * stride + 3 * x;
                    Color cur_col = GetPixelColor(pixels, x, y, stride);
                    if (cur_col == color)
                    {
                        if (bContour)
                        {
                            if (x > 0 && y > 0 && x < newFormatedBitmapSource.PixelWidth - 1 
                                && y < newFormatedBitmapSource.PixelHeight - 1)
                            {
                                KeyValuePair<int, int>[] P = new KeyValuePair<int, int>[] 
                            { 
                                new KeyValuePair<int,int> ( x - 1, y ), 
                                new KeyValuePair<int,int> ( x + 1, y ), 
                                new KeyValuePair<int,int> ( x, y - 1 ), 
                                new KeyValuePair<int,int> ( x, y + 1 )
                                //,
                                //new KeyValuePair<int,int> ( x + 1, y + 1 ),
                                //new KeyValuePair<int,int> ( x + 1, y - 1 ),
                                //new KeyValuePair<int,int> ( x - 1, y - 1 ),
                                //new KeyValuePair<int,int> ( x - 1, y + 1 ),
                            };
                                for (int i = 0; i < P.Length; i++)
                                {
                                    Color nColor = GetPixelColor(pixels, P[i].Key, P[i].Value, stride);
                                    if (nColor != color)
                                    {
                                        m_selected_points.Add(P[i]);
                                    }
                                }
                            }
                            else
                            {
                                m_selected_points.Add(new KeyValuePair<int, int>(x, y));
                            }
                        }
                        else
                        {
                            m_selected_points.Add(new KeyValuePair<int, int>(x, y));
                            pixels[index] = 0;
                            pixels[index + 1] = 255;
                            pixels[index + 2] = 0;
                        }                        
                    }
                }
            }
            m_selected_points = m_selected_points.Distinct().ToList();
            for (int i = 0; i < m_selected_points.Count; i++)
            {
                KeyValuePair<int, int> pair = m_selected_points[i];
                int index = pair.Value * stride + 3 * pair.Key;
                pixels[index] = 0;
                pixels[index + 1] = 255;
                pixels[index + 2] = 0;
                pair = new KeyValuePair<int, int>(pair.Key, newFormatedBitmapSource.PixelHeight - pair.Value - 1);
                m_selected_points[i] = pair; 
            }

            PictureUserControl.loaded_image.Source = BitmapSource.Create(newFormatedBitmapSource.PixelWidth, newFormatedBitmapSource.PixelHeight,
                newFormatedBitmapSource.DpiX, newFormatedBitmapSource.DpiY, newFormatedBitmapSource.Format,
                null, pixels, stride);
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
            
            lock (m_out_list)
            {
                m_out_list.Enqueue(command);
            }
        }

        private void ProcessCommand()
        {
            while (!StopThread)
            {
                int SleepVal = 10;
                if (m_ControlWrapper.IsControllerAvailable())
                {
                    lock (m_out_list)
                    {
                        if (m_out_list.Count <= 0)
                        {
                            SleepVal = 100;

                            ControlUserControl.textBlockInfo.Dispatcher.BeginInvoke(new Action(delegate()
                            {
                                ControlUserControl.textBlockInfo.Text = m_ControlWrapper.GetCurText(); //where item is the item to be added and listbox is the control being updated.
                            }));
            
                        }
                        else
                        {
                            Byte[] buf = m_out_list.Dequeue();
                            m_ControlWrapper.WriteCommandToController(buf);
                        }
                    }
                }
                
                Thread.Sleep(SleepVal);
            }
        }

        internal class xyz_coord
        {
            public int x = 0;
            public int y = 0;
            public int b = 0;
            public int w = 0;
        }

        [StructLayout(LayoutKind.Sequential, Size = 1 * ControlWrapper.MOTORS_COUNT), Serializable]
        public class cruise_motors
        {
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = ControlWrapper.MOTORS_COUNT)]
            public byte[] m_is_cruiser = new byte[ControlWrapper.MOTORS_COUNT];
            [MarshalAs(UnmanagedType.I1)]
            public byte m_signal_on_zero;
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
        internal class do_timer_set
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
        xyz_coord m_cur_pos = new xyz_coord();
                                    //steps max x8 x16
        public const int X_POS = 2; //2225 //1150 - x16
        public const int Y_POS = 3; //1900 //950  -x16
        public const int B_POS = 1; //12000 
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

        do_timer_set m_curtimerset = new do_timer_set();

        public void SetTimerSettings(UInt16 timer_res, UInt16 strike_impuls, byte[] multiplier)
        {
            do_timer_set timerset = new do_timer_set();
            timerset.m_timer_res = (UInt16)(UInt16.MaxValue - timer_res);
            timerset.m_strike_impuls = strike_impuls;
            timerset.m_multiplier[X_POS] = multiplier[0];
            timerset.m_multiplier[Y_POS] = multiplier[1];
            timerset.m_multiplier[B_POS] = multiplier[2];
            timerset.m_multiplier[W_POS] = multiplier[3];
            
            
            byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];
            OutputPacketBuffer[0] = 0;
            OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_TIME;
            ControlWrapper.StructureToByteArray(timerset, OutputPacketBuffer, 2);
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

        public void UpdateCurrentPosition()
        {
            ImageSource image = PictureUserControl.loaded_image.Source;
            if (image == null)
            {
                return;
            }
            try
            {
                double xratio = PictureUserControl.image_canvas.ActualWidth / image.Width;
                double yratio = PictureUserControl.image_canvas.ActualHeight / image.Height;
                PictureUserControl.UpdateCurrentPosition((m_cur_pos.x * MMPerXYStep - mProcessor.min_x) * xratio,
                    (image.Height - (m_cur_pos.y * MMPerXYStep - mProcessor.min_y
                    ) - 1) * yratio);
            }
            catch (Exception e)
            {
            }
        }

        public void SetCruisersToController()
        {
            cruise_motors cm = new cruise_motors();
            cm.m_signal_on_zero = 0;
            for (int motor = 0; motor < ControlWrapper.MOTORS_COUNT; motor++)
            {
                cm.m_is_cruiser[motor] = Convert.ToByte(motor == X_POS || motor == Y_POS);
            }
            byte[] OutputPacketBuffer = new byte[ControlWrapper.LEN_OF_PACKET];	//Allocate a memory buffer equal to our endpoint size + 1
            OutputPacketBuffer[0] = 0;				//The first byte is the "Report ID" and does not get transmitted over the USB bus.  Always set = 0.
            OutputPacketBuffer[1] = ControlWrapper.COMMAND_SET_CRUISERS;
            ControlWrapper.StructureToByteArray(cm, OutputPacketBuffer, 2);
            AddCommand(OutputPacketBuffer);
        }
        public void SetStepsToController(do_steps steps, bool update_pos = true)
        {
	        if(
		        steps.m_uSteps[X_POS] == 0 &&
		        steps.m_uSteps[Y_POS] == 0 &&
                steps.m_uSteps[B_POS] == 0 &&
		        steps.m_uSteps[W_POS] == 0)
	        {
		        return;
	        }
            if (update_pos)
            {
                m_cur_pos.y += steps.m_uSteps[Y_POS];
                m_cur_pos.x += steps.m_uSteps[X_POS];
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

            UpdateCurrentPosition();
        }

        int m_nTimerCounter = 0;
        public void GoToXY(int x, int y, int b = -1, int w = -1)
        {
            MainWindow.do_steps var_do_steps = new MainWindow.do_steps();

            //if (x >= 0)
            {
                var_do_steps.m_uSteps[X_POS] = x - m_cur_pos.x;
            }

            //if (y >= 0)
            {
                var_do_steps.m_uSteps[Y_POS] = y - m_cur_pos.y;
            }

            if (Math.Abs(var_do_steps.m_uSteps[X_POS]) > 10 || Math.Abs(var_do_steps.m_uSteps[Y_POS]) > 10)
            {
                ControlUserControl.SetTimerSetting(1);
                m_nTimerCounter = 0;
            }
            m_nTimerCounter++;
            if (m_nTimerCounter > 2)
            {
                ControlUserControl.SetTimerSetting(0);
                m_nTimerCounter = 0;
            }
            
            
            //if (b >= 0)
            {
                var_do_steps.m_uSteps[B_POS] = b - m_cur_pos.b;
            }
            //if (w >= 0)
            {
                var_do_steps.m_uSteps[W_POS] = w - m_cur_pos.w;
            }
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
        double GetDistanse(xyz_coord pt1, xyz_coord pt2)
        {
            double xx = (double)pt1.x - pt2.x;
            double yy = (double)pt1.y - pt2.y;
            double dist = Math.Sqrt((double)(xx * xx + yy * yy));
            return dist;
        }

        bool IsNeighbour(xyz_coord pt1, xyz_coord pt2)
        {
            int xx = Math.Abs(pt1.x - pt2.x);
            int yy = Math.Abs(pt1.y - pt2.y);
            if((yy == 0 && xx == 1) || (xx == 0 && yy == 1))
            {
                return true;
            }
            if (yy == 1 && xx == 1)
            {
                return true;
            }
            return false;
        }

        bool HasNeighbour(ref xyz_coord cur_coords)
        {
            xyz_coord temp_pt = new xyz_coord();
            for (int i = 0; i < m_selected_points.Count; i++)
            {
                temp_pt.x = m_selected_points[i].Key;
                temp_pt.y = m_selected_points[i].Value;
                if (IsNeighbour(cur_coords, temp_pt))
                {
                    return true;
                }
            }
            return false;
        }

        private bool DoEngraving()
        {
            if (MotionBlocks.Count <= 0 || m_counter < 0 || m_counter >= MotionBlocks.Count)
            {
                return false;
            }

            clsMotionRecord cur_rec = MotionBlocks[m_counter];

            if (cur_rec.MotionType == Motion.LINE)
            {
                Vector3 vec_start = new Vector3(cur_rec.Xold, cur_rec.Yold, cur_rec.Zold);
                Vector3 vec_end = new Vector3(cur_rec.Xpos, cur_rec.Ypos, cur_rec.Zpos);
                Vector3 vecSub = vec_end - vec_start;
                float[] mid_coord = new float[] { vecSub.X, vecSub.Y, vecSub.Z };
                Array.Sort(mid_coord);
                if (mid_coord[1] > 0.2)
                {
                    float steps = mid_coord[1] / 0.2f;
                    float step = 1.0f / steps;
                    for (float i = step; i < 1.0f; i += step)
                    {
                        Vector3 interpolation = Vector3.Lerp(vec_start, vec_end, i);
                        GoToXY(
                            GetStepsFromXYmm(interpolation.X),
                            GetStepsFromXYmm(interpolation.Y),
                            GetStepsFromBmm(interpolation.Z));
                    }
                }
            }
            
            
            GoToXY(
                GetStepsFromXYmm(cur_rec.Xpos),
                GetStepsFromXYmm(cur_rec.Ypos),
                GetStepsFromBmm(cur_rec.Zpos));
            
            

            m_counter++;
            ControlUserControl.textBlockCounter.Text = m_counter.ToString();

            return true;
        }

        public void Start()
        {
            m_counter = 0;
            if (newFormatedBitmapSource != null)
            {
                newFormatedBitmapSource.Freeze();
            }
            ControlUserControl.OnSelectionChanged();
            
            dispatcherTimer.Tick += new EventHandler(dispatcherTimer_Tick);
            dispatcherTimer.Interval = TimeSpan.FromMilliseconds(10);
            dispatcherTimer.Start(); 
        }

        int GetStepsFromBmm(double Bmm)
        {
            double ret = Bmm / MMPerBStep;
            return (int)ret;
        }
        int GetStepsFromXYmm(double XYmm)
        {
            double ret = XYmm / MMPerXYStep;
            return (int)ret;
        }
        public bool m_bEmulation = false;
        private void dispatcherTimer_Tick(object sender, EventArgs e)
        {
            if (m_bPauseSoft)
            {
                return;
            }

            if (m_bEmulation)
            {
                //Thread.Sleep(1);
                lock (m_out_list)
                m_out_list.Clear();
            }
            if (GetQueueLen() < 10)
            {
                if (DoEngraving())
                {
                    
                }
                else
                {
                    Thread.Sleep(1000);
                    GoToXY(-1, -1, 0);
                    GoToXY(0, 0);
                    ControlUserControl.checkBoxPauseSoft.IsChecked = true;
                    dispatcherTimer.Stop();
                    //ControlUserControl.checkBoxOutpusEnergy.IsChecked = false;
                    m_bEmulation = false;
                }
            }
        } 
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            //Properties.Settings.Default.TimerRes = "50";
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


        private string mCncFile = string.Empty;
        private clsProcessor mProcessor = clsProcessor.Instance();
        private clsSettings mSetup = new clsSettings();
        public static List<clsMotionRecord> MotionBlocks = new List<clsMotionRecord>();

        private void ProcessFile(string fileName)
        {
            mSetup = new clsSettings();
            MG_CS_BasicViewer.MotionBlocks.Clear();
            mProcessor.Init(mSetup.Machine);
            mProcessor.ProcessFile(fileName, MotionBlocks);
        } 

        public bool OnGCodeFileOpen(string fileName)
        {
            try
            {
                mCncFile = fileName;
                mSetup.MatchMachineToFile(mCncFile);
                ProcessFile(mCncFile);
                DrawRelief();
            }
            catch (Exception ex)
            {
                return false;
            }
            return true;
        }

        public void DrawRelief()
        {
            int width = (int)(mProcessor.max_x - mProcessor.min_x);
            int height = (int)(mProcessor.max_y - mProcessor.min_y);

            var target = new RenderTargetBitmap(width, height,
                96,96, PixelFormats.Pbgra32);

            PictureUserControl.loaded_image.Source = target;

            var visual = new DrawingVisual();

            SolidColorBrush red = new SolidColorBrush(Color.FromRgb(255,0,0));
            SolidColorBrush green = new SolidColorBrush(Color.FromRgb(0, 255, 0));
            using (var r = visual.RenderOpen())
            {
                for (int i = 0; i < MotionBlocks.Count; i++)
                {
                    //double xratio = PictureUserControl.image_canvas.ActualWidth / newFormatedBitmapSource.PixelWidth;
                    //double yratio = PictureUserControl.image_canvas.ActualHeight / newFormatedBitmapSource.PixelHeight;

                    //PictureUserControl.UpdateCurrentPosition((m_cur_pos.x * MMPerXYStep - mProcessor.min_x) * xratio,
                    //    (newFormatedBitmapSource.PixelHeight - (m_cur_pos.y * MMPerXYStep - mProcessor.min_y
                    //    ) - 1) * yratio);

                    Point pt1 = new Point(MotionBlocks[i].Xold - mProcessor.min_x, MotionBlocks[i].Yold - mProcessor.min_y);
                    Point pt2 = new Point(MotionBlocks[i].Xpos - mProcessor.min_x, MotionBlocks[i].Ypos - mProcessor.min_y);
                    byte Zold = (byte)(255 * MotionBlocks[i].Zold / 10);
                    byte Zpos = (byte)(255 * MotionBlocks[i].Zpos / 10);

                    
                    LinearGradientBrush aGradientBrush =
                        new LinearGradientBrush(Color.FromRgb(Zold, Zold, Zold), Color.FromRgb(Zpos, Zpos, Zpos),
                    new Point(0, 0), new Point(1, 1));

                    r.DrawLine(new Pen(aGradientBrush, 1),
                    pt1,
                    pt2);
                    //r.DrawEllipse(red, new Pen(red, 1), pt1, 1, 1);
                    //r.DrawEllipse(green, new Pen(green, 1), pt2, 1, 1);
                }
            }

            target.Render(visual);
            
        }
    }
}

//https://www.redblobgames.com/grids/line-drawing.html