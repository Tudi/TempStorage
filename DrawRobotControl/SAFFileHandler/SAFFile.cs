using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.IO;

namespace SAFFileHandler
{
    public interface ISAFFile
    {
        /// <summary>
        /// Read a SAF file into internal structure.
        /// Function used to preview a SAF file
        /// </summary>
        /// <param name="fileName"></param>
        /// <returns></returns>
        public int ReadFile(string fileName);
        /// <summary>
        /// After constructing a SAF file, function will agregate the data and write it to the output file
        /// </summary>
        /// <param name="fileName"></param>
        /// <returns></returns>
        public int WriteFile(string fileName);
        /// <summary>
        /// Start drawing a new line in this "transition" section, without actually adding any points into the line
        /// </summary>
        public void AddNewLine();
        /// <summary>
        /// Append a new point to the last created line
        /// </summary>
        /// <param name="nextX"></param>
        /// <param name="nextY"></param>
        public void AppendToLine(float nextX, float nextY);
        /// <summary>
        /// Create a new line in the last "transition" section and add a point to it
        /// </summary>
        /// <param name="firstX"></param>
        /// <param name="firstY"></param>
        public void AddNewLine(float firstX, float firstY);
        /// <summary>
        /// A transition is when the robot will ask for user to choose for either a head reposition of a paper swap action
        /// </summary>
        public void AppendTransition();

        /// <summary>
        /// Limited to 8 characters. Will be displayed on the LCD screen
        /// </summary>
        /// <param name="newName"></param>
        public void SetDisplayName(string newName);
        /// <summary>
        /// Description displayed on the LCD screen below the name of this file
        /// Max 64 characters
        /// </summary>
        /// <param name="newName"></param>
        public void SetDisplayDescription(string newName);

        /// <summary>
        /// Name of the current active section
        /// </summary>
        /// <param name="newName"></param>
        public void SetSectionDisplayName(string newName);

        /// <summary>
        /// Description of the current active section
        /// </summary>
        /// <param name="newDescription"></param>
        public void SetSectionDisplayDescription(string newDescription);

        /// <summary>
        /// Append a SIG file content to the current active Trnasition section
        /// </summary>
        /// <param name="fileLines"></param>
        public void AppendSigFile(string[] fileLines);

        /// <summary>
        /// While debugging
        /// </summary>
        /// <param name="t"></param>
        public void IsEqualHeader(SAFFile t);

        /// <summary>
        /// SAF can be address(proceed to next file) mode or signiture(do drawing + swap paper) mode
        /// </summary>
        /// <param name="Enabled"></param>
        public void SetAddressMode(bool Enabled = true);
    }

    /************************************************************************************************************************
     * Full implementation of the interface functions
    ************************************************************************************************************************/
    public class SAFFile : ISAFFile
    {
        private byte[] header4CC = new byte[SAFConstants.SAF_4CC_SIZE];
        internal SAFFileInfo? fileInfo = new SAFFileInfo();
        internal SAFFileInfo2? fileInfo2 = new SAFFileInfo2();
        internal List<SAFTransitionData> sections = new List<SAFTransitionData>();

