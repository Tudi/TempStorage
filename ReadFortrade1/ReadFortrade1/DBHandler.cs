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
            m_dbConnection = new SQLiteConnection("Data Source=Fortrade.db;New=False;Version=3;journal_mode=WAL;synchronous=NORMAL");
            m_dbConnection.Open();
            SQLiteCommand command;
            object name;
            {
                command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA cache_size=100000";
                command.ExecuteScalar();
            }
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
        }

        public static long GetUnixStamp()
        {
            return (long)(DateTime.Now - new DateTime(1970, 1, 1, 0, 0, 0)).TotalSeconds;
        }

        private string CreateTableForInstrument(string Instrument)
        {
            string TableName = "T"+Instrument.GetHashCode().ToString();
            TableName = TableName.Replace('-', '_');
            var cmd2 = new SQLiteCommand(m_dbConnection);
            cmd2.CommandText = "INSERT into TableNames(InstrumentName,TableName)values(@InstrumentName,@TableName)";
            cmd2.Parameters.AddWithValue("@InstrumentName", Instrument);
            cmd2.Parameters.AddWithValue("@TableName", TableName);
            cmd2.Prepare();
            cmd2.ExecuteNonQuery();

            string sql = "create table " + TableName + " (Stamp int,Sell FLOAT,Buy FLOAT,SellSentiment FLOAT)";
            var command = new SQLiteCommand(sql, m_dbConnection);
            command.ExecuteNonQuery();

            return TableName;
        }

        private string GetTableNameForInstrument(string Instrument)
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
                ReturnValue = CreateTableForInstrument(Instrument);
            return ReturnValue;
        }

        public void AddInstrumentValue(string Instrument, double pSellPrice, double pBuyPrice, double pSellSentiment)
        {
            string TableName = GetTableNameForInstrument(Instrument);
            long Stamp = GetUnixStamp();
            var cmd2 = new SQLiteCommand(m_dbConnection);
            cmd2.CommandText = "INSERT into @TableName (Stamp,Sell,Buy,SellSentiment)values(@Stamp,@Sell,@Buy,@SellSentiment)";
            cmd2.CommandText = "INSERT into "+ TableName + " (Stamp,Sell,Buy,SellSentiment)values("+ Stamp.ToString() + ","+ pSellPrice.ToString() + ","+ pBuyPrice.ToString() + ","+ pSellSentiment.ToString() + ")";
            cmd2.Parameters.AddWithValue("@TableName", TableName);
            cmd2.Parameters.AddWithValue("@Stamp", Stamp);
            cmd2.Parameters.AddWithValue("@Sell", pSellPrice);
            cmd2.Parameters.AddWithValue("@Buy", pBuyPrice);
            cmd2.Parameters.AddWithValue("@SellSentiment", pSellSentiment);
            cmd2.Prepare();
            cmd2.ExecuteNonQuery();
        }
    }
}
