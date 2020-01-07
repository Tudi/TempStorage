using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;
using System.Transactions;
using System.Globalization;

namespace ReadFortrade1
{
    public class DBHandler
    {
        public SQLiteConnection m_dbConnection = null;

        public DBHandler()
        {
            m_dbConnection = new SQLiteConnection("Data Source=Fortrade.db;New=False;Version=3");
            m_dbConnection.Open();
            SQLiteCommand command;
            object name;
            {
                command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA synchronous=OFF";
                command.ExecuteScalar();
            }
            {
                command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA journal_mode=MEMORY";
                command.ExecuteScalar();
            }
            {
                command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA temp_store=MEMORY";
                command.ExecuteScalar();
            }

            command = m_dbConnection.CreateCommand();
            command.CommandText = "SELECT name FROM sqlite_master WHERE name='TableNames'";
            name = command.ExecuteScalar();

            if (!(name != null && name.ToString() == "TableNames"))
            {
                string sql = "create table TableNames (InstrumentName varchar(200),TableName varchar(200))";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            AddIndexToTables();
        }

        public static long GetUnixStamp()
        {
            return (long)(DateTime.Now - new DateTime(1970, 1, 1, 0, 0, 0)).TotalSeconds;
        }

        public static long GetUnixStamp(DateTime time)
        {
            return (long)(time - new DateTime(1970, 1, 1, 0, 0, 0)).TotalSeconds;
        }

        public static long GetUnixStampStartDay(long SubstractMonths = 0, long SubstractWeeks = 0, long SubstractDays = 0, long SubstractHours = 0, long SubstractMinutes = 0, long SubstractSeconds = 0)
        {
            DateTime Now = DateTime.Now;
            DateTime StartNow = new DateTime(Now.Year, Now.Month, Now.Day, 0, 0, 0);
            TimeSpan Diff = (StartNow - new DateTime(1970, 1, 1, 0, 0, 0));
            long ret = (long)Diff.TotalSeconds;
            ret += SubstractSeconds;
            ret += SubstractMinutes * 60;
            ret += SubstractHours * 60 * 60;
            ret += SubstractDays * 24 * 60 * 60;
            ret += SubstractWeeks * 7 * 24 * 60 * 60;
            ret += SubstractMonths * 4 * 7 * 24 * 60 * 60;
            return ret;
        }

        private void AddIndexToTables()
        {
            using (var cmd = new SQLiteCommand(m_dbConnection))
            {
                cmd.CommandText = "SELECT TableName FROM TableNames";
                cmd.Prepare();
                SQLiteDataReader rdr = cmd.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true && !rdr.IsDBNull(0))
                {
                    string TableName = rdr.GetString(0);
                    //check if the new DB contains this table, and has values
                    using (var cmd2 = new SQLiteCommand(m_dbConnection))
                    {
                        cmd2.CommandText = "CREATE INDEX IF NOT EXISTS "+ TableName + "_ind ON "+ TableName+"_price (stamp)";
                        cmd2.ExecuteNonQuery();
                    }
                }
            }
        }
        private string CreateTablesForInstrument(string Instrument)
        {
//            string TableName = "T"+Instrument.GetHashCode().ToString();
//            TableName = TableName.Replace('-', '_');
            string TableName = Instrument;
            TableName = TableName.Replace("-", "_");
            TableName = TableName.Replace("(", "_");
            TableName = TableName.Replace(")", "_");
            TableName = TableName.Replace(" ", "_");
            TableName = TableName.Replace("/", "_");
            TableName = TableName.Replace(".", "_");
            TableName = TableName.Replace(",", "_");

            var cmd2 = new SQLiteCommand(m_dbConnection);
            cmd2.CommandText = "INSERT into TableNames(InstrumentName,TableName)values(@InstrumentName,@TableName)";
            cmd2.Parameters.AddWithValue("@InstrumentName", Instrument);
            cmd2.Parameters.AddWithValue("@TableName", TableName);
            cmd2.Prepare();
            cmd2.ExecuteNonQuery();

            cmd2 = new SQLiteCommand("create table " + TableName + "_price (Stamp int,Sell float)", m_dbConnection);
            cmd2.ExecuteNonQuery();
            cmd2 = new SQLiteCommand("create table " + TableName + "_spread (Stamp int,Spread float)", m_dbConnection);
            cmd2.ExecuteNonQuery();
            cmd2 = new SQLiteCommand("create table " + TableName + "_sentiment (Stamp int,Sentiment float)", m_dbConnection);
            cmd2.ExecuteNonQuery();

            using (var cmd3 = new SQLiteCommand(m_dbConnection))
            {
                cmd3.CommandText = "CREATE INDEX IF NOT EXISTS " + TableName + "_ind ON " + TableName + "_price (stamp)";
                cmd3.ExecuteNonQuery();
            }

            return TableName;
        }

