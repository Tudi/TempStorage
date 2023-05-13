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
    /// In order to draw straight line, you need to be able to update this line to become a curve(because robot can only draw curves)
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

        public void DrawLineRelativeInMem(float sx, float sy, float ex, float ey)
        {
            double dx = ex - sx;
            double dy = ey - sy;
            if (dx == dy && dx == 0)
            {
                return;
            }

            SetStartingPosition(sx, sy);

            // just to increase the draw accuracy. More points, more smoothness
            double lineDrawSteps;
            if (Math.Abs(dy) > Math.Abs(dx))
            {
                lineDrawSteps = Math.Abs(dy);
            }
            else
            {
                lineDrawSteps = Math.Abs(dx);
            }

            int curx = (int)sx;
            int cury = (int)sy;

            double xIncForStep = dx / lineDrawSteps;
            double yIncForStep = dy / lineDrawSteps;
            int writtenDiffX = 0;
            int writtenDiffY = 0;
            for (double step = 1; step <= lineDrawSteps; step += 1)
            {
                double curXPos = step * xIncForStep;
                double curYPos = step * yIncForStep;
                int xdiff = (int)curXPos - writtenDiffX;
                int ydiff = (int)curYPos - writtenDiffY;

                if (xdiff < -1)
                {
                    xdiff = -1;
                }
                else if (xdiff > 1)
                {
                    xdiff = 1;
                }
                if (ydiff < -1)
                {
                    ydiff = -1;
                }
                else if (ydiff > 1)
                {
                    ydiff = 1;
                }

                if (xdiff != 0)
                {
                    writtenDiffX += xdiff;
                    StoreNextPoint(xdiff, 0);
                    curx += xdiff;
                }
                if (ydiff != 0)
                {
                    writtenDiffY += ydiff;
                    StoreNextPoint(0, ydiff);
                    cury += ydiff;
                }
            }

            // fix rounding errors
            if (dx < 0)
            {
                while (writtenDiffX > (int)dx)
                {
                    writtenDiffX--;
                    StoreNextPoint(-1, 0);
                }
            }
            if (dx > 0)
            {
                while (writtenDiffX < (int)dx)
                {
                    writtenDiffX++;
                    StoreNextPoint(1, 0);
                }
            }
            if (dy < 0)
            {
                while (writtenDiffY > (int)dy)
                {
                    writtenDiffY--;
                    StoreNextPoint(0, -1);
                }
            }
            if (dy > 0)
            {
                while (writtenDiffY < (int)dy)
                {
                    writtenDiffY++;
                    StoreNextPoint(0, 1);
                }
            }

            SetEndPosition(sx + dx, sy + dy);

            Debug.Assert(writtenDiffX == (int)dx, "Did not arrive at destination X");
            Debug.Assert(writtenDiffY == (int)dy, "Did not arrive at destination y");
        }
    }
}
