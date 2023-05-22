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
            // example to create a BIN file and draw 1 line in it
            {
                SigToBin binFileWriter = new SigToBin();
                binFileWriter.SetBinFileName("LineTest.BIN");
                bool errorsEncountered = binFileWriter.AddLine(-1, -1, 1, 1);
                binFileWriter.CloseBinFile();
                if(errorsEncountered == true)
                {
                    Console.WriteLine("There have been errors while drawing");
                }
            }
            // example to convert a SIG file to BIN file
            {
                SigToBin binFileWriter = new SigToBin();

//              binFileWriter.ConvertSigFileToBin("../../../BinFiles/S001 Vertical Half Inch Line.Sig");
                bool errorsEncountered = binFileWriter.ConvertSigFileToBin("S005 Five One Inch Squares.Sig");
                if (errorsEncountered == true)
                {
                    Console.WriteLine("There have been errors while drawing");
                }
            }
        }
    }
}
