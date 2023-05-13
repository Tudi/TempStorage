using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;

namespace SigToBin
{
    public class MotionCalibrator
    {
        const string CALIBRATION_FILE_NAME = "SA2.cal";

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct PositionAdjustInfoHeader2
        {
            public int version; // make sure it's always first so we can take a peek
            public int fourCC;
            public int headerSize, infoSize; // sanity checks in case you forgot to increase version number
            public int width, height; // given in commands. Tear size was 10x10. We can squeeze less than 600 commands in an inch. Would need 6000x6000 map (that is too large!)
            public int originX, originY;
            public float scaleX, scaleY; // MapFile might get large unless scaled down
        };

        [Flags]
        public enum PositionAdjustInfoFlags2 : byte
        {
            X_IS_MEASURED = 1 << 0,
            Y_IS_MEASURED = 1 << 1,
            X_IS_SET = 1 << 2,
            Y_IS_SET = 1 << 3
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct Adjusted2DPos2
        {
            public double x, y;
            public bool HasValues;
        }

        // all the info required to make a line draw straight
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct PositionAdjustInfo2
        {
            public PositionAdjustInfoFlags2 flags; // not every location will be adjusted. Locations between known adjustments are averaged

            public bool HasX()
            {
                return (flags & PositionAdjustInfoFlags2.X_IS_SET) != 0;
            }

            public double GetNewX()
            {
                return shouldBeX;
            }

            public bool HasY()
            {
                return (flags & PositionAdjustInfoFlags2.Y_IS_SET) != 0;
            }

            public double GetNewY()
            {
                return shouldBeY;
            }

            public double shouldBeX; // line needs to be moved to this new specific location in order to look straight
            public double shouldBeY; // line needs to be moved to this new specific location in order to look straight
        };


        PositionAdjustInfoHeader2 adjustInfoHeader;
        PositionAdjustInfo2[] adjustInfoMap;

        const int POSITION_ADJUST_FILE_VERSION = 3;
        const int ADJUST_MAP_DEFAULT_WIDTH = 1600;
        const int ADJUST_MAP_DEFAULT_HEIGHT = 1600;
        const float DEFAULT_WIDTH_SCALER = 0.20f;
        const float DEFAULT_HEIGHT_SCALER = 0.20f;
        const string CALIBRATION_4CC = "cal ";

        private static readonly MotionCalibrator instance = new MotionCalibrator();
        private MotionCalibrator() { }
        static MotionCalibrator()
        {
            MotionCalibrator.Instance.adjustInfoHeader = new PositionAdjustInfoHeader2();
            MotionCalibrator.Instance.adjustInfoHeader.scaleX = DEFAULT_WIDTH_SCALER;
            MotionCalibrator.Instance.adjustInfoHeader.scaleY = DEFAULT_HEIGHT_SCALER;
            MotionCalibrator.Instance.adjustInfoHeader.width = (int)(ADJUST_MAP_DEFAULT_WIDTH);
            MotionCalibrator.Instance.adjustInfoHeader.height = (int)(ADJUST_MAP_DEFAULT_HEIGHT);
            MotionCalibrator.Instance.adjustInfoHeader.originX = MotionCalibrator.Instance.adjustInfoHeader.width / 2;
            MotionCalibrator.Instance.adjustInfoHeader.originY = MotionCalibrator.Instance.adjustInfoHeader.height / 2;

            MotionCalibrator.Instance.adjustInfoMap = null;
            // load static distortion map
            MotionCalibrator.Instance.LoadAdjusterMap();
        }
        public static MotionCalibrator Instance
        {
            get { return instance; }
        }

        public void Dispose()
        {
        }