        public string GetTableNameForInstrument(string Instrument)
        {
            string ReturnValue = "";
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT TableName FROM TableNames where InstrumentName=@InstrumentName";
            cmd.Parameters.AddWithValue("@InstrumentName", Instrument);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true && !rdr.IsDBNull(0))
                ReturnValue = rdr.GetString(0);
            else
                ReturnValue = CreateTablesForInstrument(Instrument);
            return ReturnValue;
        }

        public void AddSpreadValue(string Instrument, double pSpread, string pTableName)
        {
            string TableName = pTableName;
            if(TableName == null)
                TableName = GetTableNameForInstrument(Instrument);
            long Stamp = GetUnixStamp();
            var cmd2 = new SQLiteCommand(m_dbConnection);
            cmd2.CommandText = "INSERT into "+ TableName + "_spread (Stamp,Spread)values(" + Stamp.ToString() + ","+ pSpread.ToString() + ")";
            cmd2.Prepare();
            cmd2.ExecuteNonQuery();
        }

        public void AddInstrumentValue(string Instrument, double pSellPrice, string pTableName)
        {
            string TableName = pTableName;
            if (TableName == null)
                TableName = GetTableNameForInstrument(Instrument);
            long Stamp = GetUnixStamp();
            var cmd2 = new SQLiteCommand(m_dbConnection);
            cmd2.CommandText = "INSERT into " + TableName + "_price (Stamp,Sell)values(" + Stamp.ToString() + "," + pSellPrice.ToString() + ")";
            cmd2.Prepare();
            cmd2.ExecuteNonQuery();
        }

        public void AddSentimentValue(string Instrument, double pSellSentiment, string pTableName)
        {
            string TableName = pTableName;
            if (TableName == null)
                TableName = GetTableNameForInstrument(Instrument);
            long Stamp = GetUnixStamp();
            var cmd2 = new SQLiteCommand(m_dbConnection);
            cmd2.CommandText = "INSERT into " + TableName + "_sentiment (Stamp,Sentiment)values(" + Stamp.ToString() + "," + pSellSentiment.ToString() + ")";
            cmd2.Prepare();
            cmd2.ExecuteNonQuery();
        }

