﻿using System.Drawing;
namespace GCode
{
    public class clsDisplayList
    {
        public bool InView = true;
        public bool Rapid = true;
        public int ParentIndex;
        public Color Color;
        public PointF[] Points;
    }
}