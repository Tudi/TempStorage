// Example how to read a SAF file. Used for visualizing SAf data
{
    SAFFileHandler.ISAFFile myFile = new SAFFileHandler.SAFFile();
    myFile.ReadFile("f1.saf");
}

// example to create a SAF file and draw 1 line in it
{
    SAFFileHandler.ISAFFile myFile = new SAFFileHandler.SAFFile();
    myFile.AddNewLine(-1, -1);
    myFile.AppendToLine(-2, -2);
    myFile.WriteFile("out.saf");
}

// example SAF file to show names on the LCD screeen
{
    SAFFileHandler.ISAFFile myFile = new SAFFileHandler.SAFFile();
    myFile.SetDisplayName("MySAF"); // limited to 8 characters
    myFile.SetDisplayDescription("SAF File Name"); // limited to 64 characters
    myFile.AddNewLine(-1, -1);
    myFile.AppendToLine(-2, -2);
    myFile.WriteFile("out.saf");
}

// example to create a SAF file and put 1 SIG into a single transition segment
{
    SAFFileHandler.ISAFFile myFile = new SAFFileHandler.SAFFile();
    myFile.AppendSigFile(GetSIGFileContent("1.sig"));
    myFile.WriteFile("out2.saf");
}

// example to create a SAF file and put 2 SIG into separate transitions segments
{
    SAFFileHandler.ISAFFile myFile = new SAFFileHandler.SAFFile();
    myFile.AppendSigFile(GetSIGFileContent("1.sig"));
    myFile.AppendTransition();
    myFile.AppendSigFile(GetSIGFileContent("2.sig"));
    myFile.WriteFile("out3.saf");
}


// helper func
string[] GetSIGFileContent(string SIGFileName)
{
    string[] fileLines = new string[1];
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