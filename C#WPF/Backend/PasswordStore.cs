using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    //this implementation was not made by me. Epic ...
    public class PasswordStore
    {
        string AdminPassw = "HIPATH";

        public bool PasswMatches(string UserInput)
        {
            if (AdminPswGet() == UserInput)
                return true;
            return false;
        }

        public string AdminPswGet()
        {
            //check if we have a passw file
            string PasswFile = Globals.GetFullAppPath("Settings\\passfile");
            if(File.Exists(PasswFile))
            {
                //get the content of the file
                byte [] fileBytes = File.ReadAllBytes(PasswFile);
                //decode
                AdminPassw = Decode(fileBytes);
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Read " + fileBytes.Length.ToString() + " bytes, decoded to " + AdminPassw);
            }
            else
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Could not open password file at "+ PasswFile + " . Using default password");
            return AdminPassw;
        }

        public void AdminPswSet(string NewPassw)
        {
            AdminPassw = NewPassw;
            string PasswFile = Globals.GetFullAppPath("Settings\\passfile");
            byte[] codedPassw = Encode(NewPassw);
            File.WriteAllBytes(PasswFile, codedPassw);
        }

        const string EncoderString = "3.1415926";
        private byte[] Encode(string what)
        {
            byte []EncodedString = new byte[what.Length+1];
            EncodedString[0] = (byte)what.Length;
            for (int i= 0;i< what.Length;i++)
            {
                byte chari = (byte)what[i];
                chari += (byte)EncoderString[i % 9];
                EncodedString[i+1] = chari;
            }
            return EncodedString;
        }

        private string Decode(byte [] what)
        {
            string DecodedString = "";
            byte CharCount = what[0];
            for (int i = 0; i < CharCount; i++)
            {
                byte chari = (byte)what[i+1];
                chari -= (byte)EncoderString[i % 9];
                DecodedString += (char)chari;
            }
            return DecodedString;
        }
    }
}