        private static class SAFConstants
        {
            public const int SAF_4CC_SIZE = 8;
            public const int SAF_4CC_VAL = 0x01464153;
            public const float SAF_INCH_MULTIPLIER = 25.4f;
            public const int SAF_IV_SIZE = 16;
            public const int SAF_HASH_SIZE = 32;
            public const int SAFFileInfo2_FLAG_ADDRESS_MODE = 1;
            public static int SAF_16BYTE_ALLIGN(int x) => (((x + 15) / 16) * 16);
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        internal class SAFFileInfo
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public byte[] LCDName = new byte[8]; // shown on the robot preview screen
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
            public byte[] LCDDescription = new byte[64];
            public int transitionCount1;
            public int val2; // always 0 ?
            public int transitionCount2;
            public int val4; // always 0 ?
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
            public float[] points = new float[16]; // maybe it's 2 rects ?
            public ushort flags1; // always 0 ?
            // needed for the size of the structure to be dividable by 16
            // probably filler data because some fields got deleted from previous versions
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
            public byte[] padding6 = new byte[6]; // always 6
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
            public byte[] padding16 = new byte[16]; // always 16
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        internal class SAFFileInfo2
        {
            public byte val0; // always 0xf ?
            public int val1; // always 0 ?
            public byte val2; // always 4 ?
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)] 
            public float[] val3 = new float[10]; // always :  0.00, 0.00, 0.50, 0.00, 2.00, 1.00, 1.00, 0.10, 0.00, 0.10
            public ushort flags; // can have value 2 or 3. Always seen 2
            // probably filler data because some fields got deleted from previous versions
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public byte[] padding16 = new byte[32]; // always 16
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class SAFTransitionInfo
        {
            public int prevSectionStartOffset; // Seen it take the value of block end offset
            public int sectionEndOffset;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public byte[] sectionName = new byte[8]; // should always be 0
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
            public byte[] sectionDescription = new byte[64]; // should always be 0 
                                                    // unsure about the start of the structure
            public float minX, minY, width, height;
            public float totalLineLen;
            public int pointCount; // maybe point count
            public int lineCount; // maybe line count
            // needed for the size of the structure to be dividable by 16
            // probably filler data because some fields got deleted from previous versions
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public byte[] padding4 = new byte[4]; // always 4
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
            public byte[] padding16 = new byte[16]; // always 16
        }

        public class SAFPolylinePoint
        {
            public static int GetRawSize() { return sizeof(float) + sizeof(float); }
            public void ReadFromRaw(byte[] buff, int bufSize, ref int index)
            {
                x = BitConverter.ToSingle(buff, index);
                index += sizeof(float);
                y = BitConverter.ToSingle(buff, index);
                index += sizeof(float);
            }
            public void WriteToRaw(byte[] buff, int bufSize, ref int index)
            {
                BitConverter.GetBytes(x).CopyTo(buff, index);
                index += sizeof(float);
                BitConverter.GetBytes(y).CopyTo(buff, index);
                index += sizeof(float);
            }
            public float x, y;
            // data only available based on some file/section flags. Could not find examples
//            public int pointFlags;
//            public char unk1; // only present if *(char *)(param_1 + 0x40) != '\0'
//            public float unk2; // only present if *(char *)(param_1 + 0x41) != '\0'
        }

        public class SAFPolyline
        {
            public List<SAFPolylinePoint> points = new List<SAFPolylinePoint>();

            public void ParseFromRawBuffer(byte[] buf, int bufSize, ref int index)
            {
                short pointCount = BitConverter.ToInt16(buf, index);
                index += sizeof(short);
                for (int i = 0; i < pointCount && index < bufSize; i++)
                {
                    SAFPolylinePoint newPoint = new SAFPolylinePoint();
                    newPoint.ReadFromRaw(buf, bufSize, ref index);
                    points.Add(newPoint);
                }
            }

            public void WriteToRawBuffer(byte[] buff, int bufSize, ref int index)
            {
                // every polyline requires at least 2 points to be drawn
                if(points.Count < 2)
                {
                    return;
                }
                if (index + sizeof(short) > bufSize)
                {
                    throw new ArgumentException("Buffer size is too small for writing polyline points.");
                }

                short pointCount = (short)points.Count;
                Buffer.BlockCopy(BitConverter.GetBytes(pointCount), 0, buff, index, sizeof(short));
                index += sizeof(short);

                foreach (var point in points)
                {
                    point.WriteToRaw(buff, bufSize, ref index);
                }
            }

            public int GetRawSize()
            {
                return sizeof(short) + points.Count * SAFPolylinePoint.GetRawSize();
            }
        }

