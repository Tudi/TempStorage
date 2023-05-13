using System;
using System.Diagnostics;
using System.IO;

namespace SigToBin
{
    /// <summary>
    /// Interface class to draw a SIG file content into a BIN file
    /// Keeps tracks of the status of a BIN file in order to be able to add SIG files to it
    /// </summary>
    public class SigToBin
    {
        private const int PIXELS_IN_INCH = 600;
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
        /// Try to be smart and come up with a file name in case none has been provided.
        /// Maybe it would be better to just throw an error instead
        /// </summary>
        /// <param name="SIGFileName"></param>
        private void genBinFileNameFromInputFileName(string SIGFileName)
        {
            // same file name, but different extension
            binFileName = Path.GetFileName(Path.ChangeExtension(SIGFileName, ".bin"));
        }
        /// <summary>
        /// In case you want some quick tests, you might want to convert a single file to BIN
        /// </summary>
        /// <param name="SIGFileName"></param>
        /// <returns></returns>
        private string[] GetSIGFileContent(string SIGFileName)
        {
            string[] fileLines = null;
            try
            {
                fileLines = System.IO.File.ReadAllLines(SIGFileName);
            }
            catch (System.IO.IOException ex)
            {
                // Handle file error here
                Console.WriteLine("An error occurred while reading the file: " + ex.Message);
            }
            return fileLines;
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
        public void AddSIGToBin(string[] fileLines)
        {
            if(fileLines == null)
            {
                return;
            }
            // in case there is no BIN file yet, create one
            if (bfw == null)
            {
                bfw = new BinFileWriter(binFileName); // should add the disposable
            }

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
                    bfw.AddLine(prevX * PIXELS_IN_INCH, prevY * PIXELS_IN_INCH, curX * PIXELS_IN_INCH, curY * PIXELS_IN_INCH);
                }
                else if (curX != INVALID_VALUE && curY != INVALID_VALUE)
                {
                    // apply_translations;
                }
                prevX = curX;
                prevY = curY;
            }
        }

        /// <summary>
        /// When you are unsure what to do, fall back to this function
        /// Simplest version of a convert example
        /// </summary>
        /// <param name="fileName"></param>
        public void ConvertSigFileToBin(string SIGFileName)
        {
            Console.WriteLine("Trying to convert {0} to BIN file", SIGFileName);

            string[] fileLines = GetSIGFileContent(SIGFileName);
            if(fileLines == null)
            {
                return;
            }
            if (binFileName == "")
            {
                genBinFileNameFromInputFileName(SIGFileName);
            }

            AddSIGToBin(fileLines);
            if (bfw != null)
            {
                bfw.CloseFile();
                bfw = null;
            }
        }

        /// <summary>
        /// Append a line to the currently opened BIN file
        /// </summary>
        /// <param name="sx"></param>
        /// <param name="sy"></param>
        /// <param name="ex"></param>
        /// <param name="ey"></param>
        public void AddLine(float sx, float sy, float ex, float ey)
        {
            if (bfw == null)
            {
                bfw = new BinFileWriter(binFileName); // should add the disposable
            }
            bfw.AddLine(sx * PIXELS_IN_INCH, sy * PIXELS_IN_INCH, ex * PIXELS_IN_INCH, ey * PIXELS_IN_INCH);
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
        public PenRobotMovementCodesPrimary primaryDirection;
        public byte Transition;
        public byte penIsMoving;
        public byte alwaysZero;
        public PenRobotPenPosition penPosition;
        public byte secondaryDirection;

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
        public float curx;
        public float cury;
        public RobotCommand prevCMD;
        public RobotDrawSession()
        {
            curx = cury = 0;
            prevCMD = new RobotCommand();
            prevCMD.UnPack(0);
        }
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
            // temp test to see if this caused the head to not move at all
            for (int i = 0; i < 4; i++)
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
            byte byteVal = 0x88; // does not seem to matter?
            fBinFile.Write(new byte[] { byteVal }, 0, 1);

            for (int i = 0; i < 20; i++)
            {
                byteVal = 0x08 | 0x04;
                fBinFile.Write(new byte[] { byteVal }, 0, 1);
            }

            // only part that does not have "always 1" bit 4 set
            // probably enough for a transition
            for (int i = 0; i < 20; i++)
            {
                byteVal = 0x04;
                fBinFile.Write(new byte[] { byteVal }, 0, 1);
            }

            robotSession.prevCMD.UnPack(0x04);

            // seems like this part generates the paper swap
            if (writePaperSwap != 0)
            {
                for (int i = 0; i < 4; i++)
                {
                    byteVal = 0x08;
                    fBinFile.Write(new byte[] { byteVal }, 0, 1);
                }

                robotSession.prevCMD.UnPack(0x08);
            }
        }

        /// <summary>
        /// Practically write a "RelativePoint" line into the BIN file
        /// Right now it does not support movement speed adjustment.
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public int WriteBinLine(ref RelativePointsLine line)
        {
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
        public void AddLine(float sx, float sy, float ex, float ey)
        {
            if (fBinFile == null)
            {
                OpenBinFile();
            }
            if (bHeaderWritten == false)
            {
                WriteBinHeader();
                bHeaderWritten = true;
            }

            if ((int)robotSession.curx != (int)sx || (int)robotSession.cury != (int)sy)
            {
                RelativePointsLine line = new RelativePointsLine();
                Console.WriteLine("Moving NODRAW pen from {0:F2},{1:F2} to {2:F2},{3:F2}", robotSession.curx, robotSession.cury, sx, sy);
//                line.DrawLineRelativeInMem(robotSession.curx, robotSession.cury, sx, sy);
                MotionCalibrator.Instance.DrawLine(robotSession.curx, robotSession.cury, sx, sy, ref line);
                if (line.GetPointsCount() > 0)
                {
                    line.SetPenPosition(PenRobotPenPosition.Pen_Up);
                    WriteBinLine(ref line);
                }
            }

            RelativePointsLine line2 = new RelativePointsLine();
            Console.WriteLine("Draw line from {0:F2},{1:F2} to {2:F2},{3:F2}", sx, sy, ex, ey);
            line2.SetStartingPosition(robotSession.curx, robotSession.cury);
//            line2.DrawLineRelativeInMem(robotSession.curx, robotSession.cury, ex, ey);
            MotionCalibrator.Instance.DrawLine(robotSession.curx, robotSession.cury, ex, ey, ref line2);
            if (line2.GetPointsCount() > 0)
            {
                line2.SetPenPosition(PenRobotPenPosition.Pen_Down);
                WriteBinLine(ref line2);
            }
        }

    }
}
