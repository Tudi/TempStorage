using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CradleWindowsAgent.Tools
{
    class LogWriter
    {
        public static void WriteLog(string logMessage)
        {
            Console.WriteLine(logMessage); 

            try
            {
                var path = Path.Combine(Global.AppDataDirLogsPath, "CMALogs.txt");

                using (StreamWriter w = File.AppendText(path))
                {
                    Log(logMessage, w);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

        }

        private static void Log(string logMessage, TextWriter txtWriter)
        {
            try
            {
                txtWriter.WriteLine("{0} - {1}", DateTime.Now.ToString(), logMessage);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
