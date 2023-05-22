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
    /// <summary>
    /// Class to draw lines that get compensated enough to look straight after being drawn by the robot
    /// </summary>
    public class MotionCalibrator
    {
        const string CALIBRATION_FILE_NAME = "SA2.cal"; // can change this as long as you rename the calibration file
        const int POSITION_ADJUST_FILE_VERSION = 3; // sanity check to not load unknown formatted calibration files
        const string CALIBRATION_4CC = "cal "; // sanity check to not load unknown file types

        /// <summary>
        /// Calibration file header
        /// </summary>
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

        /// <summary>
        /// Flags to store info for every possible calibrated location
        /// A measured location is something we printed by the robot and used a scanner to get movement feedback
        /// A set location is an estimated movement feedback using linear interpolation
        /// </summary>
        [Flags]
        public enum PositionAdjustInfoFlags2 : byte
        {
            X_IS_MEASURED = 1 << 0,
            Y_IS_MEASURED = 1 << 1,
            X_IS_SET = 1 << 2,
            Y_IS_SET = 1 << 3
        }

        /// <summary>
        /// Visual feedback information what the robot actually draws compared to expected drawing
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct PositionAdjustInfo2
        {
            public PositionAdjustInfoFlags2 flags; // not every location will be adjusted. Locations between known adjustments are averaged

            /// <summary>
            /// Axes has been set by either a direct measurement or by an estimator function
            /// </summary>
            /// <returns></returns>
            public bool HasX()
            {
                return (flags & PositionAdjustInfoFlags2.X_IS_SET) != 0;
            }

            /// <summary>
            /// A compensated position that would make this line draw straight compared to curved
            /// </summary>
            /// <returns></returns>
            public double GetNewX()
            {
                return shouldBeX;
            }

            /// <summary>
            /// Axes has been set by either a direct measurement or by an estimator function
            /// </summary>
            /// <returns></returns>
            public bool HasY()
            {
                return (flags & PositionAdjustInfoFlags2.Y_IS_SET) != 0;
            }

            /// <summary>
            /// A compensated position that would make this line draw straight compared to curved
            /// </summary>
            /// <returns></returns>
            public double GetNewY()
            {
                return shouldBeY;
            }

            public double shouldBeX; // line needs to be moved to this new specific location in order to look straight
            public double shouldBeY; // line needs to be moved to this new specific location in order to look straight
        };

        /// <summary>
        /// Data cached from disk. As was stored on disk
        /// </summary>
        PositionAdjustInfoHeader2 adjustInfoHeader;
        PositionAdjustInfo2[] adjustInfoMap;

        /// <summary>
        /// Avoid reloading this 'service' every time a line is drawn. Load it on Application startup
        /// Feel free to change the load even to 'onDemand' if it suits the application better
        /// </summary>
        private static readonly MotionCalibrator instance = new MotionCalibrator();
        private MotionCalibrator() { }
        static MotionCalibrator()
        {
            MotionCalibrator.Instance.adjustInfoMap = null;
            // load static distortion map
            MotionCalibrator.Instance.LoadAdjusterMap();
        }
        public static MotionCalibrator Instance
        {
            get { return instance; }
        }

        /// <summary>
        /// Cache calibration data from disk to memory for faster access
        /// </summary>
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

            adjustInfoHeader = new PositionAdjustInfoHeader2();
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

        /// <summary>
        /// Calibration data fetch with safety checks regarding memory boundaries
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns></returns>
        PositionAdjustInfo2? GetAdjustInfoNoChange(int x, int y)
        {
            if (x < 0 || x >= adjustInfoHeader.width || y < 0 || y >= adjustInfoHeader.height)
            {
                return null;
            }

            return adjustInfoMap[adjustInfoHeader.width * y + x];
        }

        struct Adjusted2DPos2
        {
            public double x, y;
            public bool HasValues;
        }

        /// <summary>
        /// Linear interpolate values between measured points.
        /// This function returns an estimated position between actually measured positions
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns></returns>
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
                        ret.x = poi.Value.GetNewX();
                    }
                    else
                    {
                        return ret;
                    }
                    if (poi.Value.HasY())
                    {
                        ret.y = poi.Value.GetNewY();
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
                            ax[valuesX, valuesY] = poi.Value.GetNewX();
                        }
                        else
                        {
                            return ret;
                        }
                        if (poi.Value.HasY())
                        {
                            ay[valuesX, valuesY] = poi.Value.GetNewY();
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

            ret.x = x_out;
            ret.y = y_out;
            ret.HasValues = true;

            return ret;
        }

        /// <summary>
        /// Lines drawn will be broken down to sub-lines. This function will draw aprox 1mm long line
        /// It will accumulate rounding errors and pass it down to the next line to avoid acumulated errors
        /// </summary>
        /// <param name="sx"></param>
        /// <param name="sy"></param>
        /// <param name="ex"></param>
        /// <param name="ey"></param>
        /// <param name="out_line"></param>
        /// <param name="leftOverX"></param>
        /// <param name="leftOverY"></param>
        void AppendLineSegment(double sx, double sy, double ex, double ey, ref RelativePointsLine out_line, ref double leftOverX, ref double leftOverY)
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

            double xIncForStep = dx / lineDrawSteps;
            double yIncForStep = dy / lineDrawSteps;
            double writtenX = -leftOverX;
            double writtenY = -leftOverY;
            double step = 0;
            do
            {
                step += 1;
                if (step > lineDrawSteps)
                {
                    step = lineDrawSteps;
                }
                double curXPos = step * xIncForStep;
                double curYPos = step * yIncForStep;
                double xdiff = curXPos - writtenX;
                double ydiff = curYPos - writtenY;

                if (xdiff <= -1.0)
                {
                    writtenX += -1.0;
                    out_line.StoreNextPoint(-1.0, 0);
                }
                else if (xdiff >= 1.0)
                {
                    writtenX += 1.0;
                    out_line.StoreNextPoint(1.0, 0);
                }
                if (ydiff <= -1.0)
                {
                    writtenY += -1.0;
                    out_line.StoreNextPoint(0, -1.0);
                }
                else if (ydiff >= 1.0)
                {
                    writtenY += 1.0;
                    out_line.StoreNextPoint(0, 1.0);
                }
            } while (step != lineDrawSteps);

            {
                double curXPos = lineDrawSteps * xIncForStep;
                double curYPos = lineDrawSteps * yIncForStep;
                leftOverX = curXPos - writtenX;
                leftOverY = curYPos - writtenY;
            }
        }

        public enum DrawLineErrorCode
        {
            ERR_NONE = 0,
            ERR_LINE_HAS_NO_LENGTH,
            ERR_LINE_FALLS_OUTSIDE_DRAW_AREA
        };
        /// <summary>
        /// Drawn a line in memory. Input parameters are in movement commands ( converted from inches )
        /// Input parameters are from cartesian 2D coordinate system
        /// The line points will be mapped to a multi cirved 3D surface in order for the line to look straight
        /// </summary>
        /// <param name="sx"></param>
        /// <param name="sy"></param>
        /// <param name="ex"></param>
        /// <param name="ey"></param>
        /// <param name="out_line"></param>
        /// <param name="leftOverX"></param>
        /// <param name="leftOverY"></param>
        /// <returns>0 = no error. Other error code</returns>
        public DrawLineErrorCode DrawLine(double sx, double sy, double ex, double ey, ref RelativePointsLine out_line,ref double leftOverX, ref double leftOverY)
        {
            double dx = ex - sx;
            double dy = ey - sy;
            if (dx == 0 && dy == 0)
            {
                return DrawLineErrorCode.ERR_LINE_HAS_NO_LENGTH;
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
            DrawLineErrorCode ret = DrawLineErrorCode.ERR_NONE;
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
                Adjusted2DPos2 aiStart = GetAdjustedPos(sx2, sy2);
                Adjusted2DPos2 aiEnd = GetAdjustedPos(ex2, ey2);
                if (aiStart.HasValues && aiEnd.HasValues)
                {
                    sx2 = aiStart.x;
                    sy2 = aiStart.y;
                    ex2 = aiEnd.x;
                    ey2 = aiEnd.y;
                }
                else
                {
                    ret = DrawLineErrorCode.ERR_LINE_FALLS_OUTSIDE_DRAW_AREA;
                }

                AppendLineSegment(sx2, sy2, ex2, ey2, ref out_line, ref leftOverX, ref leftOverY);
            }

            out_line.SetEndPosition(ex, ey);
            return ret;
        }

    }
}