        public class SAFTransitionData
        {
            public SAFTransitionInfo? transitionInfo = new SAFTransitionInfo();
            public List<SAFPolyline> lines = new List<SAFPolyline>();
        }

        public SAFFile()
        {
            Array.Clear(header4CC, 0, header4CC.Length);
            Array.Clear(fileInfo.padding6, 0, fileInfo.padding6.Length);
            Array.Clear(fileInfo.padding16, 0, fileInfo.padding16.Length);
            Array.Clear(fileInfo2.padding16, 0, fileInfo2.padding16.Length);

            // set actual values 
            Array.Copy(BitConverter.GetBytes(SAFConstants.SAF_4CC_VAL), 0, header4CC, 0, sizeof(int));

            for (int i = 0; i < fileInfo.padding6.Length; i++)
            {
                fileInfo.padding6[i] = (byte)6;
            }

            for (int i = 0; i < fileInfo.padding16.Length; i++)
            {
                fileInfo.padding16[i] = (byte)16;
            }

            fileInfo2.val0 = 15;
            fileInfo2.val2 = 4;
            fileInfo2.flags = 2;
            fileInfo2.val3[0] = 0.0f;
            fileInfo2.val3[1] = 0.0f;
            fileInfo2.val3[2] = 0.5f;
            fileInfo2.val3[3] = 0.0f;
            fileInfo2.val3[4] = 2.0f;
            fileInfo2.val3[5] = 1.0f;
            fileInfo2.val3[6] = 1.0f;
            fileInfo2.val3[7] = 0.1f;
            fileInfo2.val3[8] = 0.0f;
            fileInfo2.val3[9] = 0.1f;

            for (int i = 0; i < fileInfo2.padding16.Length; i++)
            {
                fileInfo2.padding16[i] = (byte)16;
            }
        }


        ~SAFFile()
        {
            Array.Clear(header4CC, 0, header4CC.Length);
            sections.Clear();
        }

        public void SetAddressMode(bool Enabled = true)
        {
            if (fileInfo2 != null)
            {
                if (Enabled == true)
                {
                    fileInfo2.flags = (ushort)(fileInfo2.flags | SAFConstants.SAFFileInfo2_FLAG_ADDRESS_MODE);
                }
                else
                {
                    fileInfo2.flags = (ushort)(fileInfo2.flags & (~SAFConstants.SAFFileInfo2_FLAG_ADDRESS_MODE));
                }
            }
        }

        private int GetDB4Data(FileStream f, SAFTransitionData out_db4)
        {
            if (f == null)
            {
                return -1;
            }

            if (out_db4.transitionInfo == null)
            {
                return -2;
            }

            int bytesNeededDB4 = out_db4.transitionInfo.lineCount * sizeof(short)
                + out_db4.transitionInfo.pointCount * SAFPolylinePoint.GetRawSize();
            bytesNeededDB4 = SAFConstants.SAF_16BYTE_ALLIGN(bytesNeededDB4);
            byte[] DB4Decrypted = new byte[bytesNeededDB4];

            if (DB4Decrypted == null)
            {
                Console.WriteLine("Critical allocation error");
                return -1;
            }

            SAFCrypto.ReadGenericEncryptedBlock(f, bytesNeededDB4, DB4Decrypted);

            int readIndex = 0;
            int lineIndex = 0;
            while (readIndex < bytesNeededDB4 && lineIndex < out_db4.transitionInfo.lineCount)
            {
                SAFPolyline line = new SAFPolyline();
                line.ParseFromRawBuffer(DB4Decrypted, bytesNeededDB4, ref readIndex);
                out_db4.lines.Add(line);
                lineIndex++;
            }

            return 0;
        }

        private T? BytesToStructure<T>(byte[] bytes) where T : class
        {
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            T? structure = (T?)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            handle.Free();
            return structure;
        }

