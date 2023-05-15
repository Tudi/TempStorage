using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SigToBin
{

    /// <summary>
    /// Generic 2D point that will be used as relative distance to previous location
    /// </summary>
    public class RelativeLinePoint
    {
        public float dx;
        public float dy;
        public RelativeLinePoint()
        {
            dx = dy = 0;
        }
    }

    /// <summary>
    /// Line that will contain multiple relativ distances to previous location
    /// In order to draw straight line by the robot, you need to be able to update this line to become a curve(because robot can only draw curves)
    /// This class could be inlined directly into robot movement, but it would create a huge function
    /// </summary>
    public class RelativePointsLine
    {
        private PenRobotPenPosition penPosition;
        private float startx, starty;
        private float endx, endy;
        private RelativeLinePoint[] moves;

        public RelativePointsLine()
        {
            moves = new RelativeLinePoint[0];
            penPosition = PenRobotPenPosition.Pen_Invalid; // invalid value
            startx = endx = starty = endy = 0;
        }

        ~RelativePointsLine()
        {
            Array.Clear(moves, 0, moves.Length);
            moves = null;
        }

        /// <summary>
        /// Maybe a bit of overengineered way of adding the next point to the line
        /// </summary>
        /// <param name="dx"></param>
        /// <param name="dy"></param>
        /// <returns></returns>
        public int StoreNextPoint(double dx, double dy)
        {
            Array.Resize(ref moves, moves.Length + 1);
            moves[moves.Length - 1] = new RelativeLinePoint();
            moves[moves.Length - 1].dx = (float)dx;
            moves[moves.Length - 1].dy = (float)dy;
            return 0;
        }

        public void SetPenPosition(PenRobotPenPosition penPos)
        {
            penPosition = penPos;
        }

        public PenRobotPenPosition GetPenPosition()
        {
            return penPosition;
        }

        public void SetStartingPosition(double sx, double sy)
        {
            startx = (float)sx;
            starty = (float)sy;
        }

        public void SetEndPosition(double ex, double ey)
        {
            endx = (float)ex;
            endy = (float)ey;
        }

        public int GetPointsCount()
        {
            return moves.Length;
        }

        public float GetDX(int at)
        {
            return moves[at].dx;
        }

        public float GetDY(int at)
        {
            return moves[at].dy;
        }

        public void SetDX(int at, float dx)
        {
            moves[at].dx = dx;
        }

        public void SetDY(int at, float dy)
        {
            moves[at].dy = dy;
        }

        public float GetStartX()
        {
            return startx;
        }

        public float GetStartY()
        {
            return starty;
        }

        public float GetEndX()
        {
            return endx;
        }

        public float GetEndY()
        {
            return endy;
        }
    }
}
