using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SigToBin
{
    internal class Program
    {
        static void Main(string[] args)
        {
            SigToBin binFileWriter = new SigToBin();

//            binFileWriter.ConvertSigFileToBin("../../../BinFiles/S001 Vertical Half Inch Line.Sig");
            binFileWriter.ConvertSigFileToBin("../../../BinFiles/S005 Five One Inch Squares.Sig");
        }
    }
}