        // read and verify content
        public int ReadFile(string fileName)
        {
            using (FileStream fs = new FileStream(fileName, FileMode.Open, FileAccess.Read))
            {
                fs.Read(header4CC, 0, header4CC.Length);
                if (BitConverter.ToInt32(header4CC, 0) != SAFConstants.SAF_4CC_VAL)
                {
                    Console.WriteLine("File does not seem to be a SAF file");
                    return -3;
                }

                byte[] fileInfoBytes = new byte[Marshal.SizeOf(typeof(SAFFileInfo))];
                SAFCrypto.ReadGenericEncryptedBlock(fs, fileInfoBytes.Length, fileInfoBytes);
                fileInfo = BytesToStructure<SAFFileInfo>(fileInfoBytes);
                if (fileInfo == null)
                {
                    Console.WriteLine("Allocation error");
                    return -4;
                }

                byte[] fileInfo2Bytes = new byte[Marshal.SizeOf(typeof(SAFFileInfo2))];
                SAFCrypto.ReadGenericBlock(fs, fileInfo2Bytes.Length, fileInfo2Bytes);
                fileInfo2 = BytesToStructure<SAFFileInfo2>(fileInfo2Bytes);

                int transitionsParsed = 0;
                while (fs.Position + 31 < fs.Length)
                {
                    SAFTransitionData td = new SAFTransitionData();
                    byte[] transitionInfoBytes = new byte[Marshal.SizeOf(typeof(SAFTransitionInfo))];
                    SAFCrypto.ReadGenericEncryptedBlock(fs, transitionInfoBytes.Length, transitionInfoBytes);
                    td.transitionInfo = BytesToStructure<SAFTransitionInfo>(transitionInfoBytes);

                    GetDB4Data(fs, td);
                    sections.Add(td);

                    transitionsParsed++;
                }

                if (fileInfo.transitionCount1 != transitionsParsed)
                {
                    Console.WriteLine($"Parse mismatch. Was expecting {fileInfo.transitionCount1} transitions, but read {transitionsParsed}");
                    return -4;
                }
            }

            return 0;
        }

        public void SetDisplayName(string newName)
        {
            if (fileInfo == null)
            {
                return;
            }
            int len = Math.Min(newName.Length, fileInfo.LCDName.Length - 1);
            Array.Copy(Encoding.ASCII.GetBytes(newName), fileInfo.LCDName, len);
            fileInfo.LCDName[len] = 0;
        }

        public void SetDisplayDescription(string newDescription)
        {
            if (fileInfo == null)
            {
                return;
            }
            int len = Math.Min(newDescription.Length, fileInfo.LCDDescription.Length - 1);
            Array.Copy(Encoding.ASCII.GetBytes(newDescription), fileInfo.LCDDescription, len);
            fileInfo.LCDDescription[len] = 0;
        }

        public void AppendTransition()
        {
            SAFTransitionData td = new SAFTransitionData();
            sections.Add(td);
        }

        public void AddNewLine()
        {
            if (sections.Count == 0)
            {
                AppendTransition();
            }
            SAFTransitionData td = sections[sections.Count - 1];
            SAFPolyline pl = new SAFPolyline();
            td.lines.Add(pl);
        }


        public void AddNewLine(float firstX, float firstY)
        {
            // file does not have any sections, create the first one
            if (sections.Count == 0)
            {
                AppendTransition();
            }
            var itr = sections.Last();
            SAFTransitionData td = itr;
            SAFPolyline pl = new SAFPolyline();
            SAFPolylinePoint pp = new SAFPolylinePoint();
            pp.x = firstX;
            pp.y = firstY;
            pl.points.Add(pp);
            td.lines.Add(pl);
        }