        void LoadAdjusterMap()
        {
            FileStream fs;
            try
            {
                fs = new FileStream(CALIBRATION_FILE_NAME, FileMode.Open, FileAccess.Read);
            }
            catch (IOException)
            {
                return;
            }

            BinaryReader reader = new BinaryReader(fs);
            adjustInfoHeader.version = reader.ReadInt32();
            adjustInfoHeader.fourCC = reader.ReadInt32();
            adjustInfoHeader.headerSize = reader.ReadInt32();
            adjustInfoHeader.infoSize = reader.ReadInt32();
            adjustInfoHeader.width = reader.ReadInt32();
            adjustInfoHeader.height = reader.ReadInt32();
            adjustInfoHeader.originX = reader.ReadInt32();
            adjustInfoHeader.originY = reader.ReadInt32();
            adjustInfoHeader.scaleX = reader.ReadSingle();
            adjustInfoHeader.scaleY = reader.ReadSingle();

            if (adjustInfoHeader.fourCC != BitConverter.ToInt32(Encoding.ASCII.GetBytes(CALIBRATION_4CC), 0))
            {
                Console.WriteLine("Invalid calibration file");
                reader.Close();
                fs.Close();
                return;
            }
            if (adjustInfoHeader.version != POSITION_ADJUST_FILE_VERSION)
            {
                Console.WriteLine("Invalid calibration file version");
                reader.Close();
                fs.Close();
                return;
            }
            if (adjustInfoHeader.headerSize != Marshal.SizeOf<PositionAdjustInfoHeader2>())
            {
                Console.WriteLine("Invalid calibration file header size");
                reader.Close();
                fs.Close();
                return;
            }
            if (adjustInfoHeader.infoSize != Marshal.SizeOf<PositionAdjustInfo2>())
            {
                Console.WriteLine("Invalid calibration file content size");
                reader.Close();
                fs.Close();
                return;
            }

            adjustInfoMap = new PositionAdjustInfo2[adjustInfoHeader.width * adjustInfoHeader.height];
            if (adjustInfoMap == null)
            {
                Console.WriteLine("Failed to load calibration file content");
                reader.Close();
                fs.Close();
                return;
            }

            for (int i = 0; i < adjustInfoHeader.width * adjustInfoHeader.height; i++)
            {
                adjustInfoMap[i].flags = (PositionAdjustInfoFlags2)reader.ReadByte();
                adjustInfoMap[i].shouldBeX = reader.ReadDouble();
                adjustInfoMap[i].shouldBeY = reader.ReadDouble();
            }

            fs.Close();
        }

        PositionAdjustInfo2? GetAdjustInfoNoChange(int x, int y)
        {
            if (x < 0 || x >= adjustInfoHeader.width || y < 0 || y >= adjustInfoHeader.height)
            {
                return null;
            }

            return adjustInfoMap[adjustInfoHeader.width * y + x];
        }

        Adjusted2DPos2 GetAdjustedPos(double x, double y)
        {
            Adjusted2DPos2 ret = new Adjusted2DPos2();

            double x2 = ((double)x * adjustInfoHeader.scaleX + adjustInfoHeader.originX);
            double y2 = ((double)y * adjustInfoHeader.scaleY + adjustInfoHeader.originY);

            if ((int)Math.Floor(x2) == (int)Math.Ceiling(x2) && (int)Math.Floor(y2) == (int)Math.Ceiling(y2))
            {
                PositionAdjustInfo2? poi = GetAdjustInfoNoChange((int)x2, (int)y2);
                if (poi != null)
                {
                    if (poi.Value.HasX())
                    {
                        ret.x = (float)poi.Value.GetNewX();
                    }
                    else
                    {
                        return ret;
                    }
                    if (poi.Value.HasY())
                    {
                        ret.y = (float)poi.Value.GetNewY();
                    }
                    else
                    {
                        return ret;
                    }
                }
                ret.HasValues = true;
                return ret;
            }

            double[,] ax = new double[2, 2];
            double[,] ay = new double[2, 2];

            int valuesX = 0;
            int valuesY = 0;
            foreach (int x3 in new int[] { (int)Math.Floor(x2), (int)Math.Ceiling(x2) })
            {
                valuesX = 0;
                foreach (int y3 in new int[] { (int)Math.Floor(y2), (int)Math.Ceiling(y2) })
                {
                    PositionAdjustInfo2? poi = GetAdjustInfoNoChange(x3, y3);
                    if (poi != null)
                    {
                        if (poi.Value.HasX())
                        {
                            ax[valuesX, valuesY] = (float)poi.Value.GetNewX();
                        }
                        else
                        {
                            return ret;
                        }
                        if (poi.Value.HasY())
                        {
                            ay[valuesX, valuesY] = (float)poi.Value.GetNewY();
                        }
                        else
                        {
                            return ret;
                        }
                    }
                    valuesX++;
                }
                valuesY++;
            }

            double xCoef = x2 - Math.Floor(x2);
            double yCoef = y2 - Math.Floor(y2);
            double coef00 = (1 - xCoef) * (1 - yCoef);
            double coef10 = (1 - xCoef) * yCoef;
            double coef01 = xCoef * (1 - yCoef);
            double coef11 = xCoef * yCoef;
            double x_out = coef00 * ax[0, 0] + coef10 * ax[1, 0] + coef01 * ax[0, 1] + coef11 * ax[1, 1];
            double y_out = coef00 * ay[0, 0] + coef10 * ay[1, 0] + coef01 * ay[0, 1] + coef11 * ay[1, 1];

            ret.x = (float)x_out;
            ret.y = (float)y_out;
            ret.HasValues = true;

            return ret;
        }
        void AppendLineSegment(float sx, float sy, float ex, float ey, ref RelativePointsLine out_line, ref float leftOverX, ref float leftOverY)
        {
            double dx = ex - sx;
            double dy = ey - sy;
            if (dx == dy && dx == 0)
            {
                return;
            }

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
                    out_line.StoreNextPoint(xdiff, 0);
                    curx += xdiff;
                }
                if (ydiff != 0)
                {
                    writtenDiffY += ydiff;
                    out_line.StoreNextPoint(0, ydiff);
                    cury += ydiff;
                }
            }

