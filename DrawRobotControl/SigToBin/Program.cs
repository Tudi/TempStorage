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
                binFileWriter.SetDrawSpeed(50); // set draw speed
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
                string[] fileContent = GetSIGFileContent("S005 Five One Inch Squares.Sig");
                bool errorsEncountered = binFileWriter.AddSIGToBin(fileContent);
                if (errorsEncountered == true)
                {
                    Console.WriteLine("There have been errors while drawing");
                }
                binFileWriter.CloseBinFile();
            }
            // example to add 2 sigs to the same BIN
            {
                SigToBin binFileWriter = new SigToBin();
                string[] fileContent = GetSIGFileContent("TearDrop_05_22.sig");
                bool errorsEncountered = binFileWriter.AddSIGToBin(fileContent);
                if (errorsEncountered == true)
                {
                    Console.WriteLine("There have been errors while drawing");
                }
                fileContent = GetSIGFileContent("Paragraph.Sig");
                errorsEncountered = binFileWriter.AddSIGToBin(fileContent);
                if (errorsEncountered == true)
                {
                    Console.WriteLine("There have been errors while drawing");
                }
                binFileWriter.CloseBinFile();
            }
        }
        /// <summary>
        /// In case you want some quick tests, you might want to convert a single file to BIN
        /// </summary>
        /// <param name="SIGFileName"></param>
        /// <returns></returns>
        public static string[] GetSIGFileContent(string SIGFileName)
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
    }
}