        public void AppendToLine(float nextX, float nextY)
        {
            // file does not have any sections, create the first one
            if (sections.Count == 0)
            {
                AddNewLine(nextX, nextY);
                return;
            }
            var itrSections = sections.Last();
            SAFTransitionData td = itrSections;
            if (td.lines.Count == 0)
            {
                AddNewLine(nextX, nextY);
                return;
            }
            var itrLines = td.lines.Last();
            SAFPolyline pl = itrLines;
            SAFPolylinePoint pp = new SAFPolylinePoint();
            pp.x = nextX;
            pp.y = nextY;
            pl.points.Add(pp);
        }

        private static byte[] StructToBytes<T>(T structure)
        {
            if (structure == null)
            {
                return new byte[1];
            }
            int size = Marshal.SizeOf<T>();
            byte[] bytes = new byte[size];
            IntPtr ptr = Marshal.AllocHGlobal(size);

            try
            {
                Marshal.StructureToPtr(structure, ptr, true);
                Marshal.Copy(ptr, bytes, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
            return bytes;
        }

        public int WriteFile(string fileName)
        {
            // make sure sections are up to date
            UpdateFileInfo();
            using (FileStream f = new FileStream(fileName, FileMode.Create))
            {
                byte[] tempBuff;
                int transitionsParsed = 0;
                f.Write(header4CC, 0, header4CC.Length);
                SAFCrypto.WriteGenericEncryptedBlock(f, StructToBytes(fileInfo), Marshal.SizeOf<SAFFileInfo>());
                SAFCrypto.WriteGenericBlock(f, StructToBytes(fileInfo2), Marshal.SizeOf<SAFFileInfo2>());

                foreach (SAFTransitionData td in sections)
                {
                    if (td.transitionInfo == null)
                    {
                        continue;
                    }
                    SAFCrypto.WriteGenericEncryptedBlock(f, StructToBytes(td.transitionInfo), Marshal.SizeOf(td.transitionInfo));

                    int bytesRequired = td.transitionInfo.sectionEndOffset - td.transitionInfo.prevSectionStartOffset;
                    tempBuff = new byte[bytesRequired];

                    int writeIndex = 0;
                    foreach (SAFPolyline pl in td.lines)
                    {
                        pl.WriteToRawBuffer(tempBuff, bytesRequired, ref writeIndex);
                    }

                    writeIndex = SAFConstants.SAF_16BYTE_ALLIGN(writeIndex);
                    SAFCrypto.WriteGenericEncryptedBlock(f, tempBuff, writeIndex);

                    transitionsParsed++;
                }
            }

            return 0;
        }

        // based on sections, update file info that needs to be written to file
        private void UpdateFileInfo()
        {
            if (fileInfo == null || fileInfo2 == null)
            {
                return;
            }

            fileInfo.transitionCount1 = fileInfo.transitionCount2 = sections.Count;

            int prevSectionStartOffset = 0;
            int sectionStartOffset = (int)(SAFConstants.SAF_4CC_SIZE +
                SAFConstants.SAF_IV_SIZE + Marshal.SizeOf(typeof(SAFFileInfo)) + SAFConstants.SAF_HASH_SIZE +
                Marshal.SizeOf(typeof(SAFFileInfo2)) + SAFConstants.SAF_HASH_SIZE);

            foreach (var section in sections)
            {
                // get the total line len
                double lineLen = 0;
                double minX = 10000, minY = 10000, maxX = -10000, maxY = -10000;
                int thisSectionSize = 2 * SAFConstants.SAF_IV_SIZE + Marshal.SizeOf(typeof(SAFTransitionInfo)) + 2 * SAFConstants.SAF_HASH_SIZE;
                int pointCount = 0;

                foreach (var line in section.lines)
                {
                    if(line.points.Count < 2)
                    {
                        continue;
                    }
                    thisSectionSize += (int)line.GetRawSize();
                    pointCount += (int)line.points.Count;
                    float prevX = line.points[0].x;
                    float prevY = line.points[0].y;

                    foreach (var point in line.points)
                    {
                        double dx = prevX - point.x;
                        double dy = prevY - point.y;
                        lineLen += Math.Sqrt(dx * dx + dy * dy);

                        prevX = point.x;
                        prevY = point.y;

                        if (point.x < minX)
                        {
                            minX = point.x;
                        }
                        if (point.x > maxX)
                        {
                            maxX = point.x;
                        }

                        if (point.y < minY)
                        {
                            minY = point.y;
                        }
                        if (point.y > maxY)
                        {
                            maxY = point.y;
                        }
                    }
                }
                thisSectionSize = SAFConstants.SAF_16BYTE_ALLIGN(thisSectionSize);

                if (section.transitionInfo == null)
                {
                    return;
                }
                section.transitionInfo.prevSectionStartOffset = prevSectionStartOffset;
                section.transitionInfo.sectionEndOffset = sectionStartOffset + thisSectionSize;
                section.transitionInfo.totalLineLen = (float)lineLen;
                section.transitionInfo.minX = (float)minX;
                section.transitionInfo.minY = (float)minY;
                section.transitionInfo.width = (float)(maxX - minX);
                section.transitionInfo.height = (float)(maxY - minY);
                section.transitionInfo.lineCount = (int)section.lines.Count;
                section.transitionInfo.pointCount = pointCount;

                prevSectionStartOffset = sectionStartOffset;
                sectionStartOffset += thisSectionSize;
            }
        }

        public void IsEqualHeader(SAFFile t)
        {
            if (!header4CC.SequenceEqual(t.header4CC))
            {
                Console.WriteLine("4CC mismatch");
            }
            if (fileInfo != null && t.fileInfo != null && !fileInfo.Equals(t.fileInfo))
            {
                Console.WriteLine("fileInfo mismatch");
            }
            if (fileInfo2 != null && t.fileInfo2 != null && !fileInfo2.Equals(t.fileInfo2))
            {
                Console.WriteLine("fileInfo2 mismatch");
            }
        }

        public void SetSectionDisplayName(string newName)
        {
            if (sections.Count == 0)
            {
                return;
            }
            var lastSection = sections[sections.Count - 1].transitionInfo;
            if (lastSection == null)
            {
                return;
            }

            int len = Math.Min(newName.Length, lastSection.sectionName.Length - 1);
            Array.Copy(Encoding.ASCII.GetBytes(newName), lastSection.sectionName, len);
            lastSection.sectionName[len] = 0;

        }

        public void SetSectionDisplayDescription(string newDescription)
        {
            if (sections.Count == 0)
            {
                return;
            }
            var lastSection = sections[sections.Count - 1].transitionInfo;
            if(lastSection == null)
            {
                return;
            }

            int len = Math.Min(newDescription.Length, lastSection.sectionDescription.Length - 1);
            Array.Copy(Encoding.ASCII.GetBytes(newDescription), lastSection.sectionDescription, len);
            lastSection.sectionDescription[len] = 0;
        }

        public void AppendSigFile(string[] fileLines)
        {
            //read until the end of file
            bool insideALine = false;
            foreach (string line in fileLines)
            {
                if (line == "PLINESTART")
                {
                    insideALine = true;
                    AddNewLine();
                }
                if (line == "PLINEEND")
                {
                    insideALine = false;
                }
                else if(insideALine == true && line.IndexOf(",") > 0)
                {
                    float curX, curY;
                    bool prase_ret1 = float.TryParse(line.Substring(0, line.IndexOf(",")), out curX);
                    bool prase_ret2 = float.TryParse(line.Substring(line.IndexOf(",") + 1), out curY);
                    if (prase_ret1 && prase_ret2)
                    {
                        AppendToLine(curX, curY);
                    }
                }
            }
        }

        public int GetTransitionCount()
        {
            if(fileInfo == null)
            {
                return 0;
            }
            return fileInfo.transitionCount1;
        }

        public SAFTransitionData? GetTransitionData(int index)
        {
            if(sections.Count <= index)
            {
                return null;
            }
            return this.sections[index];
        }
    }
}
