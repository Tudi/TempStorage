using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;
using System.Transactions;

namespace CSVIngester
{
    public static class DateParser
    {
        public static long DateTimeToMinutes(DateTime dt)
        {
            return (long)(dt - new DateTime(1970, 1, 1, 0, 0, 0)).TotalMinutes;
        }
        public static long GetUnixStamp(string DateStr)
        {
            long ret = DateTimeToMinutes(new DateTime(2001, 1, 2));
            long retInitial = ret;
            if (DateStr.Length <= 8)
            {
                GlobalVariables.Logger.Log("Date string is too short to know format : " + DateStr);
                return ret;
            }
            //this is because parse date is super slow and fails a lot. Our format is pretty fixed in most cases
            if (DateStr.Length == 10)
            {
                string[] parts = DateStr.Split('/');
                if (parts.Length == 3)
                {
                    //maybe day/month/year
                    int day;
                    int month;
                    int year;
                    {
                        int.TryParse(parts[0], out day);
                        int.TryParse(parts[1], out month);
                        int.TryParse(parts[2], out year);
//                        day = int.Parse(parts[0]);
//                        month = int.Parse(parts[1]);
//                        year = int.Parse(parts[2]);
//                      long refret = (long)(DateTime.Now - new DateTime(1970, 1, 1, 0, 0, 0)).TotalMinutes;
                        if (day <= 31 && month <= 12 && year >= 1970 && year < 5000)
                            ret = DateTimeToMinutes(new DateTime(year, month, day));
                        //swap day and year
                        else if (year <= 31 && month <= 12 && day >= 1970 && day < 5000)
                            ret = DateTimeToMinutes(new DateTime(day, month, year));
                    }
                }
            }
            if (ret == retInitial)
            {
                try
                {
                    ret = (long)(DateTime.Parse(DateStr) - new DateTime(1970, 1, 1, 0, 0, 0)).TotalMinutes;
                }
                catch
                {
                }
            }
            if(ret == retInitial)
                GlobalVariables.Logger.Log("Date string unknown format : " + DateStr);

            return ret;
        }
    }
    public class DBHandler
    {
        public SQLiteConnection m_dbConnection = null;
        public DBHandler()
        {
//            SQLiteConnection.CreateFile("EbayCSV.db");
            m_dbConnection = new SQLiteConnection("Data Source=EbayCSV.db;New=False;Version=3;journal_mode=WAL;synchronous=NORMAL");
            m_dbConnection.Open();

            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA cache_size=100000";
                var name = command.ExecuteScalar();
            }
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA synchronous=OFF";
                var name = command.ExecuteScalar();
            }
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA journal_mode=MEMORY";
                var name = command.ExecuteScalar();
            }
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA temp_store=MEMORY";
                var name = command.ExecuteScalar();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='InventoryCSV'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "InventoryCSV"))
                {

                    string sql = "create table InventoryCSV (ebay_id_Hash INT4,asin_hash INT4,vat FLOAT DEFAULT "+GlobalVariables.NULLValue+", ebay_id_str varchar(20),asin_str varchar(20))";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueItemId ON InventoryCSV (ebay_id_Hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON InventoryCSV (asin_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='AMAZON_ORDERS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "AMAZON_ORDERS"))
                {

                    string sql = "create table AMAZON_ORDERS (date varchar(20),ORDER_ID varchar(20),TITLE varchar(200),GROSS float,vat float,BuyerName varchar(200),Buyeraddr varchar(200), asin varchar(20), NET float, vat_rate float, asin_hash int4, ORDER_ID_hash int4,TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON AMAZON_ORDERS (ORDER_ID_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON AMAZON_ORDERS (asin_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='AMAZON_REFUNDS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "AMAZON_REFUNDS"))
                {

                    string sql = "create table AMAZON_REFUNDS (date varchar(20),ORDER_ID varchar(20),TITLE varchar(200),GROSS float,vat float,BuyerName varchar(200),Buyeraddr varchar(200), asin varchar(20), NET float, vat_rate float, asin_hash int4, ORDER_ID_hash int4,TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON AMAZON_REFUNDS (ORDER_ID_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON AMAZON_REFUNDS (asin_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='PAYPAL_SALES'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "PAYPAL_SALES"))
                {

                    string sql = "create table PAYPAL_SALES (date varchar(20),BuyerName varchar(20),GROSS float,PayPalFee float,TransactionID varchar(200),Title varchar(200), ItemId varchar(200), BuyerAddr varchar(200), Phone varchar(200), vat float, net float, vat_rate float, TransactionID_hash int4, TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON PAYPAL_SALES (TransactionID_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='PAYPAL_REFUNDS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "PAYPAL_REFUNDS"))
                {

                    string sql = "create table PAYPAL_REFUNDS (date varchar(20),BuyerName varchar(20),GROSS float,PayPalFee float,TransactionID varchar(200),Title varchar(200), ItemId varchar(200), ReferenceID varchar(200), vat float, net float, vat_rate float, TransactionID_hash int4, ReferenceID_hash int4, TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON PAYPAL_REFUNDS (TransactionID_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }
        }

        private void ReplaceInventoryRow(string ebay_id, string asin, string vat)
        {
            int ebayhash = ebay_id.GetHashCode();
            int asinhash = asin.GetHashCode();

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO InventoryCSV(ebay_id_Hash,asin_hash,vat,ebay_id_str,asin_str) VALUES(@ebay_id_Hash, @asin_hash, @vat,@ebay_id_str,@asin_str)";

            cmd.Parameters.AddWithValue("@ebay_id_Hash", ebayhash);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@vat", float.Parse(vat));
            cmd.Parameters.AddWithValue("@ebay_id_str", ebay_id); 
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Prepare();

            cmd.ExecuteNonQuery(); 
        }
        public enum InvenotryInsertResultCodes
        {
            RowExistedButWasEmpty = 1,
            RowExisted = 2,
            RowDidNotExistInsertedNew = 3,
            RowInvalidValues = 4,
        }
        public InvenotryInsertResultCodes InsertInventory(string ebay_id, string asin, string vat)
        {
            InvenotryInsertResultCodes ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            int ebayhash = ebay_id.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ebay_id_Hash,asin_str,vat FROM InventoryCSV where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
            cmd.Parameters.AddWithValue("@ebayhash", ebayhash);
            cmd.Parameters.AddWithValue("@ebay_id", ebay_id);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if(rdr.Read() && rdr.HasRows == true)
            {
                //check if the existing row has values
                string asin_str = "";
                if (!rdr.IsDBNull(1))
                    asin_str = rdr.GetString(1);
                float oldVAT = rdr.GetFloat(2);
                if (asin_str.Length == 0)
                {
                    //value exists, but it still needs import
                    ReplaceInventoryRow(ebay_id, asin, oldVAT.ToString());
                    ReturnCode = InvenotryInsertResultCodes.RowExistedButWasEmpty;
                }
                else
                {
                    ReturnCode = InvenotryInsertResultCodes.RowExisted;
                }
            }
            else
            {
                ReplaceInventoryRow(ebay_id, asin, vat);
                ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;/**/
            }
            
            return ReturnCode;
        }

        private void ClearTable(string Table)
        {
            if (m_dbConnection == null)
                return;
            if (Table == null || Table.Length == 0)
                return;
            string sql = "delete from " + Table;
            SQLiteCommand command = new SQLiteCommand(sql, m_dbConnection);
            command.ExecuteNonQuery();
        }

        public void ClearInventory()
        {
            GlobalVariables.Logger.Log("Content of the Inventory table has been cleared");
            ClearTable("InventoryCSV");
        }

        ~DBHandler()
        {
            if(m_dbConnection != null)
                m_dbConnection = null;
        }

        public void UpdateInventoryVatEbay(string ebay_id, string vat)
        {
            if (ebay_id == null || ebay_id.Length == 0 || vat == null || vat.Length == 0)
                return;

            int ebayhash = ebay_id.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT 1 FROM InventoryCSV where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
            cmd.Parameters.AddWithValue("@ebayhash", ebayhash);
            cmd.Parameters.AddWithValue("@ebay_id", ebay_id);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "UPDATE InventoryCSV set vat=@vat where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
                cmd2.Parameters.AddWithValue("@ebayhash", ebayhash);
                cmd2.Parameters.AddWithValue("@vat", float.Parse(vat));
                cmd2.Parameters.AddWithValue("@ebay_id", ebay_id);
                cmd2.Prepare();
                cmd2.ExecuteNonQuery();
            }
            else
            {
                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "INSERT into InventoryCSV(ebay_id_Hash,vat,ebay_id_str)values(@ebayhash,@vat,@ebay_id)";
                cmd2.Parameters.AddWithValue("@ebayhash", ebayhash);
                cmd2.Parameters.AddWithValue("@vat", float.Parse(vat));
                cmd2.Parameters.AddWithValue("@ebay_id", ebay_id);
                cmd2.Prepare();
                cmd2.ExecuteNonQuery();
            }
        }

        public void UpdateInventoryVatAsin(string asin, string vat)
        {
            if (asin == null || asin.Length == 0 || vat == null || vat.Length == 0)
                return;

            int asinHash = asin.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "update InventoryCSV set vat=@vat where asin_hash=@asin_hash and asin_str=@asin_str";
            cmd.Parameters.AddWithValue("@vat", float.Parse(vat));
            cmd.Parameters.AddWithValue("@asin_hash", asinHash);
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Prepare();
            cmd.ExecuteNonQuery();
        }

        public void ExportInventoryTable()
        {
            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateInventoryRunFile("./reports/INVENTORY.csv");
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ebay_id_str,asin_str,vat FROM InventoryCSV";
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                string ebay_id_str = "";
                if (!rdr.IsDBNull(0))
                    ebay_id_str = rdr.GetString(0);
                string asin_str = "";
                if (!rdr.IsDBNull(1))
                    asin_str = rdr.GetString(1);
                float vat = rdr.GetFloat(2);
                ExportInventoryCSV.InventoryRunFileAddRow(ebay_id_str, asin_str, vat);
            }
            ExportInventoryCSV.Dispose();
        }

        private void InsertAmazonRow(string TableName, string DateCol, string IdCol, string TitleCol, string PriceCol, string VATCol, string BuyerCol, string AddressCol, string ASINCol, float NET, float VAT_RATE)
        {
            int IdColhash = IdCol.GetHashCode();
            int asinhash = ASINCol.GetHashCode();
            long TStamp = DateParser.GetUnixStamp(DateCol);

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO "+ TableName + "(date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate,asin_hash,ORDER_ID_hash,TStamp) " +
                "VALUES(@date,@ORDER_ID,@TITLE,@GROSS,@vat,@BuyerName,@Buyeraddr,@asin,@NET,@vat_rate,@asin_hash,@ORDER_ID_hash,@TStamp)";

            cmd.Parameters.AddWithValue("@date", DateCol);
            cmd.Parameters.AddWithValue("@ORDER_ID", IdCol);
            cmd.Parameters.AddWithValue("@TITLE", TitleCol);
            cmd.Parameters.AddWithValue("@GROSS", PriceCol);
            cmd.Parameters.AddWithValue("@vat", float.Parse(VATCol));
            cmd.Parameters.AddWithValue("@BuyerName", BuyerCol);
            cmd.Parameters.AddWithValue("@Buyeraddr", AddressCol);
            cmd.Parameters.AddWithValue("@asin", ASINCol);
            cmd.Parameters.AddWithValue("@NET", NET);
            cmd.Parameters.AddWithValue("@vat_rate", VAT_RATE);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", IdColhash);
            cmd.Parameters.AddWithValue("@TStamp", TStamp);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }

        public void UpdateAllMissingInventoryRows(string ASINCol, string VATCol)
        {
            int asinhash = ASINCol.GetHashCode();
            float VAT = float.Parse(VATCol);

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "update InventoryCSV set vat=@VAT where asin_hash=@asin_hash and vat="+ GlobalVariables.NULLValue+" and asin_str=@asin";

            cmd.Parameters.AddWithValue("@VAT", VAT);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@asin", ASINCol);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }
        public InvenotryInsertResultCodes InsertAmazonOrder(string TableName, string DateCol, string IdCol, string TitleCol, string PriceCol, string VATCol, string BuyerCol, string AddressCol, string ASINCol, float NET, float VAT_RATE)
        {
            InvenotryInsertResultCodes ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            int ORDER_ID_hashhash = IdCol.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT 1 FROM "+ TableName + " where ORDER_ID_hash=@ORDER_ID_hash and order_id=@order_id";
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", ORDER_ID_hashhash);
            cmd.Parameters.AddWithValue("@order_id", IdCol);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InvenotryInsertResultCodes.RowExisted;
            }
            else
            {
                InsertAmazonRow(TableName, DateCol, IdCol, TitleCol, PriceCol, VATCol, BuyerCol, AddressCol, ASINCol, NET, VAT_RATE);
                ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            }

            return ReturnCode;
        }
        public void ClearAmazonOrders()
        {
            ClearTable("AMAZON_ORDERS");
            GlobalVariables.Logger.Log("Content of the AMAZON-ORDERS table has been cleared");
        }
        public void ClearAmazonRefunds()
        {
            ClearTable("AMAZON_REFUNDS");
            GlobalVariables.Logger.Log("Content of the AMAZON-REFUNDS table has been cleared");
        }
        public void ExportAmazonOrdersTable(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateAmazonOrdersFile("./reports/" + CSVFileName + ".csv");
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate FROM "+ TableName + " where TStamp >= @TStart and TStamp <= @TEnd";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                string datecol = "";
                if (!rdr.IsDBNull(0))
                    datecol = rdr.GetString(0);
                string ORDER_ID = "";
                if (!rdr.IsDBNull(1))
                    ORDER_ID = rdr.GetString(1);
                string TITLE = "";
                if (!rdr.IsDBNull(2))
                    TITLE = rdr.GetString(2);
                float GROSS = rdr.GetFloat(3);
                float vat = rdr.GetFloat(4);
                string BuyerName = "";
                if (!rdr.IsDBNull(5))
                    BuyerName = rdr.GetString(5);
                string Buyeraddr = "";
                if (!rdr.IsDBNull(6))
                    Buyeraddr = rdr.GetString(6);
                string asin = "";
                if (!rdr.IsDBNull(7))
                    asin = rdr.GetString(7);
                float NET = rdr.GetFloat(8);
                float vat_rate = rdr.GetFloat(9);
                ExportInventoryCSV.AmazonOrdersExportFileAddRow(datecol, ORDER_ID, TITLE, GROSS, vat, BuyerName, Buyeraddr, asin, NET, vat_rate);
            }
            ExportInventoryCSV.Dispose();
        }
        public void ClearPaypalSales()
        {
            ClearTable("PAYPAL_SALES");
            GlobalVariables.Logger.Log("Content of the PAYPAL-SALES table has been cleared");
        }
        public void ClearPaypalRefunds()
        {
            ClearTable("PAYPAL_REFUNDS");
            GlobalVariables.Logger.Log("Content of the PAYPAL-REFUNDS table has been cleared");
        }

        public InvenotryInsertResultCodes InsertPaypalRow(string DateCol, string NameCol, string PriceCol, string PPFeeCol, string TransactionIDCol, string TitleCol, string ItemIdCol, string AddressCol, string PhoneCol)
        {
            InvenotryInsertResultCodes ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            int IdColhash = TransactionIDCol.GetHashCode();

            string TableName = "PAYPAL_SALES";

            //check if ts record already exists
            var cmd1 = new SQLiteCommand(m_dbConnection);
            cmd1.CommandText = "SELECT 1 FROM " + TableName + " where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
            cmd1.Parameters.AddWithValue("@IdColhash", IdColhash);
            cmd1.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
            cmd1.Prepare();

            SQLiteDataReader rdr = cmd1.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InvenotryInsertResultCodes.RowExisted;
            }
            else
            {
                long TStamp = DateParser.GetUnixStamp(DateCol);
                var cmd = new SQLiteCommand(m_dbConnection);
                cmd.CommandText = "REPLACE INTO " + TableName + "(date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate,TransactionID_hash,TStamp) " +
                    "VALUES(@DateCol,@NameCol,@PriceCol,@PPFeeCol,@TransactionIDCol,@TitleCol,@ItemIdCol,@AddressCol,@PhoneCol,@vat,@net,@vat_rate,@TransactionID_hash,@TStamp)";

                cmd.Parameters.AddWithValue("@DateCol", DateCol);
                cmd.Parameters.AddWithValue("@NameCol", NameCol);
                cmd.Parameters.AddWithValue("@PriceCol", float.Parse(PriceCol));
                cmd.Parameters.AddWithValue("@PPFeeCol", float.Parse(PPFeeCol));
                cmd.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
                cmd.Parameters.AddWithValue("@TitleCol", TitleCol);
                cmd.Parameters.AddWithValue("@ItemIdCol", ItemIdCol);
                cmd.Parameters.AddWithValue("@AddressCol", AddressCol);
                cmd.Parameters.AddWithValue("@PhoneCol", PhoneCol);
                cmd.Parameters.AddWithValue("@vat", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@net", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@vat_rate", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@TransactionID_hash", IdColhash);
                cmd.Parameters.AddWithValue("@TStamp", TStamp);
                cmd.Prepare();

                cmd.ExecuteNonQuery();

                ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            }

            return ReturnCode;
        }

        public InvenotryInsertResultCodes InsertPaypalRefundRow(string DateCol, string NameCol, string PriceCol, string PPFeeCol, string TransactionIDCol, string TitleCol, string ItemIdCol, string ReferenceIDCol)
        {
            InvenotryInsertResultCodes ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            int IdColhash = TransactionIDCol.GetHashCode();

            string TableName = "PAYPAL_REFUNDS";

            //check if ts record already exists
            var cmd1 = new SQLiteCommand(m_dbConnection);
            cmd1.CommandText = "SELECT 1 FROM " + TableName + " where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
            cmd1.Parameters.AddWithValue("@IdColhash", IdColhash);
            cmd1.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
            cmd1.Prepare();

            SQLiteDataReader rdr = cmd1.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InvenotryInsertResultCodes.RowExisted;
            }
            else
            {
                long TStamp = DateParser.GetUnixStamp(DateCol);
                var cmd = new SQLiteCommand(m_dbConnection);
                cmd.CommandText = "REPLACE INTO " + TableName + "(date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,vat,net,vat_rate,TransactionID_hash,TStamp) " +
                    "VALUES(@DateCol,@NameCol,@PriceCol,@PPFeeCol,@TransactionIDCol,@TitleCol,@ItemIdCol,@ReferenceID,@vat,@net,@vat_rate,@TransactionID_hash,@TStamp)";

                cmd.Parameters.AddWithValue("@DateCol", DateCol);
                cmd.Parameters.AddWithValue("@NameCol", NameCol);
                cmd.Parameters.AddWithValue("@PriceCol", float.Parse(PriceCol));
                cmd.Parameters.AddWithValue("@PPFeeCol", float.Parse(PPFeeCol));
                cmd.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
                cmd.Parameters.AddWithValue("@TitleCol", TitleCol);
                cmd.Parameters.AddWithValue("@ItemIdCol", ItemIdCol);
                cmd.Parameters.AddWithValue("@ReferenceID", ReferenceIDCol);
                cmd.Parameters.AddWithValue("@vat", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@net", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@vat_rate", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@TransactionID_hash", IdColhash);
                cmd.Parameters.AddWithValue("@TStamp", TStamp);
                cmd.Prepare();

                cmd.ExecuteNonQuery();

                ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            }

            return ReturnCode;
        }
        public void ExportPaypalSales(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            var cmd = new SQLiteCommand(m_dbConnection);
            if (TableName == "PAYPAL_SALES")
                cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate,tstamp FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd";
            else
                cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,1,vat,net,vat_rate FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            dynamic record = new System.Dynamic.ExpandoObject();
            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {                
                if (!rdr.IsDBNull(0))
                    record.Date = rdr.GetString(0);
                else
                    record.Date = "";
                if (!rdr.IsDBNull(1))
                    record.Name = rdr.GetString(1);
                else
                    record.Date = "";
                float Gross = rdr.GetFloat(2);
                if (Gross != (float)GlobalVariables.NULLValue)
                    record.Gross = Gross.ToString();
                else
                    record.Gross = "";
                float Fee = rdr.GetFloat(3);
                if (Fee != (float)GlobalVariables.NULLValue)
                    record.Paypal_Fee = Fee.ToString();
                else
                    record.Paypal_Fee = "";
                if (!rdr.IsDBNull(4))
                    record.Transaction_ID = rdr.GetString(4);
                else
                    record.Transaction_ID = "";
                if (!rdr.IsDBNull(5))
                    record.Title = rdr.GetString(5);
                else
                    record.Title = "";
                if (!rdr.IsDBNull(6))
                    record.Item_Id = rdr.GetString(6);
                else
                    record.Item_Id = "";
                if (TableName == "PAYPAL_SALES")
                {
                    if (!rdr.IsDBNull(7))
                        record.Address = rdr.GetString(7);
                    else
                        record.Address = "";
                    if (!rdr.IsDBNull(8))
                        record.Phone = rdr.GetString(8);
                    else
                        record.Phone = "";
                }
                else
                {
                    if (!rdr.IsDBNull(7))
                        record.Reference_Id = rdr.GetString(7);
                    else
                        record.Reference_Id = "";
                }
                float vat = rdr.GetFloat(9);
                if (vat != (float)GlobalVariables.NULLValue)
                    record.Vat = vat.ToString();
                else
                    record.Vat = "";
                float NET = rdr.GetFloat(10);
                if (NET != (float)GlobalVariables.NULLValue)
                    record.NET = NET.ToString();
                else
                    record.NET = "";
                float vat_rate = rdr.GetFloat(11);
                if (vat_rate != (float)GlobalVariables.NULLValue)
                    record.vat_rate = vat_rate.ToString();
                else
                    record.vat_rate = "";

//                long unixTimeStamp = rdr.GetInt64(12);
//                System.DateTime dtDateTime = new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc);
//                dtDateTime = dtDateTime.AddMinutes(unixTimeStamp).ToLocalTime();
//                record.tstamp = dtDateTime.ToString();/**/

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }
            ExportInventoryCSV.Dispose();
        }
        public void UpdateVAT()
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }
            GlobalVariables.Logger.Log("Update VAT - started");
            GlobalVariables.ImportingToDBBlock = "UpdateVat";
            WriteCSVFile ExportSalesCSV = new WriteCSVFile();
            ExportSalesCSV.CreateDynamicFile("./reports/no-vat-paypal-sales.csv");
            WriteCSVFile ExportRefundsCSV = new WriteCSVFile();
            ExportRefundsCSV.CreateDynamicFile("./reports/no-vat-paypal-refunds.csv");
            for (int i=0;i<2;i++)
            {
                var cmd = new SQLiteCommand(m_dbConnection);
                if (i==0)
                    cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate FROM PAYPAL_SALES";
                else
                    cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,1,vat,net,vat_rate FROM PAYPAL_REFUNDS";
                cmd.Prepare();

                dynamic record = new System.Dynamic.ExpandoObject();
                SQLiteDataReader rdr = cmd.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true)
                {
                    //for refunds table, we do not process rows that already have a value
                    float vat_rate = rdr.GetFloat(11);
                    if (i == 1 && vat_rate >= 0)
                        continue;

                    //get the itemID
                    string EbayId = "";
                    if (!rdr.IsDBNull(6))
                        EbayId = rdr.GetString(6);

                    //check if inventory has this value
                    int ebayhash = EbayId.GetHashCode();
                    var cmd1 = new SQLiteCommand(m_dbConnection);
                    cmd1.CommandText = "SELECT vat FROM InventoryCSV where ebay_id_Hash=@ebay_id_Hash and ebay_id_str=@ebay_id_str limit 1";
                    cmd1.Parameters.AddWithValue("@ebay_id_Hash", ebayhash);
                    cmd1.Parameters.AddWithValue("@ebay_id_str", EbayId);
                    cmd1.Prepare();
                    SQLiteDataReader rdr2 = cmd1.ExecuteReader();
                    float InventoryVAT = GlobalVariables.NULLValue;
                    if(rdr2.Read() && rdr2.HasRows == true)
                        InventoryVAT = rdr2.GetFloat(0);

                    if (InventoryVAT == GlobalVariables.NULLValue)
                    {
                        if (!rdr.IsDBNull(0))
                            record.Date = rdr.GetString(0);
                        else
                            record.Date = "";
                        if (!rdr.IsDBNull(1))
                            record.Name = rdr.GetString(1);
                        else
                            record.Date = "";
                        float Gross = rdr.GetFloat(2);
                        if (Gross != (float)GlobalVariables.NULLValue)
                            record.Gross = Gross.ToString();
                        else
                            record.Gross = "";
                        float Fee = rdr.GetFloat(3);
                        if (Fee != (float)GlobalVariables.NULLValue)
                            record.Paypal_Fee = Fee.ToString();
                        else
                            record.Paypal_Fee = "";
                        if (!rdr.IsDBNull(4))
                            record.Transaction_ID = rdr.GetString(4);
                        else
                            record.Transaction_ID = "";
                        if (!rdr.IsDBNull(5))
                            record.Title = rdr.GetString(5);
                        else
                            record.Title = "";
                        if (!rdr.IsDBNull(6))
                            record.Item_Id = rdr.GetString(6);
                        else
                            record.Item_Id = "";
                        if (i == 0)
                        {
                            if (!rdr.IsDBNull(7))
                                record.Address = rdr.GetString(7);
                            else
                                record.Address = "";
                            if (!rdr.IsDBNull(8))
                                record.Phone = rdr.GetString(8);
                            else
                                record.Phone = "";
                        }
                        else
                        {
                            if (!rdr.IsDBNull(7))
                                record.Reference_Id = rdr.GetString(7);
                            else
                                record.Reference_Id = "";
                        }
                        float vat = rdr.GetFloat(9);
                        if (vat != (float)GlobalVariables.NULLValue)
                            record.Vat = vat.ToString();
                        else
                            record.Vat = "";
                        float NET = rdr.GetFloat(10);
                        if (NET != (float)GlobalVariables.NULLValue)
                            record.NET = NET.ToString();
                        else
                            record.NET = "";
//                        float vat_rate = rdr.GetFloat(11);
                        if (vat_rate != (float)GlobalVariables.NULLValue)
                            record.vat_rate = vat_rate.ToString();
                        else
                            record.vat_rate = "";
                        //write it to the csv
                        if(i==0)
                            ExportSalesCSV.WriteDynamicFileRow(record);
                        else
                            ExportRefundsCSV.WriteDynamicFileRow(record);
                    }
                    //we can update vat if we got here
                    else
                    {
                        string Transaction_ID = "";
                        if (!rdr.IsDBNull(4))
                            Transaction_ID = rdr.GetString(4);

                        int Transaction_ID_Hash = Transaction_ID.GetHashCode();
                        float Gross = rdr.GetFloat(2);
                        float NET = Gross / (1 + InventoryVAT / 100);
                        float vat = Gross - NET;

                        var cmd2 = new SQLiteCommand(m_dbConnection);
                        if (i==0)
                            cmd2.CommandText = "UPDATE PAYPAL_SALES set vat_rate=@vat_rate,net=@net,vat=@vat where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
                        else
                            cmd2.CommandText = "UPDATE PAYPAL_REFUNDS set vat_rate=@vat_rate,net=@net,vat=@vat where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
                        cmd2.Parameters.AddWithValue("@vat_rate", InventoryVAT);
                        cmd2.Parameters.AddWithValue("@net", NET);
                        cmd2.Parameters.AddWithValue("@vat", vat);
                        cmd2.Parameters.AddWithValue("@IdColhash", Transaction_ID_Hash);
                        cmd2.Parameters.AddWithValue("@TransactionIDCol", Transaction_ID);
                        cmd2.Prepare();
                        cmd2.ExecuteNonQuery();
                    }
                }
            }
            GlobalVariables.ImportingToDBBlock ="";
            ExportSalesCSV.Dispose();
            ExportRefundsCSV.Dispose();
            GlobalVariables.Logger.Log("Update VAT - Finished");
        }
    }
}
