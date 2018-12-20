using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    class NetworkPacketTools
    {
        public static unsafe string BytesToString_MAX_STN_CHARNO(byte* ptr)
        {
//            ptr[Constants.MAX_STN_CHARNO - 1] = 0; //make sure it is NULL terminated
            int srtlen = Constants.MAX_STN_CHARNO;
            for (int i = 0; i < Constants.MAX_STN_CHARNO; i++)
                if (ptr[i] == 0)
                {
                    srtlen = i;
                    break;
                }
            return Marshal.PtrToStringAnsi((IntPtr)ptr, srtlen);
//            return System.Text.ASCIIEncoding.ASCII.GetString(ptr, Constants.MAX_STN_CHARNO);
        }

        public static unsafe void StrinToBytes_MAX_STN_CHARNO(string str, byte* ptr)
        {
            fixed (char* c = str)
                ASCIIEncoding.ASCII.GetBytes(c,str.Length,ptr, Constants.MAX_STN_CHARNO);
        }

        public static int PCK_HDR_TRM_LEN()
        {
            BLFWinHeader hdr = new BLFWinHeader();
            return System.Runtime.InteropServices.Marshal.SizeOf(hdr);
        }

        /// <summary>
        /// Convert a structure to a byte array as memory
        /// </summary>
        /// <param name="p"></param>
        /// <returns></returns>
        static public byte[] StructureToByteArray<T>(T str)
        {
            int size = Marshal.SizeOf(str);
            byte[] arr = new byte[size];
            GCHandle h = default(GCHandle);
            try
            {
                h = GCHandle.Alloc(arr, GCHandleType.Pinned);
                Marshal.StructureToPtr<T>(str, h.AddrOfPinnedObject(), false);
            }
            finally
            {
                if (h.IsAllocated)
                    h.Free();
            }
            return arr;
        }

        /// <summary>
        /// convert a byte array to a specific type of structure. Does not check if conversion was valid
        /// </summary>
        /// <param name="bytearray"></param>
        /// <returns></returns>
        static public T ByteArrayToStructure<T>(byte[] bytearray) where T : struct
        {
            T str = default(T);
            GCHandle h = default(GCHandle);
            try
            {
                h = GCHandle.Alloc(bytearray, GCHandleType.Pinned);
                str = Marshal.PtrToStructure<T>(h.AddrOfPinnedObject());
            }
            finally
            {
                if (h.IsAllocated)
                    h.Free();
            }
            return str;
        }
        static public T ByteArrayToStructureC<T>(byte[] bytearray) where T : class
        {
            T str = default(T);
            GCHandle h = default(GCHandle);
            try
            {
                h = GCHandle.Alloc(bytearray, GCHandleType.Pinned);
                str = Marshal.PtrToStructure<T>(h.AddrOfPinnedObject());
            }
            finally
            {
                if (h.IsAllocated)
                    h.Free();
            }
            return str;
        }

        public static int StrToByteArray(string src, byte [] dst, int index, bool WriteSize = true, bool CopyTerminatingZero = true, int FixedSize = 0)
        {
            //sanity check
            if (src == null)
                return 0;
            int OriginalIndex = index;
            if(WriteSize == true)
                index += IntoToByteArray(src.Length, dst, index);
            //copy chars
            for (int i = 0; i < src.Length; i++)
                dst[index++] = (byte)src[i];
            //is this a C type string ?
            if (CopyTerminatingZero == true)
                dst[index++] = 0;
            //if we wish to write X chars
            if(FixedSize>0)
                for(int i= index - OriginalIndex;i<FixedSize;i++)
                    dst[index++] = 0;
            return (index - OriginalIndex);
        }

        public static string ByteArrayToStr(byte [] src, int index, int size )
        {
            //sanity check
            if (src == null)
                return null;
            string ret = "";
            for (int i = 0; i < size; i++)
                ret = ret + (char)src[index + i];
            return ret;
        }

        public static int IntoToByteArray(int src, byte [] dst, int index)
        {
            dst[index + 3] = (byte)src;
            dst[index + 2] = (byte)(src >> 8);
            dst[index + 1] = (byte)(src >> 0x10);
            dst[index + 0] = (byte)(src >> 0x18);
            return 4;
        }

        public static int ByteArrayToInt(byte [] src, int index)
        {
            int ret = 0;
            ret |= ((int)src[index + 3]);
            ret |= ((int)src[index + 2] << 8);
            ret |= ((int)src[index + 1] << 16);
            ret |= ((int)src[index + 0] << 24);
            return ret;
        }
    }
}
