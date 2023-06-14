using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace SigToBin
{
    /// <summary>
    /// Interface class to draw a SIG file content into a BIN file
    /// Keeps tracks of the status of a BIN file in order to be able to add SIG files to it
    /// </summary>
    public class SigToBin
    {
        private const int PIXELS_IN_INCH = 522;
        private const float INVALID_VALUE = 100000;
        private BinFileWriter bfw;
        private string binFileName;
        public SigToBin()
        {
            bfw = null;
            binFileName = "";
        }
        public SigToBin(string pBinFileName)
        {
            bfw = null;
            binFileName = pBinFileName;
        }

        /// <summary>
        /// The output file name
        /// </summary>
        /// <param name="binName"></param>
        public void SetBinFileName(string binName)
        {
            if (bfw != null)
            {
                Console.WriteLine("Bin file already open. You can no longer change the name of it");
                return;
            }
            binFileName = binName;
        }

        /// <summary>
        /// After adding a SIG content, you might want to reposition the Robot head.
        /// By calling this function, the robot should pause in draw, and wait for an operator to manually reposition the head
        /// than resume drawing
        /// </summary>
        public void InsertHeadTransition()
        {
            if(bfw == null)
            {
                Console.WriteLine("Bin file is not yet opened. There is no point to create a transition at this point");
                return;
            }
            bfw.WriteBinTransition(0);
        }

        /// <summary>
        /// You can have multiple SIG files in a BIN file, you need to separate these by calling a paper swap transition
        /// </summary>
        public void InsertPaperTransition()
        {
            if (bfw == null)
            {
                bfw = new BinFileWriter(binFileName); // should add the disposable
            }
            bfw.WriteBinTransition(1);
        }

        /// <summary>
        /// Needs to be called manually after you added all the SIG files to the BIN file
        /// </summary>
        public void CloseBinFile()
        {
            if(bfw != null)
            {
                bfw.CloseFile();
            }
            bfw = null;
        }

        /// <summary>
        /// Create a new, or append to the existing BIN file content the content of a SIG file
        /// </summary>
        /// <param name="fileLines"></param>
        /// <returns>Encountered errors ?</returns>
        public bool AddSIGToBin(string[] fileLines)
        {
            if(fileLines == null)
            {
                return true;
            }
            // in case there is no BIN file yet, create one
            if (bfw == null)
            {
                bfw = new BinFileWriter(binFileName); // should add the disposable
            }
            bool encounteredErrors = false;

            // parse the SIG file and convert every line into relative pen movement
            float prevX = INVALID_VALUE, prevY = INVALID_VALUE;
            foreach (string line in fileLines)
            {
                // read next line
                float curX = INVALID_VALUE, curY = INVALID_VALUE;
                if (line == "PLINESTART")
                {
                    prevX = INVALID_VALUE;
                    prevY = INVALID_VALUE;
                }
                else if (line == "PLINEEND")
                {
                    prevX = INVALID_VALUE;
                    prevY = INVALID_VALUE;
                }
                else if (line == "Setting")
                {
                    break;
                }
                else
                {
                    var values = line.Split(',');
                    if (values.Length == 2)
                    {
                        curX = float.Parse(values[0]);
                        curY = float.Parse(values[1]);
                    }
                }
                // have enough data to write a new line to the BIN file
                if (prevX != INVALID_VALUE && prevY != INVALID_VALUE && curX != INVALID_VALUE && curY != INVALID_VALUE)
                {
                    // apply_translations;
                    if( bfw.AddLine(prevX * PIXELS_IN_INCH, prevY * PIXELS_IN_INCH, curX * PIXELS_IN_INCH, curY * PIXELS_IN_INCH) == true )
                    {
                        encounteredErrors = true;
                    }
                }
                else if (curX != INVALID_VALUE && curY != INVALID_VALUE)
                {
                    // apply_translations;
                }
                prevX = curX;
                prevY = curY;
            }

            return encounteredErrors;
        }

        /// <summary>
        /// Append a line to the currently opened BIN file
        /// </summary>
        /// <param name="sx"></param>
        /// <param name="sy"></param>
        /// <param name="ex"></param>
        /// <param name="ey"></param>
        public bool AddLine(float sx, float sy, float ex, float ey)
        {
            if (bfw == null)
            {
                bfw = new BinFileWriter(binFileName); // should add the disposable
            }
            return bfw.AddLine(sx * PIXELS_IN_INCH, sy * PIXELS_IN_INCH, ex * PIXELS_IN_INCH, ey * PIXELS_IN_INCH);
        }

        /// <summary>
        /// Set the pen movement speed. 100% means the pen moves fastest
        /// Setting draw speed only affect future drawn lines. 
        /// You can change draw speed multiple times
        /// </summary>
        /// <param name="penMoveSpeedPercent">100% = fastest speed, 50% = half speed, 0% = no movement</param>
        public void SetDrawSpeed(double penMoveSpeedPercent)
        {
            if (bfw == null)
            {
                bfw = new BinFileWriter(binFileName); // should add the disposable
            }
            bfw.SetDrawSpeed(penMoveSpeedPercent);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Probably the rest of the code you do not need to look at
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// <summary>
    /// Codes used by the robot as primary movement
    /// </summary>
    public enum PenRobotMovementCodesPrimary : byte
    {
        Move1_Down = 0x03,
        Move1_Left = 0x01,
        Move1_Right = 0x02,
        Move1_Up = 0x00,
        Move1_Uninitialized = 4,
        Move1_Values_Count = 4
    }

    /// <summary>
    /// Codes used by the robot as PenPosition
    /// </summary>
    public enum PenRobotPenPosition : byte
    {
        Pen_Down = 1,
        Pen_Up = 0,
        Pen_Invalid = 2
    };

    /// <summary>
    /// Single block of data that can issue a Robot action
    /// The location of the fields and the size of the fields are fixed. 
    /// </summary>
    public class RobotCommand
    {
        public PenRobotMovementCodesPrimary primaryDirection; // bits 0,1
        public byte Transition; // bit 2
        public byte penIsMoving; // bit 3 = val 0x08
        public byte alwaysZero; // bit 4
        public PenRobotPenPosition penPosition; // bit 5
        public byte secondaryDirection; // bit 6,7

        public RobotCommand()
        {
            primaryDirection = 0;
            Transition = 0;
            penIsMoving = 1;
            alwaysZero = 0;
            penPosition = 0;
            secondaryDirection = 0;
        }

        /// <summary>
        /// The fields need to fit in 8 bits. Pack the values 
        /// </summary>
        /// <returns></returns>
        public byte Pack()
        {
            int res = 0;

            res = res | (((int)primaryDirection & 0x3) << 0);
            res = res | (((int)Transition & 0x1) << 2);
            res = res | (((int)penIsMoving & 0x1) << 3);
            res = res | (((int)alwaysZero & 0x1) << 4);
            res = res | (((int)penPosition & 0x1) << 5);
            res = res | (((int)secondaryDirection & 0x3) << 6);

            return (byte)res;
        }

        /// <summary>
        /// Unpack a numerical value into different fields. For easier access
        /// </summary>
        /// <param name="val"></param>
        public void UnPack(byte val)
        {
            primaryDirection = (PenRobotMovementCodesPrimary)((val >> 0) & 0x3);
            Transition = (byte)((val >> 2) & 0x1);
            penIsMoving = (byte)((val >> 3) & 0x1);
            alwaysZero = (byte)((val >> 4) & 0x1);
            penPosition = (PenRobotPenPosition)((val >> 5) & 0x1);
            secondaryDirection = (byte)((val >> 6) & 0x3);
        }
    }

    /// <summary>
    /// You can't simply issue robot commands. You need to chain them together
    /// You also need to keep track of the Pen position to be able to issue relative movement commands
    /// </summary>
    public class RobotDrawSession
    {
        public double curx;
        public double cury;
        public RobotCommand prevCMD;
        public int linesWritten;
        public RobotDrawSession()
        {
            moveSpeedCoeff = 0;
            prevCMD = new RobotCommand();
            ResetOnTransition();
        }
        /// <summary>
        /// Reset the pen status. Needed when a transition is performed
        /// </summary>
        public void ResetOnTransition()
        {
            prevCMD.UnPack(0);
            curx = cury = 0;
            stepsWritten = 0;
            skipsWritten = 0;
            leftOverX = leftOverY = 0;
            linesWritten = 0;
        }

        public double moveSpeedCoeff; // 0 = 100% movement speed
        public int stepsWritten; // measure number of steps taken to know when to insert a delay step. Used to make pen move slower
        public int skipsWritten;

        // because some residual pixels remain at the end of the line. Try to append it to the next line
        public double leftOverX, leftOverY;
    }

    /// <summary>
    /// Class to hide the ugly implementation details from the user ( just to avoid confusion )
    /// You should not recycle a BinFileWriter class. Create a new instance if you wish to write a new file
    /// </summary>
    public class BinFileWriter
    {
        // Fixed magic numbers present in every BIN file
        private const int BIN_HEADER_BYTE_COUNT = 10;
        private const byte BIN_HEADER_BYTE = 0x08;

        private const int BIN_FOOTER_BYTE_COUNT1 = 10;
        private const byte BIN_FOOTER_BYTE1 = 0x08;
        private const int BIN_FOOTER_BYTE_COUNT2 = 10;
        private const byte BIN_FOOTER_BYTE2 = 0x00;

        // avoid adding more than 1 header footer. Also make it automatic for convenience
        private bool bHeaderWritten;
        private bool bFooterWritten;
        private string sFileName;
        private FileStream fBinFile;
        // keep track of the Robot head status
        private RobotDrawSession robotSession;

        public BinFileWriter(string fileName)
        {
            bHeaderWritten = false;
            bFooterWritten = false;
            robotSession = new RobotDrawSession();
            sFileName = fileName;
            fBinFile = null;
        }

        /// <summary>
        /// Set the pen movement speed. 100% means the pen moves fast
        /// </summary>
        /// <param name="penMoveSpeedPercent"></param>
        public void SetDrawSpeed(double penMoveSpeedPercent)
        {
            if (penMoveSpeedPercent > 0)
            {
                robotSession.moveSpeedCoeff = 100 - penMoveSpeedPercent;
            }
        }
        /// <summary>
        /// Open the output file for writing. Will overwrite the destination file if already exists
        /// </summary>
        private void OpenBinFile()
        {
            if (fBinFile != null)
            {
                Console.WriteLine("Bin file already open");
                return;
            }
            try
            {
                fBinFile = new FileStream(sFileName, FileMode.Create);
            }
            catch
            {
                Console.WriteLine("Failed to open bin file");
                return;
            }
        }

        /// <summary>
        /// Close the file. Also write the footer that must be present in every BIN file
        /// </summary>
        public void CloseFile()
        {
            if (fBinFile == null)
            {
                return;
            }
            if (bHeaderWritten == false)
            {
                return;
            }
            if (bFooterWritten == true)
            {
                return;
            }
            WriteBinFooter();
            bFooterWritten = true;
            fBinFile.Close();
            fBinFile = null;
        }

        /// <summary>
        /// Must be present in every BIN file to mark the start of a file
        /// </summary>
        private void WriteBinHeader()
        {
            for (int i = 0; i < BIN_HEADER_BYTE_COUNT; i++)
            {
                byte byteVal = BIN_HEADER_BYTE;
                fBinFile.WriteByte(byteVal);
            }
            robotSession.prevCMD.UnPack(BIN_HEADER_BYTE);
        }

        /// <summary>
        /// Must be present in every BIN file to mark the end of a file
        /// </summary>
        private void WriteBinFooter()
        {
            for (int i = 0; i < BIN_FOOTER_BYTE_COUNT1; i++)
            {
                byte byteVal = BIN_FOOTER_BYTE1;
                fBinFile.WriteByte(byteVal);
            }
            // temp test to see if this caused the head to not move at all
            for (int i = 0; i < BIN_FOOTER_BYTE_COUNT2; i++)
            {
                byte byteVal = BIN_FOOTER_BYTE2;
                fBinFile.WriteByte(byteVal);
            }
            robotSession.prevCMD.UnPack(BIN_FOOTER_BYTE2);
        }

        /// <summary>
        /// Byte sequence that triggers a transition on the Robot side. Either pauses for user input or swaps out paper
        /// </summary>
        public void WriteBinTransition(int writePaperSwap)
        {
            // make sure the Pen is raised
            robotSession.prevCMD.penPosition = PenRobotPenPosition.Pen_Up;
            fBinFile.Write(new byte[] { robotSession.prevCMD.Pack() }, 0, 1);

            RobotCommand cmd = new RobotCommand();
            cmd.Transition = 1; // 0x04
            cmd.penIsMoving = 1; // 0x08
            byte byteVal = cmd.Pack(); 
            for (int i = 0; i < 20; i++)
            {
                fBinFile.Write(new byte[] { byteVal }, 0, 1);
            }

            cmd.penIsMoving = 0; // 0x08
            byteVal = cmd.Pack();
            for (int i = 0; i < 20; i++)
            {
                fBinFile.Write(new byte[] { byteVal }, 0, 1);
            }

            // seems like this part generates the paper swap
            if (writePaperSwap != 0)
            {
                cmd.Transition = 0;
                cmd.penIsMoving = 1;
                byteVal = cmd.Pack();
                for (int i = 0; i < 4; i++)
                {
                    fBinFile.Write(new byte[] { byteVal }, 0, 1);
                }
            }

            // reset pen position, acumulated errors ..
            robotSession.ResetOnTransition();
            // this was the last command we added to the file
            robotSession.prevCMD.UnPack(byteVal);
        }

        /// <summary>
        /// Practically write a "RelativePoint" line into the BIN file
        /// Right now it does not support movement speed adjustment.
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public int WriteBinLine(ref RelativePointsLine line)
        {
            if(fBinFile == null)
            {
                return 1;
            }

            // this code section exists to try to make first line get drawn if the pen is 'up'
            {
                if (robotSession.linesWritten == 0)
                {
                    // in every example this sequence got added
                    if (line.GetPenPosition() == PenRobotPenPosition.Pen_Down)
                    {
                        RobotCommand tCMD = new RobotCommand();
                        tCMD.UnPack(0);
                        tCMD.penIsMoving = 1;
                        for (int i = 0; i < 4; i++)
                        {
                            fBinFile.WriteByte(tCMD.Pack());
                        }
                        robotSession.prevCMD = tCMD;
                    }
                }
                else if (robotSession.linesWritten == 1)
                {
                    // in every example this sequence got added
                    if (robotSession.prevCMD.penPosition == PenRobotPenPosition.Pen_Up)
                    {
                        RobotCommand tCMD = new RobotCommand();
                        tCMD.UnPack(0);
                        tCMD.penIsMoving = 1;
                        tCMD.penPosition = PenRobotPenPosition.Pen_Down;
                        tCMD.secondaryDirection = 3;
                        for (int i = 0; i < 4; i++)
                        {
                            fBinFile.WriteByte(tCMD.Pack());
                        }
                        robotSession.prevCMD = tCMD;
                    }
                }

                robotSession.linesWritten++;
            }

            RobotCommand CMD = robotSession.prevCMD;
            PenRobotMovementCodesPrimary prevPrimaryDirection = PenRobotMovementCodesPrimary.Move1_Uninitialized;

            CMD.alwaysZero = 0;
            CMD.penIsMoving = 1;
            CMD.penPosition = line.GetPenPosition();
            CMD.Transition = 0;

            for (int i = 0; i < line.GetPointsCount(); i++)
            {
                int shouldMoveAgain = 1;
                int xConsumed = 0;
                int yConsumed = 0;

                while (shouldMoveAgain != 0)
                {
                    shouldMoveAgain = 0;
                    PenRobotMovementCodesPrimary primaryDirection = PenRobotMovementCodesPrimary.Move1_Uninitialized;

                    if (line.GetDX(i) < 0)
                    {
                        if (line.GetDX(i) < xConsumed)
                        {
                            primaryDirection = PenRobotMovementCodesPrimary.Move1_Left;
                            xConsumed += -1;
                            shouldMoveAgain += (line.GetDX(i) < xConsumed ? 1 : 0);
                        }
                    }
                    else if (line.GetDX(i) > 0)
                    {
                        if (line.GetDX(i) > xConsumed)
                        {
                            primaryDirection = PenRobotMovementCodesPrimary.Move1_Right;
                            xConsumed += 1;
                            shouldMoveAgain += (line.GetDX(i) > xConsumed ? 1 : 0);
                        }
                    }

                    if (primaryDirection != PenRobotMovementCodesPrimary.Move1_Uninitialized)
                    {
                        if (prevPrimaryDirection != PenRobotMovementCodesPrimary.Move1_Uninitialized && prevPrimaryDirection != primaryDirection)
                        {
//                            fBinFile.WriteByte(CMD.Pack());
                        }

                        CMD.primaryDirection = primaryDirection;
                        CMD.secondaryDirection = (byte)~CMD.secondaryDirection;

                        fBinFile.WriteByte(CMD.Pack());

                        // adjust pen movement speed
                        robotSession.stepsWritten++;
                        if ((robotSession.stepsWritten + robotSession.skipsWritten) * robotSession.moveSpeedCoeff > robotSession.skipsWritten)
                        {
                            fBinFile.WriteByte(CMD.Pack());
                            robotSession.skipsWritten++;
                        }

                        prevPrimaryDirection = primaryDirection;
                        primaryDirection = PenRobotMovementCodesPrimary.Move1_Uninitialized;
                    }

                    if (line.GetDY(i) < 0)
                    {
                        if (line.GetDY(i) < yConsumed)
                        {
                            primaryDirection = PenRobotMovementCodesPrimary.Move1_Down;
                            yConsumed += -1;
                            shouldMoveAgain += (line.GetDY(i) < yConsumed ? 1 : 0);
                        }
                    }
                    else if (line.GetDY(i) > 0)
                    {
                        if (line.GetDY(i) > yConsumed)
                        {
                            primaryDirection = PenRobotMovementCodesPrimary.Move1_Up;
                            yConsumed += 1;
                            shouldMoveAgain += (line.GetDY(i) > yConsumed ? 1 : 0);
                        }
                    }

                    if (primaryDirection != PenRobotMovementCodesPrimary.Move1_Uninitialized)
                    {
                        if (prevPrimaryDirection != PenRobotMovementCodesPrimary.Move1_Uninitialized && prevPrimaryDirection != primaryDirection)
                        {
//                            fBinFile.WriteByte(CMD.Pack());
                        }

                        CMD.primaryDirection = primaryDirection;
                        CMD.secondaryDirection = (byte)~CMD.secondaryDirection;

                        fBinFile.WriteByte(CMD.Pack());

                        // adjust pen movement speed
                        robotSession.stepsWritten++;
                        if ((robotSession.stepsWritten + robotSession.skipsWritten) * robotSession.moveSpeedCoeff > robotSession.skipsWritten)
                        {
                            fBinFile.WriteByte(CMD.Pack());
                            robotSession.skipsWritten++;
                        }

                        prevPrimaryDirection = primaryDirection;
                    }
                }
            }

            // update robot session as it arrived to the destination
            robotSession.curx = line.GetEndX();
            robotSession.cury = line.GetEndY();
            robotSession.prevCMD = CMD;

            return 0;
        }

        /// <summary>
        /// Move the Pen to the start of the line, draw the actual line
        /// </summary>
        /// <param name="sx"></param>
        /// <param name="sy"></param>
        /// <param name="ex"></param>
        /// <param name="ey"></param>
        /// <returns>Encountered errors ?</returns>
        public bool AddLine(float sx, float sy, float ex, float ey)
        {
            if (fBinFile == null)
            {
                OpenBinFile();
            }
            if(fBinFile == null)
            {
                return false;
            }
            if (bHeaderWritten == false)
            {
                WriteBinHeader();
                bHeaderWritten = true;
            }

            bool encounteredErrors = false;
            if ((int)robotSession.curx != (int)sx || (int)robotSession.cury != (int)sy)
            {
                RelativePointsLine line = new RelativePointsLine();
                Console.WriteLine("Moving NODRAW pen from {0:F2},{1:F2} to {2:F2},{3:F2}", robotSession.curx, robotSession.cury, sx, sy);
//                line.DrawLineRelativeInMem(robotSession.curx, robotSession.cury, sx, sy);
                double leftOverX = robotSession.leftOverX;
                double leftOverY = robotSession.leftOverY;
                MotionCalibrator.DrawLineErrorCode drawErr = MotionCalibrator.Instance.DrawLine(robotSession.curx, robotSession.cury, sx, sy, ref line, ref leftOverX, ref leftOverY);
                if(drawErr != MotionCalibrator.DrawLineErrorCode.ERR_NONE)
                {
                    encounteredErrors = true;
                }
                if (line.GetPointsCount() > 0)
                {
                    line.SetPenPosition(PenRobotPenPosition.Pen_Up);
                    WriteBinLine(ref line);
                    robotSession.leftOverX = leftOverX;
                    robotSession.leftOverY = leftOverY;
                }
            }

            {
                RelativePointsLine line2 = new RelativePointsLine();
                Console.WriteLine("Draw line from {0:F2},{1:F2} to {2:F2},{3:F2}", sx, sy, ex, ey);
//              line2.DrawLineRelativeInMem(robotSession.curx, robotSession.cury, ex, ey);
                double leftOverX = robotSession.leftOverX;
                double leftOverY = robotSession.leftOverY;
                MotionCalibrator.DrawLineErrorCode drawErr = MotionCalibrator.Instance.DrawLine(robotSession.curx, robotSession.cury, ex, ey, ref line2, ref leftOverX, ref leftOverY);
                if (drawErr != MotionCalibrator.DrawLineErrorCode.ERR_NONE)
                {
                    encounteredErrors = true;
                }
                if (line2.GetPointsCount() > 0)
                {
                    line2.SetPenPosition(PenRobotPenPosition.Pen_Down);
                    WriteBinLine(ref line2);
                    robotSession.leftOverX = leftOverX;
                    robotSession.leftOverY = leftOverY;
                }
            }
            return encounteredErrors;
        }

    }
}