        public void LoadLastSamplesFromAllTables()
        {
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT TableName,InstrumentName FROM TableNames";
            cmd.Prepare();
            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true && !rdr.IsDBNull(0))
            {
                string TableName = rdr.GetString(0);
                string Instrument = rdr.GetString(1);
                double sell = 0, spread = 0, buy, sentiment = 0;
                using (var cmd2 = new SQLiteCommand(m_dbConnection))
                {
                    cmd2.CommandText = "SELECT Sell FROM " + TableName + "_price order by stamp desc limit 0,1";
                    SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                    if (rdr2.Read() && rdr2.HasRows == true)
                        sell = rdr2.GetFloat(0);
                }
                using (var cmd2 = new SQLiteCommand(m_dbConnection))
                {
                    cmd2.CommandText = "SELECT Spread FROM " + TableName + "_spread order by stamp desc limit 0,1";
                    SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                    if (rdr2.Read() && rdr2.HasRows == true)
                        spread = rdr2.GetFloat(0);
                }
                using (var cmd2 = new SQLiteCommand(m_dbConnection))
                {
                    cmd2.CommandText = "SELECT Sentiment FROM " + TableName + "_sentiment order by stamp desc limit 0,1";
                    SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                    if (rdr2.Read() && rdr2.HasRows == true)
                        sentiment = rdr2.GetFloat(0);
                }
                buy = sell + spread;
                Globals.vHistory.AddRecord(Instrument, sell, buy, sentiment, 100 - sentiment);
            }
        }

        public List<InstrumentValuePair> GetInstrumentValues(string Instrument, long StartStamp, long EndStamp)
        {
            string TableName = GetTableNameForInstrument(Instrument);
            List<InstrumentValuePair> ret = new List<InstrumentValuePair>();
            using (var cmd2 = new SQLiteCommand(m_dbConnection))
            {
                cmd2.CommandText = "SELECT stamp,Sell FROM " + TableName + "_price where stamp>="+ StartStamp.ToString() + " and stamp<="+ EndStamp.ToString();
                SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                while (rdr2.Read() && rdr2.HasRows == true)
                {
                    long stamp = rdr2.GetInt64(0);
                    double sell = rdr2.GetFloat(1);
                    ret.Add(new InstrumentValuePair(stamp, sell));
                }
            }
            return ret;
        }

        public double GetInstrumentSpread(string Instrument, long StartStamp, long EndStamp)
        {
            string TableName = GetTableNameForInstrument(Instrument);
            List<InstrumentValuePair> ret = new List<InstrumentValuePair>();
            using (var cmd2 = new SQLiteCommand(m_dbConnection))
            {
                cmd2.CommandText = "SELECT avg(Spread) FROM " + TableName + "_spread where stamp>=" + StartStamp.ToString() + " and stamp<=" + EndStamp.ToString();
                SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                if (rdr2.Read() && rdr2.HasRows == true && !rdr2.IsDBNull(0))
                    return rdr2.GetFloat(0);
            }
            return 0;
        }

        public double GetInstrumentAveragePricePeriod(string Instrument, long StartStamp, long EndStamp)
        {
            string TableName = GetTableNameForInstrument(Instrument);
            List<InstrumentValuePair> ret = new List<InstrumentValuePair>();
            using (var cmd2 = new SQLiteCommand(m_dbConnection))
            {
                cmd2.CommandText = "SELECT avg(sell) FROM " + TableName + "_price where stamp>=" + StartStamp.ToString() + " and stamp<=" + EndStamp.ToString();
                SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                if (rdr2.Read() && rdr2.HasRows == true && !rdr2.IsDBNull(0))
                    return rdr2.GetFloat(0);
            }
            return 0;
        }
        public List<string> GetAllInstrumentNames()
        {
            List<string> ret = new List<string>();
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT InstrumentName FROM TableNames";
            cmd.Prepare();
            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true && !rdr.IsDBNull(0))
                ret.Add(rdr.GetString(0));
            return ret;
        }

        public void ImportFromDB(string FileName)
        {
            SQLiteConnection m_dbConnection2 = null;
            using (m_dbConnection2 = new SQLiteConnection("Data Source=" + FileName + ";New=False;Version=3"))
            {
                m_dbConnection2.Open();
                //get a list of tables we can import from the external DB
                using (var cmd = new SQLiteCommand(m_dbConnection))
                {
                    cmd.CommandText = "SELECT TableName FROM TableNames";
                    cmd.Prepare();
                    SQLiteDataReader rdr = cmd.ExecuteReader();
                    while (rdr.Read() && rdr.HasRows == true && !rdr.IsDBNull(0))
                    {
                        string TableName = rdr.GetString(0);
                        //check if the new DB contains this table, and has values
                        using (var cmd2 = new SQLiteCommand(m_dbConnection2))
                        {
                            cmd2.CommandText = "SELECT stamp,Sell FROM " + TableName + "_price";
                            SQLiteDataReader rdr2 = cmd2.ExecuteReader();
                            while (rdr2.Read() && rdr2.HasRows == true && !rdr2.IsDBNull(0))
                            {
                                long stamp = rdr2.GetInt64(0);
                                double sell = rdr2.GetFloat(1);
                                //does the current db already have this value
                                bool AlreadyExists = false;
                                using (var cmd3 = new SQLiteCommand(m_dbConnection))
                                {
                                    //                            cmd3.CommandText = "SELECT stamp,Sell FROM " + TableName + "_price where stamp=" + stamp.ToString() + " and sell=" + sell.ToString();
                                    cmd3.CommandText = "SELECT stamp FROM " + TableName + "_price where stamp=" + stamp.ToString() + " limit 1";
                                    SQLiteDataReader rdr3 = cmd3.ExecuteReader();
                                    while (rdr3.Read() && rdr3.HasRows == true && !rdr3.IsDBNull(0))
                                        AlreadyExists = true;
                                }
                                //if it is a new value, insert it
                                if (AlreadyExists == false)
                                {
                                    using (var cmd3 = new SQLiteCommand(m_dbConnection))
                                    {
                                        cmd3.CommandText = "INSERT into " + TableName + "_price (Stamp,Sell)values(" + stamp.ToString() + "," + sell.ToString() + ")";
                                        cmd3.Prepare();
                                        cmd3.ExecuteNonQuery();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