            dx += leftOverX;
            dy += leftOverY;

            // fix rounding errors
            if (dx < 0)
            {
                while (writtenDiffX > (int)dx)
                {
                    writtenDiffX--;
                    out_line.StoreNextPoint(-1, 0);
                }
            }
            if (dx > 0)
            {
                while (writtenDiffX < (int)dx)
                {
                    writtenDiffX++;
                    out_line.StoreNextPoint(1, 0);
                }
            }
            if (dy < 0)
            {
                while (writtenDiffY > (int)dy)
                {
                    writtenDiffY--;
                    out_line.StoreNextPoint(0, -1);
                }
            }
            if (dy > 0)
            {
                while (writtenDiffY < (int)dy)
                {
                    writtenDiffY++;
                    out_line.StoreNextPoint(0, 1);
                }
            }

            leftOverX = (float)(dx - writtenDiffX);
            leftOverY = (float)(dy - writtenDiffY);
        }
        public void DrawLine(float sx, float sy, float ex, float ey, ref RelativePointsLine out_line)
        {
            double dx = ex - sx;
            double dy = ey - sy;
            float leftOverX = 0;
            float leftOverY = 0;
            if (dx == 0 && dy == 0)
            {
                return;
            }

            out_line.SetStartingPosition(sx, sy);

            // just to increase the draw accuracy. More points, more smoothness
            double out_lineDrawSteps;
            if (Math.Abs(dy) > Math.Abs(dx))
            {
                out_lineDrawSteps = Math.Abs(dy);
            }
            else
            {
                out_lineDrawSteps = Math.Abs(dx);
            }

            double xIncForStep = dx / out_lineDrawSteps;
            double yIncForStep = dy / out_lineDrawSteps;

            const int SUB_LINE_LEN = 25;
            for (double step = 0; step < out_lineDrawSteps; step += SUB_LINE_LEN)
            {
                double sx2 = sx + step * xIncForStep;
                double sy2 = sy + step * yIncForStep;
                double stepsEnd = (step + SUB_LINE_LEN);
                if (stepsEnd > out_lineDrawSteps)
                {
                    stepsEnd = out_lineDrawSteps;
                }
                double ex2 = sx + stepsEnd * xIncForStep;
                double ey2 = sy + stepsEnd * yIncForStep;

                // try sub pixel accuracy adjusting
                Adjusted2DPos2 aiStart = GetAdjustedPos((float)sx2, (float)sy2);
                Adjusted2DPos2 aiEnd = GetAdjustedPos((float)ex2, (float)ey2);
                if (aiStart.HasValues && aiEnd.HasValues)
                {
                    sx2 = aiStart.x;
                    sy2 = aiStart.y;
                    ex2 = aiEnd.x;
                    ey2 = aiEnd.y;
                }
                AppendLineSegment((float)sx2, (float)sy2, (float)ex2, (float)ey2, ref out_line, ref leftOverX, ref leftOverY);
            }

            out_line.SetEndPosition(ex, ey);
        }

    }
}
