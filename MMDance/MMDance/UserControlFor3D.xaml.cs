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
using System.Windows.Media.Media3D;

namespace MMDance
{
    /// <summary>
    /// Interaction logic for UserControlFor3D.xaml
    /// </summary>
    public partial class UserControlFor3D : UserControl
    {
        GeometryModel3D myGeometryModel = new GeometryModel3D();
        AxisAngleRotation3D ax3d = new AxisAngleRotation3D();
        private void dispatcherTimer_Tick(object sender, EventArgs e)
        {
            ax3d.Angle += 1;
        }
        public UserControlFor3D()
        {
            InitializeComponent();
            Model3DGroup myModel3DGroup = new Model3DGroup();

            ModelVisual3D myModelVisual3D = new ModelVisual3D();
            // Defines the camera used to view the 3D object. In order to view the 3D object, 
            // the camera must be positioned and pointed such that the object is within view  
            // of the camera.
            PerspectiveCamera myPCamera = new PerspectiveCamera();

            // Specify where in the 3D scene the camera is.
            myPCamera.Position = new Point3D(0, 0, -600);

            // Specify the direction that the camera is pointing.
            myPCamera.LookDirection = new Vector3D(0, 0, 1);

            // Define camera's horizontal field of view in degrees.
            myPCamera.FieldOfView = 80;

            // Asign the camera to the viewport
            myViewport3D.Camera = myPCamera;
            // Define the lights cast in the scene. Without light, the 3D object cannot  
            // be seen. Note: to illuminate an object from additional directions, create  
            // additional lights.
            AmbientLight _ambLight = new AmbientLight(System.Windows.Media.Brushes.White.Color);
            myModel3DGroup.Children.Add(_ambLight);

            // Add the geometry model to the model group.
            myModel3DGroup.Children.Add(myGeometryModel);

            // Add the group of models to the ModelVisual3d.
            myModelVisual3D.Content = myModel3DGroup;

            // 
            myViewport3D.Children.Add(myModelVisual3D);

            System.Windows.Threading.DispatcherTimer dispatcherTimer = new System.Windows.Threading.DispatcherTimer();
            dispatcherTimer.Tick += new EventHandler(dispatcherTimer_Tick);
            dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 30);
            dispatcherTimer.Start();
        }

        public void Calculate(List<Point> list)
        {
            MeshGeometry3D myMeshGeometry3D = new MeshGeometry3D();


            // Create a collection of vertex positions for the MeshGeometry3D. 
            Point3DCollection myPositionCollection = new Point3DCollection();

            // Create a collection of triangle indices for the MeshGeometry3D.
            Int32Collection myTriangleIndicesCollection = new Int32Collection();


            // Apply the mesh to the geometry model.
            myGeometryModel.Geometry = myMeshGeometry3D;

            ax3d = new AxisAngleRotation3D(new Vector3D(0, 2, 0), 1);
            RotateTransform3D myRotateTransform = new RotateTransform3D(ax3d);
            myGeometryModel.Transform = myRotateTransform;

            List<Point3DCollection>
                Discs = new List<Point3DCollection>();
            for (double Z = 0; Z < 100; Z+=10 )
            {
                Point3DCollection disc = new Point3DCollection();

                for (int i = 0; i < list.Count; i++ )
                {
                    double X = list[i].X;
                    double Y = list[i].Y;
                    disc.Add(new Point3D(X, Y, Z));
                }


                Discs.Add(disc);
            }

            LinearGradientBrush myHorizontalGradient = new LinearGradientBrush();
            myHorizontalGradient.StartPoint = new Point(0, 0);
            myHorizontalGradient.EndPoint = new Point(1, 1);
            myHorizontalGradient.GradientStops.Add(new GradientStop(Colors.White, 0.0));
            myHorizontalGradient.GradientStops.Add(new GradientStop(Colors.Black, 1.0));

            DiffuseMaterial myDiffuseMaterial = new DiffuseMaterial(myHorizontalGradient);
            MaterialGroup myMaterialGroup = new MaterialGroup();
            myMaterialGroup.Children.Add(myDiffuseMaterial);

            PointCollection myTextureCoordinatesCollection = new PointCollection();

            myMeshGeometry3D.TextureCoordinates = myTextureCoordinatesCollection;
            myGeometryModel.Material = myMaterialGroup;

            Triangle t1 = null;
            for (int i = 1; i < Discs.Count; i++)
            {
                Point3DCollection disc1 = Discs[i - 1];
                Point3DCollection disc2 = Discs[i];
                t1 = null;
                for (int j = 1; j < disc1.Count; j++)
                {
                    Point3D p1 = disc1[j - 1];
                    Point3D p2 = disc1[j];
                    Point3D p3 = disc2[j];
                    Point3D p4 = disc2[j - 1];

                    if (j == 1)
                    {
                        Vector3D v21 = p2 - p1;
                        Double distanceBetween21 = v21.Length;

                        Vector3D v43 = p4 - p3;
                        Double distanceBetween43 = v43.Length;

                        Vector3D v32 = p3 - p2;
                        Double distanceBetween32 = v32.Length;

                        Vector3D v41 = p4 - p1;
                        Double distanceBetween41 = v41.Length;

                        Double h = (distanceBetween43 - distanceBetween21) / 2;
                        h = Math.Sqrt(Math.Pow(distanceBetween32, 2) - Math.Pow(h, 2));
                    }

                    if (t1 != null)
                    {
                        Triangle t2 = new Triangle(p1, p2, p3);
                        double angle = t2.GetAngle(t1);
                    }
                    t1 = new Triangle(p1, p2, p3);



                    myTextureCoordinatesCollection.Add(new Point(p1.X, p1.Y));
                    myTextureCoordinatesCollection.Add(new Point(p2.X, p2.Y));
                    myTextureCoordinatesCollection.Add(new Point(p3.X, p3.Y));
                    myTextureCoordinatesCollection.Add(new Point(p4.X, p4.Y));


                    myPositionCollection.Add(p4);
                    myPositionCollection.Add(p3);
                    myPositionCollection.Add(p1);

                    myPositionCollection.Add(p1);
                    myPositionCollection.Add(p3);
                    myPositionCollection.Add(p2);
                }
            }

            myMeshGeometry3D.Positions = myPositionCollection;

            myMeshGeometry3D.TriangleIndices = myTriangleIndicesCollection;

        }
    }
}
